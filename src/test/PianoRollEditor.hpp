/*
	A project-specific tool built on top of ogfx::gui::PianoRoll - not part of Omnia itself
	(hence living here, outside any namespace), but reusing PianoRoll entirely via
	inheritance rather than duplicating its rendering/playback/voice-loading code. Adds:
	note selection (click/double-click/box-select, with an additive-by-default, Shift-to-
	subtract convention), undo/redo over that selection, 10 RTS-group-style saved selection
	slots, exporting a selection (or all saved slots) as new .mid files that preserve the
	source file's exact tick/velocity/channel data, and saving/loading the whole editing
	session (MIDI + selections) as one binary project file.
*/

#pragma once

#include <ogfx/gui/widgets/PianoRoll.hpp>
#include <ostd/utils/Time.hpp>
#include <array>

// This header is deliberately not namespaced (per its purpose - a one-off tool, not part of
// Omnia), so it pulls in ogfx/ogfx::gui unqualified rather than spelling out every type.
// Anything that includes this header inherits that - acceptable for a test-only, single-
// purpose file not meant for broad reuse.
using namespace ogfx;
using namespace ogfx::gui;

// Signal payload for PianoRollEditor::NoteHoverSignal - fired once when the mouse enters a
// note (not on every move event while still inside it), carrying everything a listener would
// plausibly need without re-deriving it, including which saved slots (0-9) currently contain
// this exact note.
class PianoRollEditorNoteHoverData : public ostd::Object
{
	public:
		inline PianoRollEditorNoteHoverData(void) { setTypeName("PianoRollEditorNoteHoverData"); validate(); }
		u32 voiceIndex { 0 };
		u32 noteIndex { 0 };
		i32 pitch { 0 };
		i32 velocity { 0 };
		i32 channel { 0 };
		f64 startTime { 0.0 };
		f64 endTime { 0.0 };
		ostd::MidiParser::NoteInfo noteInfo;
		stdvec<u32> slots; // which of the 10 saved slots currently contain this note
};

class PianoRollEditor : public ogfx::gui::PianoRoll
{
	public:
		inline PianoRollEditor(ogfx::gui::Window& window) : ogfx::gui::PianoRoll(window)
		{
			m_doubleClickTimer.setCount(24);
			create_default_keybinds(); // was never actually called - every slot keyCode was stuck at 0
			setStylesheetCategoryName("pianoRollEditor");
		}

		void applyTheme(const ostd::Stylesheet& theme) override;

		// --- selection ---
		// Every mutating call here pushes the pre-change state onto the undo stack first (see
		// undo()/redo()) and is additive by default (never clears an existing selection by
		// itself) - matching "avoid deselecting everything by mistake".
		void setNoteSelected(u32 voiceIndex, u32 noteIndex, bool selected);
		void selectAll(bool additive = true); // additive=false subtracts (Shift) instead of adding
		void invertSelection(void);
		// Every note at this exact MIDI pitch (octave-specific - "C3" only matches other C3s,
		// not C4), across every voice - double-click's effect.
		void selectSamePitch(i32 midiPitch, bool additive = true);
		// Every note whose on-screen falling-note rectangle intersects screenRect right now -
		// box-select's effect, evaluated only against currently-visible (on-screen) notes.
		void boxSelect(const Rectangle& screenRect, bool additive = true);
		inline bool isNoteSelected(u32 voiceIndex, u32 noteIndex) const
		{
			return voiceIndex < m_selected.size() && noteIndex < m_selected[voiceIndex].size() && m_selected[voiceIndex][noteIndex];
		}
		u32 getSelectionCount(void) const;

		// --- undo/redo ---
		void undo(void);
		void redo(void);

		// --- saved selection slots (RTS-group style) ---
		// slot 0 isn't directly assignable (a no-op here, same as its keybind) - it's reserved
		// as the automatic backup recallSlot() writes to.
		void saveSlot(u32 slot /* 1-9 */);
		// Replaces the entire active selection with slot's content (slot 0-9; an unoccupied
		// slot recalls as "nothing selected"). Before replacing, if the current selection is
		// non-empty and doesn't already exactly match some other occupied slot, it's backed up
		// into slot 0 first.
		void recallSlot(u32 slot);
		inline bool isSlotOccupied(u32 slot) const { return slot < m_slots.size() && m_slotOccupied[slot]; }
		u32 getSlotNoteCount(u32 slot) const;
		bool isNoteInSlot(u32 slot, u32 voiceIndex, u32 noteIndex) const;
		// Named externally - the widget has no UI of its own to name them from, so this is
		// pure data storage (empty by default; no built-in "Slot N" fallback text, since
		// display formatting belongs to whatever UI is built around this).
		void setSlotName(u32 slot, const String& name);
		String getSlotName(u32 slot) const;
		// Per-slot highlight color (see resolve_active_highlight_color()'s use of these) -
		// configurable per slot via the theme (slot0Color..slot9Color) with sensible built-in
		// defaults; setSlotColor() overrides a slot's color at runtime independent of the theme.
		void setSlotColor(u32 slot, const Color& color);
		Color getSlotColor(u32 slot) const;

		// --- undo/redo/selection introspection (for a UI with no in-widget state of its own) ---
		inline bool canUndo(void) const { return !m_undoStack.empty(); }
		inline bool canRedo(void) const { return !m_redoStack.empty(); }
		// Every currently-selected (voiceIndex, noteIndex) pair.
		stdvec<std::pair<u32, u32>> getSelectedNotes(void) const;

		// --- export ---
		// Writes every currently-selected note (preserving exact tick/velocity/channel data)
		// as a new, single-track .mid file. X's effect.
		bool exportSelectionToFile(const String& path);
		// Writes every occupied slot 1-9 (plus slot 0 if includeSlotZeroInExportAll() is true)
		// into folderPath, named "<original base name> - voice_N.mid". Returns how many files
		// were written. Ctrl+X's effect.
		u32 exportAllSlotsToFolder(const String& folderPath);
		OSTD_BOOL_PARAM_GETSET_E(IncludeSlotZeroInExportAll, m_includeSlotZeroInExportAll); // default false

		// --- project save/load ---
		// Embeds both the source .mid file's and (if loaded) the audio file's raw bytes
		// directly (byte-exact reopening, independent of anything NoteEvent does or doesn't
		// capture), plus the active selection and all 10 slots. Both files are re-read from
		// disk at save time, so replacing either one (loadVoice()/loadAudio() again, or just
		// overwriting the file on disk) and saving again embeds the new version, not whatever
		// was loaded originally.
		bool saveProject(const String& path); // acts as saveProjectAs() if nothing's been saved yet
		bool saveProjectAs(const String& path);
		bool loadProject(const String& path);
		inline const String& getCurrentProjectPath(void) const { return m_currentProjectPath; }
		inline const String& getSourceMidiPath(void) const { return m_sourceMidiPath; }
		inline const String& getAudioPath(void) const { return m_audioPath; }
		inline i32 getTicksPerQuarterNote(void) const { return m_ticksPerQuarterNote; }

		// --- focus mode ---
		// While active, every note that's neither selected, hovered (see NoteHoverSignal), nor
		// (while dragging) inside the live box-select preview is dimmed to focusModeAlpha.
		inline void toggleFocusMode(void) { m_focusModeActive = !m_focusModeActive; }
		inline void setFocusModeActive(bool active) { m_focusModeActive = active; }
		inline bool isFocusModeActive(void) const { return m_focusModeActive; }
		OSTD_PARAM_GETSET(f32, FocusModeAlpha, m_focusModeAlpha); // 0..1, default 0.4, themeable ("focusModeAlpha")

		// --- help overlay ---
		// Top-left, lists every enabled keybind below with a short description, one per line -
		// toggled by helpOverlayKey (on by default, see m_showHelpOverlay).
		inline void setHelpOverlayVisible(bool visible) { m_showHelpOverlay = visible; }
		inline bool isHelpOverlayVisible(void) const { return m_showHelpOverlay; }
		inline void toggleHelpOverlay(void) { m_showHelpOverlay = !m_showHelpOverlay; }
		// The description text's color; the keybind itself (e.g. "Ctrl+Z") is drawn separately
		// in HelpOverlayKeyColor, so the two are visually distinguishable at a glance.
		OSTD_PARAM_GETSET(Color, HelpOverlayColor, m_helpOverlayColor); // themeable ("helpOverlayColor")
		OSTD_PARAM_GETSET(Color, HelpOverlayKeyColor, m_helpOverlayKeyColor); // themeable ("helpOverlayKeyColor")

		// --- live interaction state (for a UI with no in-widget state of its own) ---
		inline bool isBoxSelecting(void) const { return m_boxSelecting; }
		inline bool isBoxSelectSubtracting(void) const { return m_boxSelectSubtract; }
		Rectangle getBoxSelectionRect(void) const; // the live drag rect; an empty Rectangle if not box-selecting
		inline bool isHoveringNote(void) const { return m_hoveredVoiceIndex != (u32)-1; }
		inline std::pair<u32, u32> getHoveredNote(void) const { return { m_hoveredVoiceIndex, m_hoveredNoteIndex }; }
		inline bool isScrollAnimating(void) const { return m_scrollAnimating; }
		inline f64 getScrollTargetSeconds(void) const { return m_scrollTargetSeconds; }

		// Fired once when the mouse enters a note (not repeatedly while it stays inside one) -
		// see PianoRollEditorNoteHoverData.
		inline static const u32 NoteHoverSignal { ostd::SignalHandler::newCustomSignal(12100) };

		// --- keybinds - every binding here is independently remappable and can be disabled
		// outright (enabled=false); Ctrl/Shift themselves are fixed, not remappable. ---
		struct Keybind { i32 keyCode { 0 }; bool enabled { true }; };
		Keybind invertSelectionKey { ogfx::KeyCode::I };
		Keybind exportSelectionKey { ogfx::KeyCode::X };
		Keybind exportAllKey       { ogfx::KeyCode::X };     // + primary
		Keybind undoKey            { ogfx::KeyCode::Z };     // + primary
		Keybind redoKey            { ogfx::KeyCode::Y };     // + primary
		Keybind playPauseKey       { ogfx::KeyCode::Space };
		Keybind saveKey            { ogfx::KeyCode::S };     // + primary
		Keybind saveAsKey          { ogfx::KeyCode::S };     // + primary + shift
		std::array<Keybind, 10> recallSlotKey; // index 0-9, default Num0-Num9, set in create_default_keybinds()
		std::array<Keybind, 9>  saveSlotKey;   // index 0-8 -> slot 1-9, default Num1-Num9 + primary
		Keybind focusModeKey { ogfx::KeyCode::F };
		Keybind helpOverlayKey { ogfx::KeyCode::H };

		// --- style (themeable - see applyTheme(); category "pianoRollEditor") ---
		OSTD_PARAM_GETSET(Color, SelectionHighlightColor, m_selectionHighlightColor);
		OSTD_PARAM_GETSET(Color, BoxSelectColor, m_boxSelectColor);
		// Multiplies the base per-scroll-tick seek step (seconds). 1.0 = the base step.
		OSTD_PARAM_GETSET(f32, ScrollSpeedMultiplier, m_scrollSpeedMultiplier);
		OSTD_PARAM_GETSET(f32, SelectionBorderWidth, m_selectionBorderWidth);

	public:
		// Overridden (not just inherited) so the editor's own per-note selection bookkeeping
		// (m_selected, plus the first-loaded-voice tick/tempo capture) stays in sync - public,
		// same as on PianoRoll, so this is still a plain drop-in replacement for it.
		u32 loadVoice(const String& midiFilePath, const Color& color = Colors::Transparent, const String& name = "") override;
		bool removeVoice(u32 index) override;
		void clearVoices(void) override;
		// Overridden just to remember the path (needed for saveProject() - PianoRoll itself
		// has no "what audio is loaded" accessor), nothing else changes.
		bool loadAudio(const String& audioFilePath) override;
		void clearAudio(void) override;

	protected:
		void onDraw(BasicRenderer2D& gfx) override;
		void onUpdate(void) override; // drives m_doubleClickTimer, same frame-counted pattern as TextEdit's
		void onMousePressed(const Event& event) override;
		void onMouseDragged(const Event& event) override;
		void onMouseReleased(const Event& event) override;
		void onMouseScrolled(const Event& event) override;
		void onMouseMoved(const Event& event) override;  // hover tracking + NoteHoverSignal
		void onMouseExited(const Event& event) override; // clears hover state
		void onKeyPressed(const Event& event) override;
		// MouseEventData carries no modifier-key state of its own, so Shift's live state (used
		// by every mouse-driven selection action) is tracked here from whichever key events
		// happen to arrive - including the Shift key's own press/release.
		void onKeyReleased(const Event& event) override;

	private:
		void create_default_keybinds(void);
		void push_undo_snapshot(void);
		void resize_selection_for_voices(void);
		// Returns {voiceIndex, noteIndex} of the topmost visible note under screenPos, or
		// {(u32)-1, (u32)-1} if none.
		std::pair<u32, u32> hit_test_note(Vec2 screenPos) const;
		bool selection_equals(const stdvec<stdvec<bool>>& a, const stdvec<stdvec<bool>>& b) const;
		bool selection_is_empty(const stdvec<stdvec<bool>>& sel) const;

		bool export_notes(const stdvec<stdvec<bool>>& selection, const String& path);
		// Which of the 10 slots currently equal the active selection exactly (by content) -
		// the active-selection highlight uses the first match's color, falling back to
		// selectionHighlightColor() if none match. Mirrors the same "does this selection
		// already exist in a slot" comparison recallSlot() uses.
		Color resolve_active_highlight_color(void) const;
		stdvec<u32> slots_containing(u32 voiceIndex, u32 noteIndex) const;
		void fire_hover_signal(u32 voiceIndex, u32 noteIndex);

		void draw_help_overlay(BasicRenderer2D& gfx) const;
		// One {keybind, description} pair per line - the keybind half is rendered in
		// HelpOverlayKeyColor, the description half in HelpOverlayColor, so draw_help_overlay()
		// needs them kept apart rather than one pre-joined string. Built fresh every draw so a
		// runtime remap (or disabling a binding) is reflected immediately. The 10 recall-slot
		// and 9 save-slot bindings are summarized as two lines ("0-9"/"Ctrl+1-9") instead of one
		// line per slot - correct for the default contiguous layout create_default_keybinds()
		// sets up, not re-derived from whatever the slot keys actually are right now.
		stdvec<std::pair<String, String>> build_help_lines(void) const;
		// SDL_GetKeyName() keeps this correct under remapping without hand-naming every
		// KeyCode; primary/primaryShift prepend "Ctrl+"/"Cmd+" (see Events.hpp's own MAC_OS
		// split) to match how matches() in onKeyPressed() actually interprets the binding.
		String format_keybind(const Keybind& kb, bool primary = false, bool primaryShift = false) const;

	private:
		stdvec<stdvec<bool>> m_selected; // [voiceIndex][noteIndex]

		std::array<stdvec<stdvec<bool>>, 10> m_slots;
		std::array<bool, 10> m_slotOccupied { false, false, false, false, false, false, false, false, false, false };
		std::array<String, 10> m_slotNames; // empty by default - see setSlotName()
		// Sensible, visually-distinct defaults; setSlotColor()/the theme (slot0Color..
		// slot9Color) can override any of them. Slot 0 (the auto-backup slot) gets a neutral
		// gray since it's not a deliberate "voice", unlike 1-9.
		std::array<Color, 10> m_slotColors {
			Color("#888888C0"), Color("#E53935C0"), Color("#1E88E5C0"), Color("#43A047C0"), Color("#FB8C00C0"),
			Color("#8E24AAC0"), Color("#00ACC1C0"), Color("#D81B60C0"), Color("#AFB42BC0"), Color("#6D4C41C0")
		};

		static constexpr size_t kMaxUndoHistory = 100;
		stdvec<stdvec<stdvec<bool>>> m_undoStack, m_redoStack;

		bool m_boxSelecting { false };
		Vec2 m_boxSelectStart { 0, 0 };
		Vec2 m_boxSelectCurrent { 0, 0 }; // live drag position - onDraw() renders the box between this and m_boxSelectStart
		bool m_boxSelectSubtract { false };
		bool m_shiftHeld { false };

		bool m_focusModeActive { false };
		f32 m_focusModeAlpha { 0.4f };

		// (u32)-1 for both = not currently hovering any note.
		u32 m_hoveredVoiceIndex { (u32)-1 };
		u32 m_hoveredNoteIndex { (u32)-1 };

		// Scrolling accumulates into a target instead of seeking straight to it, then
		// onUpdate() glides toward that target over a fixed real-time duration (ease-out),
		// instead of an instant jump per wheel tick. The glide is a pure function of wall-clock
		// time elapsed since it started (m_scrollGlideStartValue/StartTimeNs), deliberately
		// never re-reading the live (and, while playing, constantly self-advancing)
		// getCurrentTimeSeconds() as feedback into its own math - doing that previously caused
		// a permanent small oscillation while playing, since the live position advances on its
		// own between frames on top of whatever the easing step already moved it by, so the
		// per-frame "distance remaining" never actually converges to zero.
		bool m_scrollAnimating { false };
		f64 m_scrollTargetSeconds { 0.0 };
		f64 m_scrollGlideStartValue { 0.0 };
		i64 m_scrollGlideStartTimeNs { 0 };

		// Double-click detection - same frame-counted-timer + click-radius pattern as
		// TextEdit::onMousePressed (src/ogfx/gui/widgets/Text.cpp).
		ostd::BasicCounter m_doubleClickTimer { 24 };
		Vec2 m_lastClickPos { 0, 0 };
		bool m_lastClickValid { false };

		bool m_includeSlotZeroInExportAll { false };
		String m_currentProjectPath { "" };
		// Captured from the first voice loaded (export/project save are built around this
		// tool's actual use case - one source performance, split into hand/voice selections
		// from the one file - not arbitrary multi-file mixing, so one shared tick resolution
		// and tempo map for everything exported is the right scope here).
		String m_sourceMidiPath { "" };
		i32 m_ticksPerQuarterNote { 480 };
		stdvec<ostd::MidiParser::TempoEvent> m_tempoMap;
		// The currently-loaded audio file's path - re-read fresh at saveProjectAs() time (not
		// cached bytes from load time), so replacing the file and saving again picks up the
		// new version. Empty if no audio is loaded.
		String m_audioPath { "" };

		Color m_selectionHighlightColor { "#FFD700C0" }; // translucent gold
		Color m_boxSelectColor { "#4AA3FFA0" }; // translucent light blue
		f32 m_scrollSpeedMultiplier { 1.0f };
		f32 m_selectionBorderWidth { 3.0f };

		bool m_showHelpOverlay { true };
		Color m_helpOverlayColor { "#FFFFFF80" }; // description text - white, ~50% alpha ("somewhat low-alpha")
		Color m_helpOverlayKeyColor { "#FFFFFFD0" }; // the keybind itself - brighter/more opaque, to stand apart
};
