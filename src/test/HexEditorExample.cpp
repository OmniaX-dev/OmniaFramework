/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

	This file is part of OmniaFramework.

	OmniaFramework is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	OmniaFramework is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with OmniaFramework.  If not, see <https://www.gnu.org/licenses/>.
*/

// ============================================================================
// HexEditor example — inspecting the memory of a (fake) running machine.
//
// A toy "CPU" scribbles over 1 MiB of RAM: the program counter walks the code
// region (the fixed highlight, try "Follow"), registers and a counter change,
// the stack grows and shrinks. Named highlights mark the regions, the "r1"
// register is watched from the start: every change arrives as the
// HexEditor::WatchValueChangedSignal signal (see onSignal()). A few aliases
// name byte patterns ("B1" = ret, "F1 ??" = int, ...) in the code region.
//
// Toolbar: run / pause the machine and single-step it.
// ============================================================================

#include <ogfx/ogfx.hpp>

using namespace ogfx;
using namespace ogfx::gui;

class FakeMachine
{
	public:
		inline static constexpr StreamIndex CodeStart { 0x0000 };
		inline static constexpr StreamIndex CodeSize { 0x1000 };
		inline static constexpr StreamIndex TextStart { 0x1000 };
		inline static constexpr StreamIndex RegsStart { 0x2000 };
		inline static constexpr StreamIndex CounterAddr { 0x2020 };
		inline static constexpr StreamIndex StackTop { 0x10000 };
		inline static constexpr StreamIndex VideoStart { 0x20000 };
		inline static constexpr StreamIndex VideoSize { 0x400 };
		inline static constexpr u32 MemorySize { 0x100000 };

	public:
		inline FakeMachine(void) : memory(MemorySize, 0)
		{
			for (StreamIndex i = CodeStart; i < CodeStart + CodeSize; i++)
				memory[i] = cast<i8>(next_random() & 0xFF);
			const char* text = "Hello from the fantasy VM! This text lives at 0x1000.";
			for (u32 i = 0; text[i] != 0; i++)
				memory[TextStart + i] = text[i];
		}

		inline void step(void)
		{
			opLength = 1 + (next_random() % 4);
			pc = (pc + opLength) % CodeSize;
			// r0 changes every step, r1 every 16 steps, r2..r7 now and then.
			write32(RegsStart, read32(RegsStart) + opLength);
			if (++steps % 16 == 0)
				write32(RegsStart + 4, read32(RegsStart + 4) + 1);
			if (next_random() % 8 == 0)
				write32(RegsStart + 8 + (next_random() % 6) * 4, next_random());
			write32(CounterAddr, cast<u32>(steps));
			// Push/pop on the stack.
			if (next_random() % 3 == 0 && sp > StackTop - 0x100)
				memory[--sp] = cast<i8>(next_random() & 0xFF);
			else if (next_random() % 4 == 0 && sp < StackTop)
				memory[sp++] = 0;
			// "Video" noise.
			memory[VideoStart + (next_random() % VideoSize)] = cast<i8>(next_random() & 0xFF);
		}

		inline u32 read32(StreamIndex at) const
		{
			return cast<u8>(memory[at]) | (cast<u8>(memory[at + 1]) << 8) | (cast<u8>(memory[at + 2]) << 16) | (cast<u32>(cast<u8>(memory[at + 3])) << 24);
		}

		inline void write32(StreamIndex at, u32 value)
		{
			for (u32 i = 0; i < 4; i++)
				memory[at + i] = cast<i8>((value >> (8 * i)) & 0xFF);
		}

	private:
		inline u32 next_random(void)
		{
			m_seed ^= m_seed << 13;
			m_seed ^= m_seed >> 17;
			m_seed ^= m_seed << 5;
			return m_seed;
		}

	public:
		stdvec<i8> memory;
		StreamIndex pc { 0 };
		u32 opLength { 1 };
		StreamIndex sp { StackTop };
		u64 steps { 0 };

	private:
		u32 m_seed { 0x12345678 };
};

class HexEditorExampleWindow : public Window
{
	public:
		inline void onInitialize(void) override
		{
			m_theme.loadFromFile("./DefaultThemeDark.oss", true, getDefaultStylesheetVariableList());
			setTheme(m_theme);

			addWidget(m_editor);
			m_editor.setAnchorPreset(eAnchorPreset::FullRect, ePresetResizeMode::KeepSize, 6);
			// Like Memory::printByteStream(): data, start, bytes per row (0 = fit),
			// rows (0 = fit), fixed highlight + range, title.
			m_editor.setData(std::span<i8>(m_vm.memory), 0, 0, 0, cast<i32>(m_vm.pc), m_vm.opLength, "RAM");
			m_editor.addHighlight(FakeMachine::CodeStart, FakeMachine::CodeSize, Color(40, 80, 160, 60), "code");
			m_editor.addHighlight(FakeMachine::TextStart, 53, Color(160, 120, 20, 60), "text");
			m_editor.addHighlight(FakeMachine::RegsStart, 32, Color(150, 60, 170, 90), "registers r0-r7");
			m_editor.addHighlight(FakeMachine::CounterAddr, 4, Color(60, 160, 60, 90), "step counter");
			m_stackHighlight = m_editor.addHighlight(m_vm.sp, 1, Color(40, 160, 100, 70), "stack");
			m_editor.addHighlight(FakeMachine::VideoStart, FakeMachine::VideoSize, Color(120, 120, 120, 40), "video");
			m_editor.addWatch(FakeMachine::RegsStart + 4, 4, "r1");
			// Names for byte patterns (longest match wins, ?? = any byte): their bytes
			// are drawn in the alias color and the status bar names them.
			m_editor.addAlias("B1", "ret", Color(255, 120, 120));
			m_editor.addAlias("B1 50", "ret 0x50", Color(255, 60, 160));
			m_editor.addAlias("F1 ??", "int", Color(120, 200, 255));
			m_editor.addAlias("F1 0D", "MemMap Int Vector", Color(120, 255, 140));
			// Watch changes come as a signal (any Object can connect to it).
			ostd::SignalHandler::connect(*this, HexEditor::WatchValueChangedSignal);

			build_bars();
		}

		inline void onUpdate(f64 delta) override
		{
			if (m_running)
			{
				m_stepAccumulator += delta * 20.0;
				while (m_stepAccumulator >= 1.0)
				{
					m_vm.step();
					m_stepAccumulator -= 1.0;
				}
			}
			sync_editor();
		}

		inline void onSignal(ostd::Signal& signal) override
		{
			if (signal.ID != HexEditor::WatchValueChangedSignal || signal.userData == nullptr)
				return;
			auto& evt = *static_cast<HexEditor::WatchEvent*>(signal.userData);
			m_watchEvents++;
			m_watchLabel.setText(String("Watch \"").add(evt.label).add("\" at ").add(evt.editor->formatAddress(evt.address))
				.add(": ").add(std::to_string(evt.oldValue)).add(" -> ").add(std::to_string(evt.newValue))
				.add("   (").add(std::to_string(m_watchEvents)).add(" changes)"));
		}

	private:
		inline void sync_editor(void)
		{
			// Cheap per-frame updates: they don't move the view (setData() would).
			m_editor.setHighlight(cast<i32>(m_vm.pc), m_vm.opLength);
			m_editor.updateHighlight(m_stackHighlight, m_vm.sp, std::max<u32>(1, FakeMachine::StackTop - m_vm.sp));
			m_pcLabel.setText(String("PC ").add(m_editor.formatAddress(m_vm.pc)).add("  SP ").add(m_editor.formatAddress(m_vm.sp))
				.add("  steps ").add(std::to_string(m_vm.steps)));
		}

		inline void build_bars(void)
		{
			auto& tb = getToolBar();
			m_runCheck.setText("Run");
			m_runCheck.setChecked(true);
			m_runCheck.setStateChangedCallback([this](CheckBox&, bool checked) { m_running = checked; });
			tb.addWidget(m_runCheck);
			m_runCheck.setAnchorPreset(eAnchorPreset::CenterLeft, ePresetResizeMode::KeepSize, 8);
			m_stepButton.setText("Step");
			m_stepButton.setCallback(Widget::eCallback::ActionPerformed, [this](const Event&) { m_vm.step(); sync_editor(); });
			tb.addWidget(m_stepButton);
			m_stepButton.setAnchorPreset(eAnchorPreset::CenterLeft, ePresetResizeMode::KeepSize, 90);
			tb.addWidget(m_pcLabel);
			m_pcLabel.setAnchorPreset(eAnchorPreset::CenterRight, ePresetResizeMode::KeepSize, 10);
			showToolBar(true);

			getStatusBar().setHeight(28);
			getStatusBar().addWidget(m_watchLabel);
			m_watchLabel.setText("No watch changes yet");
			m_watchLabel.setAnchorPreset(eAnchorPreset::CenterLeft, ePresetResizeMode::KeepSize, 6);
			showStatusBar(true);
		}

	private:
		ostd::Stylesheet m_theme;
		FakeMachine m_vm;
		HexEditor m_editor { *this };
		i32 m_stackHighlight { -1 };
		bool m_running { true };
		f64 m_stepAccumulator { 0.0 };
		u64 m_watchEvents { 0 };
		CheckBox m_runCheck { *this };
		Button m_stepButton { *this };
		Label m_pcLabel { *this };
		Label m_watchLabel { *this };
};

i32 main(i32 argc, char** argv)
{
	HexEditorExampleWindow window;
	window.initialize(1400, 860, "OmniaFramework - HexEditor Example");
	window.setClearColor({ 0, 0, 0 });
	window.mainLoop();
	return 0;
}
