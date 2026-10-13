#include "PianoRollEditor.hpp"
#include <ogfx/gui/Window.hpp>
#include <ogfx/render/BasicRenderer.hpp>
#include <ostd/io/Serial.hpp>
#include <ostd/io/Memory.hpp>
#include <ostd/io/Logger.hpp>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace
{
	inline i64 now_ns(void)
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	constexpr u32 kProjectMagic = 0x314A5250; // "PRJ1" as a little-endian u32
	// Bumped for the slot-name/music-key additions - this format isn't meant to stay stable
	// across the widget's own active development (it's a one-off tool, not a public format),
	// so a mismatch just fails the load cleanly rather than attempting migration.
	constexpr u32 kProjectVersion = 2;

	// Hand-rolled append/read over a plain ostd::ByteStream (stdvec<i8>), using the low-level
	// to_bytes()/from_bytes() packing utilities directly - ostd::Serial's own Writer/Reader
	// need a fixed-size buffer up front and writeToFile() always writes that whole buffer
	// (including any unused tail), which doesn't fit a variable-length file like this one.
	template<typename T>
	void append_scalar(ostd::ByteStream& buf, T value)
	{
		auto bytes = ostd::serial::to_bytes(value);
		buf.insert(buf.end(), bytes.begin(), bytes.end());
	}

	template<typename T>
	T read_scalar(const ostd::ByteStream& buf, size_t& offset)
	{
		T value = ostd::serial::from_bytes<T>(std::span<const i8>(buf.data() + offset, sizeof(T)));
		offset += sizeof(T);
		return value;
	}

	void append_bytes(ostd::ByteStream& buf, const void* data, u32 size)
	{
		append_scalar<u32>(buf, size);
		const i8* p = reinterpret_cast<const i8*>(data);
		buf.insert(buf.end(), p, p + size);
	}

	ostd::ByteStream read_bytes(const ostd::ByteStream& buf, size_t& offset)
	{
		u32 size = read_scalar<u32>(buf, offset);
		ostd::ByteStream out(buf.begin() + (ptrdiff_t)offset, buf.begin() + (ptrdiff_t)(offset + size));
		offset += size;
		return out;
	}

	void append_string(ostd::ByteStream& buf, const String& s)
	{
		std::string str = s.cpp_str();
		append_bytes(buf, str.data(), (u32)str.size());
	}

	String read_string(const ostd::ByteStream& buf, size_t& offset)
	{
		auto bytes = read_bytes(buf, offset);
		return String(std::string(bytes.begin(), bytes.end()));
	}

	// A selection as: voice count, then per voice a size-prefixed 0/1 flag byte per note -
	// simple and robust over premature bit-packing.
	void write_selection(ostd::ByteStream& buf, const stdvec<stdvec<bool>>& selection)
	{
		append_scalar<u32>(buf, (u32)selection.size());
		for (auto& voiceFlags : selection)
		{
			ostd::ByteStream flags(voiceFlags.size());
			for (size_t i = 0; i < voiceFlags.size(); i++)
				flags[i] = voiceFlags[i] ? 1 : 0;
			append_bytes(buf, flags.data(), (u32)flags.size());
		}
	}

	stdvec<stdvec<bool>> read_selection(const ostd::ByteStream& buf, size_t& offset)
	{
		u32 voiceCount = read_scalar<u32>(buf, offset);
		stdvec<stdvec<bool>> result(voiceCount);
		for (u32 v = 0; v < voiceCount; v++)
		{
			auto flags = read_bytes(buf, offset);
			result[v].resize(flags.size());
			for (size_t i = 0; i < flags.size(); i++)
				result[v][i] = flags[i] != 0;
		}
		return result;
	}
}

void PianoRollEditor::applyTheme(const ostd::Stylesheet& theme)
{
	setStylesheetCategoryName("pianoRoll");
	PianoRoll::applyTheme(theme);
	setStylesheetCategoryName("pianoRollEditor");
	setSelectionHighlightColor(getThemeValue<Color>(theme, "selectionHighlightColor", getSelectionHighlightColor()));
	setBoxSelectColor(getThemeValue<Color>(theme, "boxSelectColor", getBoxSelectColor()));
	setScrollSpeedMultiplier(getThemeValue<f32>(theme, "scrollSpeedMultiplier", getScrollSpeedMultiplier()));
	setSelectionBorderWidth(getThemeValue<f32>(theme, "selectionBorderWidth", getSelectionBorderWidth()));
	setFocusModeAlpha(getThemeValue<f32>(theme, "focusModeAlpha", getFocusModeAlpha()));
	setHelpOverlayColor(getThemeValue<Color>(theme, "helpOverlayColor", getHelpOverlayColor()));
	setHelpOverlayKeyColor(getThemeValue<Color>(theme, "helpOverlayKeyColor", getHelpOverlayKeyColor()));
	for (u32 i = 0; i < 10; i++)
		m_slotColors[i] = getThemeValue<Color>(theme, String("slot").add((i32)i).add("Color"), m_slotColors[i]);
}

void PianoRollEditor::create_default_keybinds(void)
{
	for (i32 i = 0; i < 10; i++)
		recallSlotKey[(size_t)i] = { (i32)(KeyCode::Num0 + i), true };
	for (i32 i = 0; i < 9; i++)
		saveSlotKey[(size_t)i] = { (i32)(KeyCode::Num1 + i), true };
}

void PianoRollEditor::push_undo_snapshot(void)
{
	m_undoStack.push_back(m_selected);
	if (m_undoStack.size() > kMaxUndoHistory)
		m_undoStack.erase(m_undoStack.begin());
	m_redoStack.clear();
}

void PianoRollEditor::resize_selection_for_voices(void)
{
	m_selected.resize(getVoiceCount());
	for (u32 i = 0; i < getVoiceCount(); i++)
		m_selected[i].resize(getVoice(i).notes.size(), false);
}

bool PianoRollEditor::selection_is_empty(const stdvec<stdvec<bool>>& sel) const
{
	for (auto& v : sel)
		for (bool b : v)
			if (b) return false;
	return true;
}

bool PianoRollEditor::selection_equals(const stdvec<stdvec<bool>>& a, const stdvec<stdvec<bool>>& b) const
{
	if (a.size() != b.size())
		return false;
	for (size_t v = 0; v < a.size(); v++)
	{
		if (a[v].size() != b[v].size())
			return false;
		for (size_t n = 0; n < a[v].size(); n++)
			if (a[v][n] != b[v][n])
				return false;
	}
	return true;
}

std::pair<u32, u32> PianoRollEditor::hit_test_note(Vec2 screenPos) const
{
	Rectangle bounds = getGlobalBounds();
	f32 scale = compute_scale(bounds);
	for (u32 v = 0; v < getVoiceCount(); v++)
	{
		auto& voice = getVoice(v);
		if (!voice.visible)
			continue;
		for (u32 n = 0; n < voice.notes.size(); n++)
		{
			auto& note = voice.notes[n];
			if (!is_note_in_fall_window(note))
				continue;
			bool isBlack = ostd::MidiParser::getNoteInfo(note.pitch).isBlackKey();
			Rectangle rect = compute_falling_note_rect(note, isBlack, bounds, scale);
			if (rect.contains(screenPos))
				return { v, n };
		}
	}
	return { (u32)-1, (u32)-1 };
}

void PianoRollEditor::setNoteSelected(u32 voiceIndex, u32 noteIndex, bool selected)
{
	if (voiceIndex >= m_selected.size() || noteIndex >= m_selected[voiceIndex].size())
		return;
	push_undo_snapshot();
	m_selected[voiceIndex][noteIndex] = selected;
}

void PianoRollEditor::selectAll(bool additive)
{
	push_undo_snapshot();
	for (auto& v : m_selected)
		std::fill(v.begin(), v.end(), additive);
}

void PianoRollEditor::invertSelection(void)
{
	push_undo_snapshot();
	for (auto& v : m_selected)
		for (size_t i = 0; i < v.size(); i++)
			v[i] = !v[i];
}

void PianoRollEditor::selectSamePitch(i32 midiPitch, bool additive)
{
	push_undo_snapshot();
	for (u32 v = 0; v < getVoiceCount(); v++)
	{
		auto& voice = getVoice(v);
		for (u32 n = 0; n < voice.notes.size(); n++)
			if (voice.notes[n].pitch == midiPitch)
				m_selected[v][n] = additive;
	}
}

void PianoRollEditor::boxSelect(const Rectangle& screenRect, bool additive)
{
	push_undo_snapshot();
	Rectangle bounds = getGlobalBounds();
	f32 scale = compute_scale(bounds);
	for (u32 v = 0; v < getVoiceCount(); v++)
	{
		auto& voice = getVoice(v);
		if (!voice.visible)
			continue;
		for (u32 n = 0; n < voice.notes.size(); n++)
		{
			auto& note = voice.notes[n];
			if (!is_note_in_fall_window(note))
				continue;
			bool isBlack = ostd::MidiParser::getNoteInfo(note.pitch).isBlackKey();
			Rectangle rect = compute_falling_note_rect(note, isBlack, bounds, scale);
			if (rect.intersects(screenRect))
				m_selected[v][n] = additive;
		}
	}
}

u32 PianoRollEditor::getSelectionCount(void) const
{
	u32 count = 0;
	for (auto& v : m_selected)
		for (bool b : v)
			if (b) count++;
	return count;
}

void PianoRollEditor::undo(void)
{
	if (m_undoStack.empty())
		return;
	m_redoStack.push_back(m_selected);
	m_selected = m_undoStack.back();
	m_undoStack.pop_back();
}

void PianoRollEditor::redo(void)
{
	if (m_redoStack.empty())
		return;
	m_undoStack.push_back(m_selected);
	m_selected = m_redoStack.back();
	m_redoStack.pop_back();
}

void PianoRollEditor::saveSlot(u32 slot)
{
	if (slot == 0 || slot > 9)
		return;
	m_slots[slot] = m_selected;
	m_slotOccupied[slot] = true;
}

u32 PianoRollEditor::getSlotNoteCount(u32 slot) const
{
	if (slot >= m_slots.size())
		return 0;
	u32 count = 0;
	for (auto& v : m_slots[slot])
		for (bool b : v)
			if (b) count++;
	return count;
}

bool PianoRollEditor::isNoteInSlot(u32 slot, u32 voiceIndex, u32 noteIndex) const
{
	if (slot >= m_slots.size() || voiceIndex >= m_slots[slot].size() || noteIndex >= m_slots[slot][voiceIndex].size())
		return false;
	return m_slots[slot][voiceIndex][noteIndex];
}

void PianoRollEditor::setSlotName(u32 slot, const String& name)
{
	if (slot < m_slotNames.size())
		m_slotNames[slot] = name;
}

String PianoRollEditor::getSlotName(u32 slot) const
{
	return slot < m_slotNames.size() ? m_slotNames[slot] : String("");
}

void PianoRollEditor::setSlotColor(u32 slot, const Color& color)
{
	if (slot < m_slotColors.size())
		m_slotColors[slot] = color;
}

Color PianoRollEditor::getSlotColor(u32 slot) const
{
	return slot < m_slotColors.size() ? m_slotColors[slot] : m_selectionHighlightColor;
}

stdvec<std::pair<u32, u32>> PianoRollEditor::getSelectedNotes(void) const
{
	stdvec<std::pair<u32, u32>> result;
	for (u32 v = 0; v < m_selected.size(); v++)
		for (u32 n = 0; n < m_selected[v].size(); n++)
			if (m_selected[v][n])
				result.push_back({ v, n });
	return result;
}

Rectangle PianoRollEditor::getBoxSelectionRect(void) const
{
	if (!m_boxSelecting)
		return {};
	f32 left = std::min(m_boxSelectStart.x, m_boxSelectCurrent.x);
	f32 top = std::min(m_boxSelectStart.y, m_boxSelectCurrent.y);
	return { left, top, std::abs(m_boxSelectCurrent.x - m_boxSelectStart.x), std::abs(m_boxSelectCurrent.y - m_boxSelectStart.y) };
}

Color PianoRollEditor::resolve_active_highlight_color(void) const
{
	for (u32 s = 0; s < 10; s++)
		if (m_slotOccupied[s] && selection_equals(m_selected, m_slots[s]))
			return m_slotColors[s];
	return m_selectionHighlightColor;
}

stdvec<u32> PianoRollEditor::slots_containing(u32 voiceIndex, u32 noteIndex) const
{
	stdvec<u32> result;
	for (u32 s = 0; s < 10; s++)
		if (isNoteInSlot(s, voiceIndex, noteIndex))
			result.push_back(s);
	return result;
}

void PianoRollEditor::fire_hover_signal(u32 voiceIndex, u32 noteIndex)
{
	auto& note = getVoice(voiceIndex).notes[noteIndex];
	PianoRollEditorNoteHoverData data;
	data.voiceIndex = voiceIndex;
	data.noteIndex = noteIndex;
	data.pitch = note.pitch;
	data.velocity = note.velocity;
	data.channel = note.channel;
	data.startTime = note.startTime;
	data.endTime = note.endTime;
	data.noteInfo = ostd::MidiParser::getNoteInfo(note.pitch);
	data.slots = slots_containing(voiceIndex, noteIndex);
	ostd::SignalHandler::emitSignal(NoteHoverSignal, ostd::Signal::Priority::RealTime, &data);
}

void PianoRollEditor::recallSlot(u32 slot)
{
	if (slot > 9)
		return;

	if (!selection_is_empty(m_selected))
	{
		bool foundElsewhere = false;
		for (u32 s = 0; s < 10 && !foundElsewhere; s++)
			if (m_slotOccupied[s] && selection_equals(m_selected, m_slots[s]))
				foundElsewhere = true;
		if (!foundElsewhere)
		{
			m_slots[0] = m_selected;
			m_slotOccupied[0] = true;
		}
	}

	push_undo_snapshot();
	if (m_slotOccupied[slot])
		m_selected = m_slots[slot];
	else
		for (auto& v : m_selected)
			std::fill(v.begin(), v.end(), false);
}

bool PianoRollEditor::export_notes(const stdvec<stdvec<bool>>& selection, const String& path)
{
	stdvec<ostd::MidiParser::NoteEvent> collected;
	for (u32 v = 0; v < getVoiceCount() && v < selection.size(); v++)
	{
		auto& voice = getVoice(v);
		for (u32 n = 0; n < voice.notes.size() && n < selection[v].size(); n++)
			if (selection[v][n])
				collected.push_back(voice.notes[n]);
	}
	if (collected.empty())
		return false;
	std::sort(collected.begin(), collected.end(), [](auto& a, auto& b) { return a.startTick < b.startTick; });
	return ostd::MidiParser::writeFile(path, collected, m_ticksPerQuarterNote, m_tempoMap);
}

bool PianoRollEditor::exportSelectionToFile(const String& path)
{
	return export_notes(m_selected, path);
}

u32 PianoRollEditor::exportAllSlotsToFolder(const String& folderPath)
{
	u32 written = 0;
	String baseName = String(std::filesystem::path(m_sourceMidiPath.cpp_str()).stem().string());
	u32 firstSlot = m_includeSlotZeroInExportAll ? 0 : 1;
	for (u32 slot = firstSlot; slot <= 9; slot++)
	{
		if (!m_slotOccupied[slot] || selection_is_empty(m_slots[slot]))
			continue;
		String outPath = String(folderPath).add("/").add(baseName).add(" - voice_").add((i32)slot).add(".mid");
		if (export_notes(m_slots[slot], outPath))
			written++;
	}
	return written;
}

bool PianoRollEditor::saveProjectAs(const String& path)
{
	ostd::ByteStream buf;
	append_scalar<u32>(buf, kProjectMagic);
	append_scalar<u32>(buf, kProjectVersion);

	// The .mid file's raw bytes, embedded verbatim - re-read fresh from disk right now, so
	// replacing the file (a new loadVoice() call, or just overwriting it externally) and
	// saving again embeds the current version, not whatever was loaded originally.
	ostd::ByteStream midiBytes;
	if (!ostd::Memory::loadByteStreamFromFile(m_sourceMidiPath, midiBytes))
		return false;
	append_bytes(buf, midiBytes.data(), (u32)midiBytes.size());

	// Audio, embedded the same way (same "re-read at save time" guarantee) - stored alongside
	// its own original filename (not the full path, just enough to recover the extension
	// AudioClip::load() dispatches on), empty if no audio is loaded.
	String audioFilename = m_audioPath.empty() ? "" : String(std::filesystem::path(m_audioPath.cpp_str()).filename().string());
	append_string(buf, audioFilename);
	if (!audioFilename.empty())
	{
		ostd::ByteStream audioBytes;
		if (!ostd::Memory::loadByteStreamFromFile(m_audioPath, audioBytes))
			return false;
		append_bytes(buf, audioBytes.data(), (u32)audioBytes.size());
	}

	write_selection(buf, m_selected);
	for (u32 slot = 0; slot < 10; slot++)
	{
		append_scalar<u8>(buf, m_slotOccupied[slot] ? 1 : 0);
		write_selection(buf, m_slots[slot]);
		append_string(buf, m_slotNames[slot]);
	}
	append_scalar<i32>(buf, (i32)getMusicKey());

	if (!ostd::Memory::saveByteStreamToFile(buf, path))
		return false;
	m_currentProjectPath = path;
	return true;
}

bool PianoRollEditor::saveProject(const String& path)
{
	// A non-empty path always wins (and becomes the newly tracked one, via saveProjectAs());
	// an empty one falls back to wherever this project was last saved.
	return saveProjectAs(!path.empty() ? path : m_currentProjectPath);
}

bool PianoRollEditor::loadProject(const String& path)
{
	ostd::ByteStream buf;
	if (!ostd::Memory::loadByteStreamFromFile(path, buf))
		return false;

	size_t offset = 0;
	if (read_scalar<u32>(buf, offset) != kProjectMagic)
		return false;
	if (read_scalar<u32>(buf, offset) != kProjectVersion)
	{
		OX_WARN("PianoRollEditor::loadProject: \"%s\" is a different project file version - re-save it with the current version to load it.", path.c_str());
		return false;
	}

	ostd::ByteStream midiBytes = read_bytes(buf, offset);
	String tempMidiPath = String(path).add(".loaded.mid");
	if (!ostd::Memory::saveByteStreamToFile(midiBytes, tempMidiPath))
		return false;

	String audioFilename = read_string(buf, offset);
	String tempAudioPath = "";
	if (!audioFilename.empty())
	{
		ostd::ByteStream audioBytes = read_bytes(buf, offset);
		tempAudioPath = String(path).add(".loaded.").add(audioFilename);
		if (!ostd::Memory::saveByteStreamToFile(audioBytes, tempAudioPath))
			return false;
	}

	clearVoices();
	if (loadVoice(tempMidiPath) == (u32)-1)
		return false;

	m_selected = read_selection(buf, offset);
	resize_selection_for_voices(); // in case the active selection's shape predates this voice count
	for (u32 slot = 0; slot < 10; slot++)
	{
		m_slotOccupied[slot] = read_scalar<u8>(buf, offset) != 0;
		m_slots[slot] = read_selection(buf, offset);
		m_slotNames[slot] = read_string(buf, offset);
	}
	setMusicKey((ostd::MidiParser::eMusicKey)read_scalar<i32>(buf, offset));

	if (!tempAudioPath.empty())
		loadAudio(tempAudioPath);
	else
		clearAudio();

	m_currentProjectPath = path;
	return true;
}

void PianoRollEditor::onDraw(BasicRenderer2D& gfx)
{
	PianoRoll::onDraw(gfx);

	Rectangle bounds = getGlobalBounds();
	f32 scale = compute_scale(bounds);
	Color activeHighlightColor = resolve_active_highlight_color();
	Rectangle liveBoxRect = getBoxSelectionRect(); // {} (zero-size) when not box-selecting - never intersects anything, so no special-casing needed below
	Color dimOverlay = Color(0, 0, 0, (u8)std::clamp((1.0f - m_focusModeAlpha) * 255.0f, 0.0f, 255.0f));

	for (u32 v = 0; v < getVoiceCount(); v++)
	{
		auto& voice = getVoice(v);
		if (!voice.visible || v >= m_selected.size())
			continue;
		for (u32 n = 0; n < voice.notes.size() && n < m_selected[v].size(); n++)
		{
			auto& note = voice.notes[n];
			if (!is_note_in_fall_window(note))
				continue;
			bool isBlack = ostd::MidiParser::getNoteInfo(note.pitch).isBlackKey();
			Rectangle rect = compute_falling_note_rect(note, isBlack, bounds, scale);

			bool selected = m_selected[v][n];
			bool hovered = !m_shiftHeld && v == m_hoveredVoiceIndex && n == m_hoveredNoteIndex;
			bool inLiveBox = m_boxSelecting && rect.intersects(liveBoxRect);

			// Focus mode dims everything except what's selected, hovered (unless Shift is
			// held), or - while actively dragging - inside the live box-select preview.
			if (m_focusModeActive && !selected && !hovered && !inLiveBox)
				gfx.fillRoundRect(rect, dimOverlay, 3.0f * scale);

			if (selected)
				gfx.outlinedRoundRect(rect, Colors::Transparent, activeHighlightColor, 3.0f * scale, (i32)std::round(m_selectionBorderWidth * scale));
		}
	}

	if (m_boxSelecting)
		gfx.outlinedRoundRect(liveBoxRect, Colors::Transparent, m_boxSelectColor, 0.0f, 1);

	if (m_showHelpOverlay)
		draw_help_overlay(gfx);
}

namespace
{
	inline const char* primary_key_name(void)
	{
#ifdef MAC_OS
		return "Cmd";
#else
		return "Ctrl";
#endif
	}
}

void PianoRollEditor::draw_help_overlay(BasicRenderer2D& gfx) const
{
	constexpr f32 fontSize = 14.0f;
	constexpr f32 lineSpacing = 4.0f;
	constexpr f32 margin = 8.0f;
	constexpr f32 columnGap = 8.0f; // between the keybind column and the description column

	auto lines = build_help_lines();
	Rectangle bounds = getGlobalBounds();

	// A single shared column width (the widest keybind string) keeps every description aligned,
	// instead of each one starting wherever its own keybind text happens to end.
	f32 keyColumnWidth = 0.0f;
	for (auto& [key, description] : lines)
		keyColumnWidth = std::max(keyColumnWidth, gfx.getStringDimensions(key, (i32)fontSize).x);

	f32 y = bounds.y + margin;
	for (auto& [key, description] : lines)
	{
		gfx.drawString(key, { bounds.x + margin, y }, m_helpOverlayKeyColor, (i32)fontSize);
		gfx.drawString(description, { bounds.x + margin + keyColumnWidth + columnGap, y }, m_helpOverlayColor, (i32)fontSize);
		y += fontSize + lineSpacing;
	}
}

stdvec<std::pair<String, String>> PianoRollEditor::build_help_lines(void) const
{
	stdvec<std::pair<String, String>> lines;
	auto addLine = [&](const Keybind& kb, bool primary, bool primaryShift, const String& description)
	{
		if (!kb.enabled)
			return; // a disabled binding has nothing to show
		lines.push_back({ format_keybind(kb, primary, primaryShift).add(":"), description });
	};

	addLine(helpOverlayKey, false, false, "Toggle this overlay");
	addLine(playPauseKey, false, false, "Play / pause");
	addLine(invertSelectionKey, false, false, "Invert selection");
	addLine(focusModeKey, false, false, "Toggle focus mode");
	addLine(undoKey, true, false, "Undo");
	addLine(redoKey, true, false, "Redo");
	addLine(exportSelectionKey, false, false, "Export selection to .mid");
	addLine(exportAllKey, true, false, "Export all slots to folder");
	addLine(saveKey, true, false, "Save project");
	addLine(saveAsKey, false, true, "Save project as...");
	// The 10 recall-slot and 9 save-slot bindings are really one family each (the key pressed
	// directly names the slot), so one summary line per family reads far better than 19 nearly
	// identical ones - this assumes the default contiguous Num0-Num9 layout
	// create_default_keybinds() sets up, not whatever recallSlotKey/saveSlotKey actually hold
	// right now.
	lines.push_back({ "0-9:", "Recall slot (0 = last auto-backup)" });
	lines.push_back({ String(primary_key_name()).add("+1-9:"), "Save to slot" });
	return lines;
}

String PianoRollEditor::format_keybind(const Keybind& kb, bool primary, bool primaryShift) const
{
	String name = SDL_GetKeyName((SDL_Keycode)kb.keyCode);
	if (name.empty())
		name = "?";
	if (primaryShift)
		return String(primary_key_name()).add("+Shift+").add(name);
	if (primary)
		return String(primary_key_name()).add("+").add(name);
	return name;
}

void PianoRollEditor::onUpdate(void)
{
	PianoRoll::onUpdate();
	m_doubleClickTimer.update();

	if (m_scrollAnimating)
	{
		constexpr f64 glideDurationSeconds = 0.25;
		f64 elapsed = (f64)(now_ns() - m_scrollGlideStartTimeNs) / 1e9;
		f64 t = std::clamp(elapsed / glideDurationSeconds, 0.0, 1.0);
		f64 easedT = 1.0 - (1.0 - t) * (1.0 - t); // ease-out quadratic
		seekSeconds(m_scrollGlideStartValue + (m_scrollTargetSeconds - m_scrollGlideStartValue) * easedT);
		if (t >= 1.0)
			m_scrollAnimating = false;
	}
}

void PianoRollEditor::onMousePressed(const Event& event)
{
	if (event.mouse->button != MouseEventData::eButton::Left)
		return;

	Vec2 pos { event.mouse->position_x, event.mouse->position_y };
	constexpr f32 doubleClickRadius = 6.0f;
	bool isDoubleClick = m_lastClickValid && m_doubleClickTimer.isCounting()
		&& std::abs(pos.x - m_lastClickPos.x) < doubleClickRadius
		&& std::abs(pos.y - m_lastClickPos.y) < doubleClickRadius;

	auto [voiceIndex, noteIndex] = hit_test_note(pos);
	bool hitNote = voiceIndex != (u32)-1;
	bool additive = !m_shiftHeld;

	if (isDoubleClick)
	{
		m_lastClickValid = false; // don't chain into a triple-click
		if (hitNote)
			selectSamePitch(getVoice(voiceIndex).notes[noteIndex].pitch, additive);
		else
			selectAll(additive);
	}
	else
	{
		m_lastClickPos = pos;
		m_lastClickValid = true;
		m_doubleClickTimer.start();

		if (hitNote)
			setNoteSelected(voiceIndex, noteIndex, additive);
		else
		{
			m_boxSelecting = true;
			m_boxSelectStart = pos;
			m_boxSelectCurrent = pos;
			m_boxSelectSubtract = m_shiftHeld;
		}
	}
}

void PianoRollEditor::onMouseDragged(const Event& event)
{
	// The actual selection is applied on release (boxSelect() itself does the hit-test) -
	// this just tracks where to draw the live box, see onDraw().
	if (m_boxSelecting)
		m_boxSelectCurrent = { event.mouse->position_x, event.mouse->position_y };
}

void PianoRollEditor::onMouseReleased(const Event& event)
{
	if (!m_boxSelecting)
		return;
	m_boxSelecting = false;
	if (event.mouse->button != MouseEventData::eButton::Left)
		return;

	Vec2 end { event.mouse->position_x, event.mouse->position_y };
	f32 left = std::min(m_boxSelectStart.x, end.x);
	f32 top = std::min(m_boxSelectStart.y, end.y);
	Rectangle dragRect { left, top, std::abs(end.x - m_boxSelectStart.x), std::abs(end.y - m_boxSelectStart.y) };
	if (dragRect.w < 1.0f || dragRect.h < 1.0f)
		return; // a click, not a drag - onMousePressed already handled it (or it hit nothing)
	boxSelect(dragRect, !m_boxSelectSubtract);
}

void PianoRollEditor::onMouseScrolled(const Event& event)
{
	f64 direction = event.mouse->scroll == MouseEventData::eScrollDirection::Down ? -1.0 : 1.0;
	constexpr f64 baseSeekStepSeconds = 2.0;
	f64 step = direction * baseSeekStepSeconds * (f64)m_scrollSpeedMultiplier;

	// Accumulates into a target rather than seeking straight to it - several quick wheel ticks
	// build on the same in-flight target instead of each one restarting the glide from
	// wherever playback happens to be at that instant. The glide itself always restarts fresh
	// from the current position/time though, so a new tick never causes a visible jump back
	// to some earlier glide's starting point.
	f64 base = m_scrollAnimating ? m_scrollTargetSeconds : getCurrentTimeSeconds();
	m_scrollTargetSeconds = std::clamp(base + step, 0.0, getTotalDurationSeconds());
	m_scrollGlideStartValue = getCurrentTimeSeconds();
	m_scrollGlideStartTimeNs = now_ns();
	m_scrollAnimating = true;
}

void PianoRollEditor::onMouseMoved(const Event& event)
{
	Vec2 pos { event.mouse->position_x, event.mouse->position_y };
	auto [voiceIndex, noteIndex] = hit_test_note(pos);
	if (voiceIndex == m_hoveredVoiceIndex && noteIndex == m_hoveredNoteIndex)
		return; // still inside (or still outside) the same note - not a new "enter"

	m_hoveredVoiceIndex = voiceIndex;
	m_hoveredNoteIndex = noteIndex;
	if (voiceIndex != (u32)-1)
		fire_hover_signal(voiceIndex, noteIndex);
}

void PianoRollEditor::onMouseExited(const Event& event)
{
	(void)event;
	m_hoveredVoiceIndex = (u32)-1;
	m_hoveredNoteIndex = (u32)-1;
}

void PianoRollEditor::onKeyPressed(const Event& event)
{
	auto& key = *event.keyboard;
	m_shiftHeld = key.modifiers.anyShift();

	auto matches = [&](const Keybind& kb, bool needPrimary, bool needPrimaryShiftOnly) -> bool
	{
		if (!kb.enabled || key.keyCode != kb.keyCode)
			return false;
		if (needPrimaryShiftOnly)
			return key.modifiers.primaryShiftOnly();
		if (needPrimary)
			return key.modifiers.primaryOnly();
		return !key.modifiers.any();
	};

	if (matches(invertSelectionKey, false, false)) { invertSelection(); return; }
	if (matches(focusModeKey, false, false)) { toggleFocusMode(); return; }
	if (matches(helpOverlayKey, false, false)) { toggleHelpOverlay(); return; }
	if (matches(exportAllKey, true, false)) { exportAllSlotsToFolder(getWindow().showOpenFolderDialog()); return; }
	if (matches(exportSelectionKey, false, false))
	{
		WindowCore::FileDialogFilterList filters { { "MIDI File", { "mid" } } };
		String path = getWindow().showSaveFileDialog(filters);
		if (!path.empty())
			exportSelectionToFile(path);
		return;
	}
	if (matches(undoKey, true, false)) { undo(); return; }
	if (matches(redoKey, true, false)) { redo(); return; }
	if (matches(playPauseKey, false, false)) { isPlaying() ? pause() : play(); return; }
	if (matches(saveAsKey, false, true))
	{
		WindowCore::FileDialogFilterList filters { { "PianoRoll Project", { "pianoproj" } } };
		String path = getWindow().showSaveFileDialog(filters);
		if (!path.empty())
			saveProjectAs(path);
		return;
	}
	if (matches(saveKey, true, false))
	{
		if (m_currentProjectPath.empty())
		{
			WindowCore::FileDialogFilterList filters { { "PianoRoll Project", { "pianoproj" } } };
			String path = getWindow().showSaveFileDialog(filters);
			if (!path.empty())
				saveProjectAs(path);
		}
		else
			saveProject(m_currentProjectPath);
		return;
	}

	if (key.modifiers.primaryOnly())
	{
		for (u32 slot = 1; slot <= 9; slot++)
			if (matches(saveSlotKey[(size_t)slot - 1], true, false))
			{
				saveSlot(slot);
				return;
			}
	}
	else if (!key.modifiers.any())
	{
		for (u32 slot = 0; slot <= 9; slot++)
			if (matches(recallSlotKey[(size_t)slot], false, false))
			{
				recallSlot(slot);
				return;
			}
	}
}

void PianoRollEditor::onKeyReleased(const Event& event)
{
	m_shiftHeld = event.keyboard->modifiers.anyShift();
}

u32 PianoRollEditor::loadVoice(const String& midiFilePath, const Color& color, const String& name)
{
	u32 index = PianoRoll::loadVoice(midiFilePath, color, name);
	if (index == (u32)-1)
		return index;

	resize_selection_for_voices();

	// The exact tick/tempo data used for export/project-save is scoped to the first voice
	// loaded (see m_ticksPerQuarterNote's declaration) - only capture it once.
	if (m_sourceMidiPath.empty())
	{
		m_sourceMidiPath = midiFilePath;
		stdvec<ostd::MidiParser::NoteEvent> discard;
		ostd::MidiParser::parseFileEx(midiFilePath, discard, m_ticksPerQuarterNote, m_tempoMap);
	}
	return index;
}

bool PianoRollEditor::removeVoice(u32 index)
{
	bool ok = PianoRoll::removeVoice(index);
	if (ok && index < m_selected.size())
		m_selected.erase(m_selected.begin() + index);
	return ok;
}

void PianoRollEditor::clearVoices(void)
{
	PianoRoll::clearVoices();
	m_selected.clear();
	for (u32 slot = 0; slot < 10; slot++)
	{
		m_slots[slot].clear();
		m_slotOccupied[slot] = false;
	}
	m_undoStack.clear();
	m_redoStack.clear();
	m_sourceMidiPath = "";
}

bool PianoRollEditor::loadAudio(const String& audioFilePath)
{
	bool ok = PianoRoll::loadAudio(audioFilePath);
	if (ok)
		m_audioPath = audioFilePath;
	return ok;
}

void PianoRollEditor::clearAudio(void)
{
	PianoRoll::clearAudio();
	m_audioPath = "";
}
