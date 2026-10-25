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
// Console example — TextConsoleCanvas + the CRT and bloom effects.
//
// An old green-phosphor PC console: a TextConsoleCanvas (80 x 25 cells of 9 x 16
// pixels, 720 x 400, like VGA text mode) filling the window, with a tiny fake
// command shell. Type commands and press Enter (HELP lists them).
//
//   - The shell only writes cells into its buffer (a stdvec<u32> it owns); the canvas
//     turns them into pixels at its own 30 fps, redrawing only the cells that changed.
//   - Output is "typed out" a few dozen characters per canvas frame, for the feel of a
//     slow terminal (Enter while it prints shows the rest at once).
//   - Scrolling uses the canvas offset: the buffer holds two screens; PgUp / PgDn look
//     back through them.
//
// Keys: F1 CRT effect, F2 bloom, F3 keep aspect ratio, F4 integer scaling,
//       Up / Down command history, PgUp / PgDn scroll back, Esc clears the line.
// The status bar has the same toggles plus canvas statistics.
// ============================================================================

#include <ogfx/ogfx.hpp>
#include <cmath>
#include <ctime>
#include <deque>

using namespace ogfx;
using namespace ogfx::gui;

// Palette entries the shell uses (the rest is the default xterm palette).
namespace pal
{
	constexpr u8 Dim { 1 };
	constexpr u8 Normal { 2 };
	constexpr u8 Bright { 3 };
	constexpr u8 Error { 4 };
	constexpr u8 Cursor { 5 };
	constexpr u8 Black { 6 };
}

static String num(i64 value) { return String("").add(value); }

struct ColorScheme
{
	const char* name;
	Color dim;
	Color normal;
	Color bright;
	Color error;
};

static const ColorScheme ColorSchemes[] = {
	{ "GREEN", {  24,  96,  48 }, {  51, 190,  92 }, { 140, 255, 160 }, { 220, 255, 120 } },
	{ "AMBER", { 110,  60,   8 }, { 230, 150,  30 }, { 255, 210, 110 }, { 255, 110,  60 } },
	{ "WHITE", {  90,  96, 100 }, { 196, 204, 210 }, { 255, 255, 255 }, { 255, 140, 140 } },
	{ "BLUE",  {  30,  70, 130 }, {  90, 170, 255 }, { 190, 225, 255 }, { 255, 160, 120 } },
};

class ConsoleExampleWindow : public Window
{
	public:
		inline ConsoleExampleWindow(void) {  }

		inline void onInitialize(void) override
		{
			m_theme.loadFromFile("./DefaultThemeDark.oss", true, getDefaultStylesheetVariableList());
			setTheme(m_theme);

			m_font.loadFromFile("./ConsoleFont.bmp");     // 16 x 6 glyphs of 11 x 26 pixels
			addWidget(m_console);
			m_console.setAnchorPreset(eAnchorPreset::FullRect);
			m_console.setFont(m_font);
			m_console.setLetterboxColor({ 0, 0, 0 });
			apply_scheme(0);
			// The font's 11 x 26 cells are stretched to 9 x 16: 80 x 25 cells on 720 x 400.
			m_console.enterTextConsoleMode(m_cells, 0, m_palette, CellWidth, CellHeight);
			m_columns = m_console.getColumns();
			m_rows = m_console.getRows();
			m_console.enableCRTEffect();
			m_console.enableBloomEffect();
			m_console.setUpdateCallback([this](PixelCanvas&, f64 dt) { console_frame(dt); });

			build_status_bar();
			boot();
		}

		inline void onSignal(ostd::Signal& signal) override
		{
			if (signal.ID == ostd::BuiltinSignals::TextEntered)
				on_text(((KeyEventData*)signal.userData)->text);
			else if (signal.ID == ostd::BuiltinSignals::KeyPressed)
				on_key(*(KeyEventData*)signal.userData);
		}

		inline void onUpdate(f64 delta) override
		{
			m_statsTimer += delta;
			if (m_statsTimer < 0.25)
				return;
			m_statsTimer = 0.0;
			const IPoint res = m_console.getResolution();
			m_statsLbl.setText(String("Canvas ").add(res.x).add("x").add(res.y)
				.add(" @ ").add(m_console.getMeasuredFrameRate(), 1).add(" fps  |  ")
				.add(m_columns).add("x").add(m_rows).add(" cells, ")
				.add(m_console.getLastRedrawnCellCount()).add(" redrawn  |  ")
				.add(m_console.areEffectsSupported() ? "shaders OK" : "no shader support"));
		}

	private:
		// ================================== CONSOLE OUTPUT =================================
		inline u32 cell_index(u32 line, u32 column) const { return line * m_columns + column; }

		// Writes one character at the cursor (no typing-out delay).
		inline void put(char c, u8 fg = 0, u8 bg = 0)
		{
			hide_cursor();
			if (c == '\n')
			{
				new_line();
				return;
			}
			m_cells[cell_index(m_line, m_column)] = TextConsoleCanvas::makeCell(cast<u8>(c), fg, bg);
			if (++m_column >= m_columns)
				new_line();
		}

		inline void new_line(void)
		{
			m_column = 0;
			m_line++;
			// The buffer holds two screens: when it's full, drop the oldest screen.
			const u32 screen = m_columns * m_rows;
			if (m_line >= m_rows * 2)
			{
				std::copy(m_cells.begin() + screen, m_cells.begin() + screen * 2, m_cells.begin());
				std::fill(m_cells.begin() + screen, m_cells.begin() + screen * 2, 0);
				m_line -= m_rows;
				m_top = m_top >= m_rows ? m_top - m_rows : 0;
			}
			if (m_line >= m_top + m_rows)
				m_top = m_line - m_rows + 1;
			update_offset();
		}

		inline void update_offset(void)
		{
			m_scrollBack = std::min(m_scrollBack, m_top);
			m_console.setOffset((m_top - m_scrollBack) * m_columns);
		}

		// Queued output, typed out by console_frame().
		inline void print(const String& text, u8 fg = 0, u8 bg = 0)
		{
			for (const char c : text.cpp_str())
				m_output.push_back({ c, fg, bg });
		}
		inline void println(const String& text = "", u8 fg = 0, u8 bg = 0) { print(text, fg, bg); print("\n"); }
		inline void prompt(void) { print("C:\\> ", pal::Bright); }

		inline void clear_screen(void)
		{
			hide_cursor();
			std::fill(m_cells.begin(), m_cells.end(), 0);
			m_line = m_column = m_top = m_scrollBack = 0;
			update_offset();
		}

		// The cursor is a cell drawn inverted; the cell under it is saved and put back first
		// whenever the buffer is written.
		inline void show_cursor(void)
		{
			if (m_cursorShown)
				return;
			m_cursorIndex = cell_index(m_line, m_column);
			m_cursorSaved = m_cells[m_cursorIndex];
			const u8 under = TextConsoleCanvas::getCellCharacter(m_cursorSaved);
			m_cells[m_cursorIndex] = TextConsoleCanvas::makeCell(under ? under : ' ', pal::Black, pal::Cursor);
			m_cursorShown = true;
		}

		inline void hide_cursor(void)
		{
			if (!m_cursorShown)
				return;
			m_cells[m_cursorIndex] = m_cursorSaved;
			m_cursorShown = false;
		}

		// Runs at the canvas frame rate, right before the canvas renders the cells.
		inline void console_frame(f64 dt)
		{
			u32 budget = m_flushOutput ? cast<u32>(m_output.size()) : CharsPerFrame;
			while (budget-- > 0 && !m_output.empty())
			{
				const auto out = m_output.front();
				m_output.pop_front();
				put(out.c, out.fg, out.bg);
			}
			if (m_output.empty())
				m_flushOutput = false;

			m_blinkTimer += dt;
			const bool blinkOn = std::fmod(m_blinkTimer, 1.0) < 0.55;
			if (blinkOn && m_scrollBack == 0)
				show_cursor();
			else
				hide_cursor();
		}
		// ==========================================================================



		// ================================== INPUT =================================
		inline void on_text(const String& text)
		{
			if (!m_output.empty())
				return;
			for (const char c : text.cpp_str())
			{
				if (c < 32 || c > 126 || m_input.len() >= MaxInputLength)
					continue;
				m_input += c;
				put(c);
			}
			m_scrollBack = 0;
			m_blinkTimer = 0.0;
			update_offset();
		}

		inline void on_key(const KeyEventData& key)
		{
			switch (key.keyCode)
			{
				case KeyCode::Return:
				case KeyCode::KpEnter:
					if (!m_output.empty())
					{
						m_flushOutput = true;
						return;
					}
					put('\n');
					if (!m_input.new_trim().empty())
					{
						m_history.push_back(m_input);
						if (m_history.size() > 64)
							m_history.erase(m_history.begin());
					}
					m_historyPos = cast<i32>(m_history.size());
					execute(m_input);
					m_input = "";
					prompt();
					break;
				case KeyCode::Backspace:
					if (m_output.empty() && !m_input.empty())
						erase_input_chars(1);
					break;
				case KeyCode::Escape:
					if (m_output.empty())
						erase_input_chars(cast<u32>(m_input.len()));
					break;
				case KeyCode::Up:
					recall_history(-1);
					break;
				case KeyCode::Down:
					recall_history(1);
					break;
				case KeyCode::Pageup:
					m_scrollBack = std::min(m_scrollBack + m_rows / 2, m_top);
					update_offset();
					break;
				case KeyCode::Pagedown:
					m_scrollBack = m_scrollBack > m_rows / 2 ? m_scrollBack - m_rows / 2 : 0;
					update_offset();
					break;
				case KeyCode::F1: m_crtCheck.setChecked(!m_crtCheck.isChecked()); break;
				case KeyCode::F2: m_bloomCheck.setChecked(!m_bloomCheck.isChecked()); break;
				case KeyCode::F3: m_aspectCheck.setChecked(!m_aspectCheck.isChecked()); break;
				case KeyCode::F4: m_integerCheck.setChecked(!m_integerCheck.isChecked()); break;
				default: break;
			}
		}

		inline void erase_input_chars(u32 count)
		{
			hide_cursor();
			for (u32 i = 0; i < count && !m_input.empty(); i++)
			{
				m_input = m_input.new_substr(0, m_input.len() - 1);
				if (m_column == 0)
				{
					if (m_line == 0)
						break;
					m_line--;
					m_column = m_columns - 1;
					if (m_line < m_top)
					{
						m_top = m_line;
						update_offset();
					}
				}
				else
				{
					m_column--;
				}
				m_cells[cell_index(m_line, m_column)] = 0;
			}
			m_blinkTimer = 0.0;
		}

		inline void recall_history(i32 direction)
		{
			if (!m_output.empty() || m_history.empty())
				return;
			m_historyPos = std::clamp(m_historyPos + direction, 0, cast<i32>(m_history.size()));
			erase_input_chars(cast<u32>(m_input.len()));
			if (m_historyPos < cast<i32>(m_history.size()))
				on_text(m_history[m_historyPos]);
		}
		// ==========================================================================



		// ================================== COMMANDS =================================
		inline void execute(const String& line)
		{
			String args = String(line).trim();
			if (args.empty())
				return;
			String command = args;
			const i32 space = args.indexOf(" ");
			if (space >= 0)
			{
				command = args.new_substr(0, space);
				args = args.new_substr(space + 1).trim();
			}
			else
			{
				args = "";
			}
			command.toUpper();
			const String option = String(args).toUpper();

			if (command == "HELP" || command == "?")
			{
				println("Available commands:", pal::Bright);
				println("  HELP              this list");
				println("  CLS               clear the screen");
				println("  ECHO <text>       print text");
				println("  VER               version information");
				println("  TIME, DATE        current time / date");
				println("  DIR               list files");
				println("  TYPE <file>       show a file");
				println("  COLOR <scheme>    GREEN, AMBER, WHITE or BLUE");
				println("  CRT [ON|OFF]      CRT monitor effect          (F1)");
				println("  BLOOM [ON|OFF]    phosphor glow               (F2)");
				println("  PALETTE           the 256-color palette");
				println("  STATS             canvas statistics");
				println("  EXIT              quit");
				println("PgUp/PgDn scroll back, Up/Down recall commands.", pal::Dim);
			}
			else if (command == "CLS")
			{
				m_output.clear();
				clear_screen();
			}
			else if (command == "ECHO")
			{
				println(args);
			}
			else if (command == "VER")
			{
				println("OMNIA-DOS Version 1.0", pal::Bright);
				println("OmniaFramework TextConsoleCanvas demo, " + num(m_columns) + "x" + num(m_rows) + " text mode");
			}
			else if (command == "TIME" || command == "DATE")
			{
				char text[64];
				const std::time_t now = std::time(nullptr);
				std::strftime(text, sizeof(text), command == "TIME" ? "Current time is %H:%M:%S" : "Current date is %a %Y-%m-%d", std::localtime(&now));
				println(text);
			}
			else if (command == "DIR")
			{
				println(" Volume in drive C is OMNIA");
				println(" Directory of C:\\");
				println("");
				println("COMMAND  COM     54,645  10-25-26  12:00a");
				println("CONFIG   SYS        312  10-25-26  12:00a");
				println("README   TXT        871  10-25-26  12:00a");
				println("HELLO    TXT         64  10-25-26  12:00a");
				println("GAMES        <DIR>       10-25-26  12:00a");
				println("        4 file(s)     55,892 bytes", pal::Dim);
				println("        1 dir(s)   1,457,664 bytes free", pal::Dim);
			}
			else if (command == "TYPE")
			{
				if (option == "README.TXT")
				{
					println("This console is an ogfx::gui::TextConsoleCanvas.", pal::Bright);
					println("Every character on screen is one u32 cell in a buffer owned by the");
					println("program: 8 bits of character, 8 bits of foreground and 8 bits of");
					println("background color (indices into a 256-color palette).");
					println("The canvas redraws only the cells that changed, at 30 fps, and the");
					println("CRT and bloom effects run on the GPU when the canvas is drawn.");
				}
				else if (option == "HELLO.TXT")
				{
					println("Hello, world!");
				}
				else if (option == "CONFIG.SYS")
				{
					println("FILES=40");
					println("BUFFERS=30");
					println("DEVICE=C:\\CRT.SYS /SCANLINES /GLOW");
				}
				else
				{
					println(args.empty() ? "Required parameter missing" : "File not found", pal::Error);
				}
			}
			else if (command == "COLOR")
			{
				for (u32 i = 0; i < std::size(ColorSchemes); i++)
				{
					if (option == ColorSchemes[i].name)
					{
						apply_scheme(i);
						return;
					}
				}
				println("Usage: COLOR GREEN | AMBER | WHITE | BLUE", pal::Error);
			}
			else if (command == "CRT" || command == "BLOOM")
			{
				CheckBox& check = command == "CRT" ? m_crtCheck : m_bloomCheck;
				if (option == "ON" || option == "OFF")
					check.setChecked(option == "ON");
				else if (!option.empty())
				{
					println("Usage: " + command + " [ON|OFF]", pal::Error);
					return;
				}
				println(command + " is " + (check.isChecked() ? "ON" : "OFF"));
			}
			else if (command == "PALETTE")
			{
				// 16..255: the 6x6x6 color cube and the gray ramp, as background colors.
				for (u32 i = 16; i < 256; i++)
				{
					print("  ", 0, cast<u8>(i));
					if ((i - 16) % 36 == 35 || i == 255)
						print("\n");
				}
			}
			else if (command == "STATS")
			{
				const IPoint res = m_console.getResolution();
				const IPoint cell = m_console.getCellSize();
				println("Canvas resolution : " + num(res.x) + " x " + num(res.y));
				println("Text grid         : " + num(m_columns) + " x " + num(m_rows) + " cells of " + num(cell.x) + " x " + num(cell.y));
				println("Canvas frame rate : " + String("").add(m_console.getMeasuredFrameRate(), 1) + " fps (target " + String("").add(m_console.getFrameRate(), 0) + ")");
				println("Canvas frames     : " + num((i64)m_console.getCanvasFrameCount()));
				println("Shaders           : " + String(m_console.areEffectsSupported() ? "supported" : "not supported"));
			}
			else if (command == "EXIT")
			{
				close();
			}
			else
			{
				println("Bad command or file name", pal::Error);
			}
		}

		inline void apply_scheme(u32 index)
		{
			const ColorScheme& scheme = ColorSchemes[index];
			m_palette.setColor(pal::Dim, scheme.dim);
			m_palette.setColor(pal::Normal, scheme.normal);
			m_palette.setColor(pal::Bright, scheme.bright);
			m_palette.setColor(pal::Error, scheme.error);
			m_palette.setColor(pal::Cursor, scheme.normal);
			m_palette.setColor(pal::Black, { 0, 0, 0 });
			m_console.setDefaultColors(scheme.normal, { 0, 0, 0 });
		}

		inline void boot(void)
		{
			println("OMNIA BIOS v2.6  (C) 2026 OmniaX-Dev", pal::Bright);
			println("Memory test: 640K OK", pal::Dim);
			println("Video: " + num(m_columns) + "x" + num(m_rows) + " text, 256 colors, CRT emulation", pal::Dim);
			println("");
			println("Starting OMNIA-DOS...");
			println("");
			println("Type HELP for a list of commands.");
			println("");
			prompt();
		}
		// ==========================================================================



		// ================================== STATUS BAR =================================
		inline void build_status_bar(void)
		{
			getStatusBar().setHeight(30);
			showStatusBar(true);
			getStatusBar().addWidget(m_toggles);
			m_toggles.setAnchorPreset(eAnchorPreset::CenterLeft, ePresetResizeMode::KeepSize, 6);
			m_toggles.setSpacing(14);

			m_crtCheck.setText("CRT (F1)");
			m_crtCheck.setChecked(true);
			m_crtCheck.setStateChangedCallback([this](CheckBox&, bool checked) { m_console.enableCRTEffect(checked); });
			m_toggles.addWidget(m_crtCheck);

			m_bloomCheck.setText("Bloom (F2)");
			m_bloomCheck.setChecked(true);
			m_bloomCheck.setStateChangedCallback([this](CheckBox&, bool checked) { m_console.enableBloomEffect(checked); });
			m_toggles.addWidget(m_bloomCheck);

			m_aspectCheck.setText("Keep aspect (F3)");
			m_aspectCheck.setChecked(true);
			m_aspectCheck.setStateChangedCallback([this](CheckBox&, bool checked) { m_console.enableMaintainAspectRatio(checked); });
			m_toggles.addWidget(m_aspectCheck);

			m_integerCheck.setText("Integer scaling (F4)");
			m_integerCheck.setChecked(false);
			m_integerCheck.setStateChangedCallback([this](CheckBox&, bool checked) { m_console.enableIntegerScaling(checked); });
			m_toggles.addWidget(m_integerCheck);

			getStatusBar().addWidget(m_statsLbl);
			m_statsLbl.setAnchorPreset(eAnchorPreset::CenterRight, ePresetResizeMode::KeepSize, 8);
		}
		// ==========================================================================

	private:
		struct Output
		{
			char c;
			u8 fg;
			u8 bg;
		};

		inline static constexpr u8 CellWidth { 9 };
		inline static constexpr u8 CellHeight { 16 };
		inline static constexpr u32 CharsPerFrame { 40 };
		inline static constexpr u32 MaxInputLength { 160 };

		ostd::Stylesheet m_theme;
		PixelFont m_font;
		PixelCanvas::ColorPalette256 m_palette;
		stdvec<u32> m_cells;
		TextConsoleCanvas m_console { *this, 720, 400 };

		u32 m_columns { 0 };
		u32 m_rows { 0 };
		u32 m_line { 0 };        // cursor, in buffer lines
		u32 m_column { 0 };
		u32 m_top { 0 };         // first line of the screen (when not scrolled back)
		u32 m_scrollBack { 0 };  // lines scrolled back with PgUp

		std::deque<Output> m_output;
		bool m_flushOutput { false };
		String m_input { "" };
		stdvec<String> m_history;
		i32 m_historyPos { 0 };

		bool m_cursorShown { false };
		u32 m_cursorIndex { 0 };
		u32 m_cursorSaved { 0 };
		f64 m_blinkTimer { 0.0 };

		HBoxContainer m_toggles { *this };
		CheckBox m_crtCheck { *this };
		CheckBox m_bloomCheck { *this };
		CheckBox m_aspectCheck { *this };
		CheckBox m_integerCheck { *this };
		Label m_statsLbl { *this };
		f64 m_statsTimer { 1.0 };
};

i32 main(i32 argc, char** argv)
{
	ConsoleExampleWindow window;
	window.initialize(1280, 820, "OmniaFramework - Console Example");
	window.setClearColor({ 0, 0, 0 });
	window.mainLoop();
	return 0;
}
