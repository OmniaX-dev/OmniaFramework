/*
    OmniaFramework - A collection of useful functionality
    Copyright (C) 2025  OmniaX-Dev

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

#pragma once

#include <ogfx/utils/SDLInclude.hpp>

namespace ogfx
{
	struct KeyCode
	{
  inline static constexpr u32 Unknown { SDLK_UNKNOWN };
  inline static constexpr u32 Return { SDLK_RETURN };
  inline static constexpr u32 Escape { SDLK_ESCAPE };
  inline static constexpr u32 Backspace { SDLK_BACKSPACE };
  inline static constexpr u32 Tab { SDLK_TAB };
  inline static constexpr u32 Space { SDLK_SPACE };
  inline static constexpr u32 Exclaim { SDLK_EXCLAIM };
  inline static constexpr u32 Dblapostrophe { SDLK_DBLAPOSTROPHE };
  inline static constexpr u32 Hash { SDLK_HASH };
  inline static constexpr u32 Dollar { SDLK_DOLLAR };
  inline static constexpr u32 Percent { SDLK_PERCENT };
  inline static constexpr u32 Ampersand { SDLK_AMPERSAND };
  inline static constexpr u32 Apostrophe { SDLK_APOSTROPHE };
  inline static constexpr u32 Leftparen { SDLK_LEFTPAREN };
  inline static constexpr u32 Rightparen { SDLK_RIGHTPAREN };
  inline static constexpr u32 Asterisk { SDLK_ASTERISK };
  inline static constexpr u32 Plus { SDLK_PLUS };
  inline static constexpr u32 Comma { SDLK_COMMA };
  inline static constexpr u32 Minus { SDLK_MINUS };
  inline static constexpr u32 Period { SDLK_PERIOD };
  inline static constexpr u32 Slash { SDLK_SLASH };
  inline static constexpr u32 Num0 { SDLK_0 };
  inline static constexpr u32 Num1 { SDLK_1 };
  inline static constexpr u32 Num2 { SDLK_2 };
  inline static constexpr u32 Num3 { SDLK_3 };
  inline static constexpr u32 Num4 { SDLK_4 };
  inline static constexpr u32 Num5 { SDLK_5 };
  inline static constexpr u32 Num6 { SDLK_6 };
  inline static constexpr u32 Num7 { SDLK_7 };
  inline static constexpr u32 Num8 { SDLK_8 };
  inline static constexpr u32 Num9 { SDLK_9 };
  inline static constexpr u32 Colon { SDLK_COLON };
  inline static constexpr u32 Semicolon { SDLK_SEMICOLON };
  inline static constexpr u32 Less { SDLK_LESS };
  inline static constexpr u32 Equals { SDLK_EQUALS };
  inline static constexpr u32 Greater { SDLK_GREATER };
  inline static constexpr u32 Question { SDLK_QUESTION };
  inline static constexpr u32 At { SDLK_AT };
  inline static constexpr u32 LeftBracket { SDLK_LEFTBRACKET };
  inline static constexpr u32 Backslash { SDLK_BACKSLASH };
  inline static constexpr u32 RightBracket { SDLK_RIGHTBRACKET };
  inline static constexpr u32 Caret { SDLK_CARET };
  inline static constexpr u32 Underscore { SDLK_UNDERSCORE };
  inline static constexpr u32 Grave { SDLK_GRAVE };
  inline static constexpr u32 A { SDLK_A };
  inline static constexpr u32 B { SDLK_B };
  inline static constexpr u32 C { SDLK_C };
  inline static constexpr u32 D { SDLK_D };
  inline static constexpr u32 E { SDLK_E };
  inline static constexpr u32 F { SDLK_F };
  inline static constexpr u32 G { SDLK_G };
  inline static constexpr u32 H { SDLK_H };
  inline static constexpr u32 I { SDLK_I };
  inline static constexpr u32 J { SDLK_J };
  inline static constexpr u32 K { SDLK_K };
  inline static constexpr u32 L { SDLK_L };
  inline static constexpr u32 M { SDLK_M };
  inline static constexpr u32 N { SDLK_N };
  inline static constexpr u32 O { SDLK_O };
  inline static constexpr u32 P { SDLK_P };
  inline static constexpr u32 Q { SDLK_Q };
  inline static constexpr u32 R { SDLK_R };
  inline static constexpr u32 S { SDLK_S };
  inline static constexpr u32 T { SDLK_T };
  inline static constexpr u32 U { SDLK_U };
  inline static constexpr u32 V { SDLK_V };
  inline static constexpr u32 W { SDLK_W };
  inline static constexpr u32 X { SDLK_X };
  inline static constexpr u32 Y { SDLK_Y };
  inline static constexpr u32 Z { SDLK_Z };
  inline static constexpr u32 Leftbrace { SDLK_LEFTBRACE };
  inline static constexpr u32 Pipe { SDLK_PIPE };
  inline static constexpr u32 Rightbrace { SDLK_RIGHTBRACE };
  inline static constexpr u32 Tilde { SDLK_TILDE };
  inline static constexpr u32 Delete { SDLK_DELETE };
  inline static constexpr u32 Plusminus { SDLK_PLUSMINUS };
  inline static constexpr u32 Capslock { SDLK_CAPSLOCK };
  inline static constexpr u32 F1 { SDLK_F1 };
  inline static constexpr u32 F2 { SDLK_F2 };
  inline static constexpr u32 F3 { SDLK_F3 };
  inline static constexpr u32 F4 { SDLK_F4 };
  inline static constexpr u32 F5 { SDLK_F5 };
  inline static constexpr u32 F6 { SDLK_F6 };
  inline static constexpr u32 F7 { SDLK_F7 };
  inline static constexpr u32 F8 { SDLK_F8 };
  inline static constexpr u32 F9 { SDLK_F9 };
  inline static constexpr u32 F10 { SDLK_F10 };
  inline static constexpr u32 F11 { SDLK_F11 };
  inline static constexpr u32 F12 { SDLK_F12 };
  inline static constexpr u32 Printscreen { SDLK_PRINTSCREEN };
  inline static constexpr u32 Scrolllock { SDLK_SCROLLLOCK };
  inline static constexpr u32 Pause { SDLK_PAUSE };
  inline static constexpr u32 Insert { SDLK_INSERT };
  inline static constexpr u32 Home { SDLK_HOME };
  inline static constexpr u32 Pageup { SDLK_PAGEUP };
  inline static constexpr u32 End { SDLK_END };
  inline static constexpr u32 Pagedown { SDLK_PAGEDOWN };
  inline static constexpr u32 Right { SDLK_RIGHT };
  inline static constexpr u32 Left { SDLK_LEFT };
  inline static constexpr u32 Down { SDLK_DOWN };
  inline static constexpr u32 Up { SDLK_UP };
  inline static constexpr u32 Numlockclear { SDLK_NUMLOCKCLEAR };
  inline static constexpr u32 KpDivide { SDLK_KP_DIVIDE };
  inline static constexpr u32 KpMultiply { SDLK_KP_MULTIPLY };
  inline static constexpr u32 KpMinus { SDLK_KP_MINUS };
  inline static constexpr u32 KpPlus { SDLK_KP_PLUS };
  inline static constexpr u32 KpEnter { SDLK_KP_ENTER };
  inline static constexpr u32 Kp1 { SDLK_KP_1 };
  inline static constexpr u32 Kp2 { SDLK_KP_2 };
  inline static constexpr u32 Kp3 { SDLK_KP_3 };
  inline static constexpr u32 Kp4 { SDLK_KP_4 };
  inline static constexpr u32 Kp5 { SDLK_KP_5 };
  inline static constexpr u32 Kp6 { SDLK_KP_6 };
  inline static constexpr u32 Kp7 { SDLK_KP_7 };
  inline static constexpr u32 Kp8 { SDLK_KP_8 };
  inline static constexpr u32 Kp9 { SDLK_KP_9 };
  inline static constexpr u32 Kp0 { SDLK_KP_0 };
  inline static constexpr u32 KpPeriod { SDLK_KP_PERIOD };
  inline static constexpr u32 Application { SDLK_APPLICATION };
  inline static constexpr u32 Power { SDLK_POWER };
  inline static constexpr u32 KpEquals { SDLK_KP_EQUALS };
  inline static constexpr u32 F13 { SDLK_F13 };
  inline static constexpr u32 F14 { SDLK_F14 };
  inline static constexpr u32 F15 { SDLK_F15 };
  inline static constexpr u32 F16 { SDLK_F16 };
  inline static constexpr u32 F17 { SDLK_F17 };
  inline static constexpr u32 F18 { SDLK_F18 };
  inline static constexpr u32 F19 { SDLK_F19 };
  inline static constexpr u32 F20 { SDLK_F20 };
  inline static constexpr u32 F21 { SDLK_F21 };
  inline static constexpr u32 F22 { SDLK_F22 };
  inline static constexpr u32 F23 { SDLK_F23 };
  inline static constexpr u32 F24 { SDLK_F24 };
  inline static constexpr u32 Execute { SDLK_EXECUTE };
  inline static constexpr u32 Help { SDLK_HELP };
  inline static constexpr u32 Menu { SDLK_MENU };
  inline static constexpr u32 Select { SDLK_SELECT };
  inline static constexpr u32 Stop { SDLK_STOP };
  inline static constexpr u32 Again { SDLK_AGAIN };
  inline static constexpr u32 Undo { SDLK_UNDO };
  inline static constexpr u32 Cut { SDLK_CUT };
  inline static constexpr u32 Copy { SDLK_COPY };
  inline static constexpr u32 Paste { SDLK_PASTE };
  inline static constexpr u32 Find { SDLK_FIND };
  inline static constexpr u32 Mute { SDLK_MUTE };
  inline static constexpr u32 Volumeup { SDLK_VOLUMEUP };
  inline static constexpr u32 Volumedown { SDLK_VOLUMEDOWN };
  inline static constexpr u32 KpComma { SDLK_KP_COMMA };
  inline static constexpr u32 KpEqualsas400 { SDLK_KP_EQUALSAS400 };
  inline static constexpr u32 Alterase { SDLK_ALTERASE };
  inline static constexpr u32 Sysreq { SDLK_SYSREQ };
  inline static constexpr u32 Cancel { SDLK_CANCEL };
  inline static constexpr u32 Clear { SDLK_CLEAR };
  inline static constexpr u32 Prior { SDLK_PRIOR };
  inline static constexpr u32 Return2 { SDLK_RETURN2 };
  inline static constexpr u32 Separator { SDLK_SEPARATOR };
  inline static constexpr u32 Out { SDLK_OUT };
  inline static constexpr u32 Oper { SDLK_OPER };
  inline static constexpr u32 Clearagain { SDLK_CLEARAGAIN };
  inline static constexpr u32 Crsel { SDLK_CRSEL };
  inline static constexpr u32 Exsel { SDLK_EXSEL };
  inline static constexpr u32 Kp00 { SDLK_KP_00 };
  inline static constexpr u32 Kp000 { SDLK_KP_000 };
  inline static constexpr u32 Thousandsseparator { SDLK_THOUSANDSSEPARATOR };
  inline static constexpr u32 Decimalseparator { SDLK_DECIMALSEPARATOR };
  inline static constexpr u32 Currencyunit { SDLK_CURRENCYUNIT };
  inline static constexpr u32 Currencysubunit { SDLK_CURRENCYSUBUNIT };
  inline static constexpr u32 KpLeftparen { SDLK_KP_LEFTPAREN };
  inline static constexpr u32 KpRightparen { SDLK_KP_RIGHTPAREN };
  inline static constexpr u32 KpLeftbrace { SDLK_KP_LEFTBRACE };
  inline static constexpr u32 KpRightbrace { SDLK_KP_RIGHTBRACE };
  inline static constexpr u32 KpTab { SDLK_KP_TAB };
  inline static constexpr u32 KpBackspace { SDLK_KP_BACKSPACE };
  inline static constexpr u32 KpA { SDLK_KP_A };
  inline static constexpr u32 KpB { SDLK_KP_B };
  inline static constexpr u32 KpC { SDLK_KP_C };
  inline static constexpr u32 KpD { SDLK_KP_D };
  inline static constexpr u32 KpE { SDLK_KP_E };
  inline static constexpr u32 KpF { SDLK_KP_F };
  inline static constexpr u32 KpXor { SDLK_KP_XOR };
  inline static constexpr u32 KpPower { SDLK_KP_POWER };
  inline static constexpr u32 KpPercent { SDLK_KP_PERCENT };
  inline static constexpr u32 KpLess { SDLK_KP_LESS };
  inline static constexpr u32 KpGreater { SDLK_KP_GREATER };
  inline static constexpr u32 KpAmpersand { SDLK_KP_AMPERSAND };
  inline static constexpr u32 KpDblampersand { SDLK_KP_DBLAMPERSAND };
  inline static constexpr u32 KpVerticalbar { SDLK_KP_VERTICALBAR };
  inline static constexpr u32 KpDblverticalbar { SDLK_KP_DBLVERTICALBAR };
  inline static constexpr u32 KpColon { SDLK_KP_COLON };
  inline static constexpr u32 KpHash { SDLK_KP_HASH };
  inline static constexpr u32 KpSpace { SDLK_KP_SPACE };
  inline static constexpr u32 KpAt { SDLK_KP_AT };
  inline static constexpr u32 KpExclam { SDLK_KP_EXCLAM };
  inline static constexpr u32 KpMemstore { SDLK_KP_MEMSTORE };
  inline static constexpr u32 KpMemrecall { SDLK_KP_MEMRECALL };
  inline static constexpr u32 KpMemclear { SDLK_KP_MEMCLEAR };
  inline static constexpr u32 KpMemadd { SDLK_KP_MEMADD };
  inline static constexpr u32 KpMemsubtract { SDLK_KP_MEMSUBTRACT };
  inline static constexpr u32 KpMemmultiply { SDLK_KP_MEMMULTIPLY };
  inline static constexpr u32 KpMemdivide { SDLK_KP_MEMDIVIDE };
  inline static constexpr u32 KpPlusminus { SDLK_KP_PLUSMINUS };
  inline static constexpr u32 KpClear { SDLK_KP_CLEAR };
  inline static constexpr u32 KpClearentry { SDLK_KP_CLEARENTRY };
  inline static constexpr u32 KpBinary { SDLK_KP_BINARY };
  inline static constexpr u32 KpOctal { SDLK_KP_OCTAL };
  inline static constexpr u32 KpDecimal { SDLK_KP_DECIMAL };
  inline static constexpr u32 KpHexadecimal { SDLK_KP_HEXADECIMAL };
  inline static constexpr u32 Lctrl { SDLK_LCTRL };
  inline static constexpr u32 Lshift { SDLK_LSHIFT };
  inline static constexpr u32 Lalt { SDLK_LALT };
  inline static constexpr u32 Lgui { SDLK_LGUI };
  inline static constexpr u32 Rctrl { SDLK_RCTRL };
  inline static constexpr u32 Rshift { SDLK_RSHIFT };
  inline static constexpr u32 Ralt { SDLK_RALT };
  inline static constexpr u32 Rgui { SDLK_RGUI };
  inline static constexpr u32 Mode { SDLK_MODE };
  inline static constexpr u32 Sleep { SDLK_SLEEP };
  inline static constexpr u32 Wake { SDLK_WAKE };
  inline static constexpr u32 ChannelIncrement { SDLK_CHANNEL_INCREMENT };
  inline static constexpr u32 ChannelDecrement { SDLK_CHANNEL_DECREMENT };
  inline static constexpr u32 MediaPlay { SDLK_MEDIA_PLAY };
  inline static constexpr u32 MediaPause { SDLK_MEDIA_PAUSE };
  inline static constexpr u32 MediaRecord { SDLK_MEDIA_RECORD };
  inline static constexpr u32 MediaFastForward { SDLK_MEDIA_FAST_FORWARD };
  inline static constexpr u32 MediaRewind { SDLK_MEDIA_REWIND };
  inline static constexpr u32 MediaNextTrack { SDLK_MEDIA_NEXT_TRACK };
  inline static constexpr u32 MediaPreviousTrack { SDLK_MEDIA_PREVIOUS_TRACK };
  inline static constexpr u32 MediaStop { SDLK_MEDIA_STOP };
  inline static constexpr u32 MediaEject { SDLK_MEDIA_EJECT };
  inline static constexpr u32 MediaPlayPause { SDLK_MEDIA_PLAY_PAUSE };
  inline static constexpr u32 MediaSelect { SDLK_MEDIA_SELECT };
  inline static constexpr u32 AcNew { SDLK_AC_NEW };
  inline static constexpr u32 AcOpen { SDLK_AC_OPEN };
  inline static constexpr u32 AcClose { SDLK_AC_CLOSE };
  inline static constexpr u32 AcExit { SDLK_AC_EXIT };
  inline static constexpr u32 AcSave { SDLK_AC_SAVE };
  inline static constexpr u32 AcPrint { SDLK_AC_PRINT };
  inline static constexpr u32 AcProperties { SDLK_AC_PROPERTIES };
  inline static constexpr u32 AcSearch { SDLK_AC_SEARCH };
  inline static constexpr u32 AcHome { SDLK_AC_HOME };
  inline static constexpr u32 AcBack { SDLK_AC_BACK };
  inline static constexpr u32 AcForward { SDLK_AC_FORWARD };
  inline static constexpr u32 AcStop { SDLK_AC_STOP };
  inline static constexpr u32 AcRefresh { SDLK_AC_REFRESH };
  inline static constexpr u32 AcBookmarks { SDLK_AC_BOOKMARKS };
  inline static constexpr u32 Softleft { SDLK_SOFTLEFT };
  inline static constexpr u32 Softright { SDLK_SOFTRIGHT };
  inline static constexpr u32 Call { SDLK_CALL };
  inline static constexpr u32 Endcall { SDLK_ENDCALL };
  inline static constexpr u32 LeftTab { SDLK_LEFT_TAB };
  inline static constexpr u32 Level5Shift { SDLK_LEVEL5_SHIFT };
  inline static constexpr u32 MultiKeyCompose { SDLK_MULTI_KEY_COMPOSE };
  inline static constexpr u32 Lmeta { SDLK_LMETA };
  inline static constexpr u32 Rmeta { SDLK_RMETA };
  inline static constexpr u32 Lhyper { SDLK_LHYPER };
  inline static constexpr u32 Rhyper { SDLK_RHYPER };
	};
}
