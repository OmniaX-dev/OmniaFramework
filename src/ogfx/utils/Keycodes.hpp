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
	    inline static constexpr uint32_t Unknown { SDLK_UNKNOWN };
	    inline static constexpr uint32_t Return { SDLK_RETURN };
	    inline static constexpr uint32_t Escape { SDLK_ESCAPE };
	    inline static constexpr uint32_t Backspace { SDLK_BACKSPACE };
	    inline static constexpr uint32_t Tab { SDLK_TAB };
	    inline static constexpr uint32_t Space { SDLK_SPACE };
	    inline static constexpr uint32_t Exclaim { SDLK_EXCLAIM };
	    inline static constexpr uint32_t Dblapostrophe { SDLK_DBLAPOSTROPHE };
	    inline static constexpr uint32_t Hash { SDLK_HASH };
	    inline static constexpr uint32_t Dollar { SDLK_DOLLAR };
	    inline static constexpr uint32_t Percent { SDLK_PERCENT };
	    inline static constexpr uint32_t Ampersand { SDLK_AMPERSAND };
	    inline static constexpr uint32_t Apostrophe { SDLK_APOSTROPHE };
	    inline static constexpr uint32_t Leftparen { SDLK_LEFTPAREN };
	    inline static constexpr uint32_t Rightparen { SDLK_RIGHTPAREN };
	    inline static constexpr uint32_t Asterisk { SDLK_ASTERISK };
	    inline static constexpr uint32_t Plus { SDLK_PLUS };
	    inline static constexpr uint32_t Comma { SDLK_COMMA };
	    inline static constexpr uint32_t Minus { SDLK_MINUS };
	    inline static constexpr uint32_t Period { SDLK_PERIOD };
	    inline static constexpr uint32_t Slash { SDLK_SLASH };
	    inline static constexpr uint32_t Num0 { SDLK_0 };
	    inline static constexpr uint32_t Num1 { SDLK_1 };
	    inline static constexpr uint32_t Num2 { SDLK_2 };
	    inline static constexpr uint32_t Num3 { SDLK_3 };
	    inline static constexpr uint32_t Num4 { SDLK_4 };
	    inline static constexpr uint32_t Num5 { SDLK_5 };
	    inline static constexpr uint32_t Num6 { SDLK_6 };
	    inline static constexpr uint32_t Num7 { SDLK_7 };
	    inline static constexpr uint32_t Num8 { SDLK_8 };
	    inline static constexpr uint32_t Num9 { SDLK_9 };
	    inline static constexpr uint32_t Colon { SDLK_COLON };
	    inline static constexpr uint32_t Semicolon { SDLK_SEMICOLON };
	    inline static constexpr uint32_t Less { SDLK_LESS };
	    inline static constexpr uint32_t Equals { SDLK_EQUALS };
	    inline static constexpr uint32_t Greater { SDLK_GREATER };
	    inline static constexpr uint32_t Question { SDLK_QUESTION };
	    inline static constexpr uint32_t At { SDLK_AT };
	    inline static constexpr uint32_t LeftBracket { SDLK_LEFTBRACKET };
	    inline static constexpr uint32_t Backslash { SDLK_BACKSLASH };
	    inline static constexpr uint32_t RightBracket { SDLK_RIGHTBRACKET };
	    inline static constexpr uint32_t Caret { SDLK_CARET };
	    inline static constexpr uint32_t Underscore { SDLK_UNDERSCORE };
	    inline static constexpr uint32_t Grave { SDLK_GRAVE };
	    inline static constexpr uint32_t A { SDLK_A };
	    inline static constexpr uint32_t B { SDLK_B };
	    inline static constexpr uint32_t C { SDLK_C };
	    inline static constexpr uint32_t D { SDLK_D };
	    inline static constexpr uint32_t E { SDLK_E };
	    inline static constexpr uint32_t F { SDLK_F };
	    inline static constexpr uint32_t G { SDLK_G };
	    inline static constexpr uint32_t H { SDLK_H };
	    inline static constexpr uint32_t I { SDLK_I };
	    inline static constexpr uint32_t J { SDLK_J };
	    inline static constexpr uint32_t K { SDLK_K };
	    inline static constexpr uint32_t L { SDLK_L };
	    inline static constexpr uint32_t M { SDLK_M };
	    inline static constexpr uint32_t N { SDLK_N };
	    inline static constexpr uint32_t O { SDLK_O };
	    inline static constexpr uint32_t P { SDLK_P };
	    inline static constexpr uint32_t Q { SDLK_Q };
	    inline static constexpr uint32_t R { SDLK_R };
	    inline static constexpr uint32_t S { SDLK_S };
	    inline static constexpr uint32_t T { SDLK_T };
	    inline static constexpr uint32_t U { SDLK_U };
	    inline static constexpr uint32_t V { SDLK_V };
	    inline static constexpr uint32_t W { SDLK_W };
	    inline static constexpr uint32_t X { SDLK_X };
	    inline static constexpr uint32_t Y { SDLK_Y };
	    inline static constexpr uint32_t Z { SDLK_Z };
	    inline static constexpr uint32_t Leftbrace { SDLK_LEFTBRACE };
	    inline static constexpr uint32_t Pipe { SDLK_PIPE };
	    inline static constexpr uint32_t Rightbrace { SDLK_RIGHTBRACE };
	    inline static constexpr uint32_t Tilde { SDLK_TILDE };
	    inline static constexpr uint32_t Delete { SDLK_DELETE };
	    inline static constexpr uint32_t Plusminus { SDLK_PLUSMINUS };
	    inline static constexpr uint32_t Capslock { SDLK_CAPSLOCK };
	    inline static constexpr uint32_t F1 { SDLK_F1 };
	    inline static constexpr uint32_t F2 { SDLK_F2 };
	    inline static constexpr uint32_t F3 { SDLK_F3 };
	    inline static constexpr uint32_t F4 { SDLK_F4 };
	    inline static constexpr uint32_t F5 { SDLK_F5 };
	    inline static constexpr uint32_t F6 { SDLK_F6 };
	    inline static constexpr uint32_t F7 { SDLK_F7 };
	    inline static constexpr uint32_t F8 { SDLK_F8 };
	    inline static constexpr uint32_t F9 { SDLK_F9 };
	    inline static constexpr uint32_t F10 { SDLK_F10 };
	    inline static constexpr uint32_t F11 { SDLK_F11 };
	    inline static constexpr uint32_t F12 { SDLK_F12 };
	    inline static constexpr uint32_t Printscreen { SDLK_PRINTSCREEN };
	    inline static constexpr uint32_t Scrolllock { SDLK_SCROLLLOCK };
	    inline static constexpr uint32_t Pause { SDLK_PAUSE };
	    inline static constexpr uint32_t Insert { SDLK_INSERT };
	    inline static constexpr uint32_t Home { SDLK_HOME };
	    inline static constexpr uint32_t Pageup { SDLK_PAGEUP };
	    inline static constexpr uint32_t End { SDLK_END };
	    inline static constexpr uint32_t Pagedown { SDLK_PAGEDOWN };
	    inline static constexpr uint32_t Right { SDLK_RIGHT };
	    inline static constexpr uint32_t Left { SDLK_LEFT };
	    inline static constexpr uint32_t Down { SDLK_DOWN };
	    inline static constexpr uint32_t Up { SDLK_UP };
	    inline static constexpr uint32_t Numlockclear { SDLK_NUMLOCKCLEAR };
	    inline static constexpr uint32_t KpDivide { SDLK_KP_DIVIDE };
	    inline static constexpr uint32_t KpMultiply { SDLK_KP_MULTIPLY };
	    inline static constexpr uint32_t KpMinus { SDLK_KP_MINUS };
	    inline static constexpr uint32_t KpPlus { SDLK_KP_PLUS };
	    inline static constexpr uint32_t KpEnter { SDLK_KP_ENTER };
	    inline static constexpr uint32_t Kp1 { SDLK_KP_1 };
	    inline static constexpr uint32_t Kp2 { SDLK_KP_2 };
	    inline static constexpr uint32_t Kp3 { SDLK_KP_3 };
	    inline static constexpr uint32_t Kp4 { SDLK_KP_4 };
	    inline static constexpr uint32_t Kp5 { SDLK_KP_5 };
	    inline static constexpr uint32_t Kp6 { SDLK_KP_6 };
	    inline static constexpr uint32_t Kp7 { SDLK_KP_7 };
	    inline static constexpr uint32_t Kp8 { SDLK_KP_8 };
	    inline static constexpr uint32_t Kp9 { SDLK_KP_9 };
	    inline static constexpr uint32_t Kp0 { SDLK_KP_0 };
	    inline static constexpr uint32_t KpPeriod { SDLK_KP_PERIOD };
	    inline static constexpr uint32_t Application { SDLK_APPLICATION };
	    inline static constexpr uint32_t Power { SDLK_POWER };
	    inline static constexpr uint32_t KpEquals { SDLK_KP_EQUALS };
	    inline static constexpr uint32_t F13 { SDLK_F13 };
	    inline static constexpr uint32_t F14 { SDLK_F14 };
	    inline static constexpr uint32_t F15 { SDLK_F15 };
	    inline static constexpr uint32_t F16 { SDLK_F16 };
	    inline static constexpr uint32_t F17 { SDLK_F17 };
	    inline static constexpr uint32_t F18 { SDLK_F18 };
	    inline static constexpr uint32_t F19 { SDLK_F19 };
	    inline static constexpr uint32_t F20 { SDLK_F20 };
	    inline static constexpr uint32_t F21 { SDLK_F21 };
	    inline static constexpr uint32_t F22 { SDLK_F22 };
	    inline static constexpr uint32_t F23 { SDLK_F23 };
	    inline static constexpr uint32_t F24 { SDLK_F24 };
	    inline static constexpr uint32_t Execute { SDLK_EXECUTE };
	    inline static constexpr uint32_t Help { SDLK_HELP };
	    inline static constexpr uint32_t Menu { SDLK_MENU };
	    inline static constexpr uint32_t Select { SDLK_SELECT };
	    inline static constexpr uint32_t Stop { SDLK_STOP };
	    inline static constexpr uint32_t Again { SDLK_AGAIN };
	    inline static constexpr uint32_t Undo { SDLK_UNDO };
	    inline static constexpr uint32_t Cut { SDLK_CUT };
	    inline static constexpr uint32_t Copy { SDLK_COPY };
	    inline static constexpr uint32_t Paste { SDLK_PASTE };
	    inline static constexpr uint32_t Find { SDLK_FIND };
	    inline static constexpr uint32_t Mute { SDLK_MUTE };
	    inline static constexpr uint32_t Volumeup { SDLK_VOLUMEUP };
	    inline static constexpr uint32_t Volumedown { SDLK_VOLUMEDOWN };
	    inline static constexpr uint32_t KpComma { SDLK_KP_COMMA };
	    inline static constexpr uint32_t KpEqualsas400 { SDLK_KP_EQUALSAS400 };
	    inline static constexpr uint32_t Alterase { SDLK_ALTERASE };
	    inline static constexpr uint32_t Sysreq { SDLK_SYSREQ };
	    inline static constexpr uint32_t Cancel { SDLK_CANCEL };
	    inline static constexpr uint32_t Clear { SDLK_CLEAR };
	    inline static constexpr uint32_t Prior { SDLK_PRIOR };
	    inline static constexpr uint32_t Return2 { SDLK_RETURN2 };
	    inline static constexpr uint32_t Separator { SDLK_SEPARATOR };
	    inline static constexpr uint32_t Out { SDLK_OUT };
	    inline static constexpr uint32_t Oper { SDLK_OPER };
	    inline static constexpr uint32_t Clearagain { SDLK_CLEARAGAIN };
	    inline static constexpr uint32_t Crsel { SDLK_CRSEL };
	    inline static constexpr uint32_t Exsel { SDLK_EXSEL };
	    inline static constexpr uint32_t Kp00 { SDLK_KP_00 };
	    inline static constexpr uint32_t Kp000 { SDLK_KP_000 };
	    inline static constexpr uint32_t Thousandsseparator { SDLK_THOUSANDSSEPARATOR };
	    inline static constexpr uint32_t Decimalseparator { SDLK_DECIMALSEPARATOR };
	    inline static constexpr uint32_t Currencyunit { SDLK_CURRENCYUNIT };
	    inline static constexpr uint32_t Currencysubunit { SDLK_CURRENCYSUBUNIT };
	    inline static constexpr uint32_t KpLeftparen { SDLK_KP_LEFTPAREN };
	    inline static constexpr uint32_t KpRightparen { SDLK_KP_RIGHTPAREN };
	    inline static constexpr uint32_t KpLeftbrace { SDLK_KP_LEFTBRACE };
	    inline static constexpr uint32_t KpRightbrace { SDLK_KP_RIGHTBRACE };
	    inline static constexpr uint32_t KpTab { SDLK_KP_TAB };
	    inline static constexpr uint32_t KpBackspace { SDLK_KP_BACKSPACE };
	    inline static constexpr uint32_t KpA { SDLK_KP_A };
	    inline static constexpr uint32_t KpB { SDLK_KP_B };
	    inline static constexpr uint32_t KpC { SDLK_KP_C };
	    inline static constexpr uint32_t KpD { SDLK_KP_D };
	    inline static constexpr uint32_t KpE { SDLK_KP_E };
	    inline static constexpr uint32_t KpF { SDLK_KP_F };
	    inline static constexpr uint32_t KpXor { SDLK_KP_XOR };
	    inline static constexpr uint32_t KpPower { SDLK_KP_POWER };
	    inline static constexpr uint32_t KpPercent { SDLK_KP_PERCENT };
	    inline static constexpr uint32_t KpLess { SDLK_KP_LESS };
	    inline static constexpr uint32_t KpGreater { SDLK_KP_GREATER };
	    inline static constexpr uint32_t KpAmpersand { SDLK_KP_AMPERSAND };
	    inline static constexpr uint32_t KpDblampersand { SDLK_KP_DBLAMPERSAND };
	    inline static constexpr uint32_t KpVerticalbar { SDLK_KP_VERTICALBAR };
	    inline static constexpr uint32_t KpDblverticalbar { SDLK_KP_DBLVERTICALBAR };
	    inline static constexpr uint32_t KpColon { SDLK_KP_COLON };
	    inline static constexpr uint32_t KpHash { SDLK_KP_HASH };
	    inline static constexpr uint32_t KpSpace { SDLK_KP_SPACE };
	    inline static constexpr uint32_t KpAt { SDLK_KP_AT };
	    inline static constexpr uint32_t KpExclam { SDLK_KP_EXCLAM };
	    inline static constexpr uint32_t KpMemstore { SDLK_KP_MEMSTORE };
	    inline static constexpr uint32_t KpMemrecall { SDLK_KP_MEMRECALL };
	    inline static constexpr uint32_t KpMemclear { SDLK_KP_MEMCLEAR };
	    inline static constexpr uint32_t KpMemadd { SDLK_KP_MEMADD };
	    inline static constexpr uint32_t KpMemsubtract { SDLK_KP_MEMSUBTRACT };
	    inline static constexpr uint32_t KpMemmultiply { SDLK_KP_MEMMULTIPLY };
	    inline static constexpr uint32_t KpMemdivide { SDLK_KP_MEMDIVIDE };
	    inline static constexpr uint32_t KpPlusminus { SDLK_KP_PLUSMINUS };
	    inline static constexpr uint32_t KpClear { SDLK_KP_CLEAR };
	    inline static constexpr uint32_t KpClearentry { SDLK_KP_CLEARENTRY };
	    inline static constexpr uint32_t KpBinary { SDLK_KP_BINARY };
	    inline static constexpr uint32_t KpOctal { SDLK_KP_OCTAL };
	    inline static constexpr uint32_t KpDecimal { SDLK_KP_DECIMAL };
	    inline static constexpr uint32_t KpHexadecimal { SDLK_KP_HEXADECIMAL };
	    inline static constexpr uint32_t Lctrl { SDLK_LCTRL };
	    inline static constexpr uint32_t Lshift { SDLK_LSHIFT };
	    inline static constexpr uint32_t Lalt { SDLK_LALT };
	    inline static constexpr uint32_t Lgui { SDLK_LGUI };
	    inline static constexpr uint32_t Rctrl { SDLK_RCTRL };
	    inline static constexpr uint32_t Rshift { SDLK_RSHIFT };
	    inline static constexpr uint32_t Ralt { SDLK_RALT };
	    inline static constexpr uint32_t Rgui { SDLK_RGUI };
	    inline static constexpr uint32_t Mode { SDLK_MODE };
	    inline static constexpr uint32_t Sleep { SDLK_SLEEP };
	    inline static constexpr uint32_t Wake { SDLK_WAKE };
	    inline static constexpr uint32_t ChannelIncrement { SDLK_CHANNEL_INCREMENT };
	    inline static constexpr uint32_t ChannelDecrement { SDLK_CHANNEL_DECREMENT };
	    inline static constexpr uint32_t MediaPlay { SDLK_MEDIA_PLAY };
	    inline static constexpr uint32_t MediaPause { SDLK_MEDIA_PAUSE };
	    inline static constexpr uint32_t MediaRecord { SDLK_MEDIA_RECORD };
	    inline static constexpr uint32_t MediaFastForward { SDLK_MEDIA_FAST_FORWARD };
	    inline static constexpr uint32_t MediaRewind { SDLK_MEDIA_REWIND };
	    inline static constexpr uint32_t MediaNextTrack { SDLK_MEDIA_NEXT_TRACK };
	    inline static constexpr uint32_t MediaPreviousTrack { SDLK_MEDIA_PREVIOUS_TRACK };
	    inline static constexpr uint32_t MediaStop { SDLK_MEDIA_STOP };
	    inline static constexpr uint32_t MediaEject { SDLK_MEDIA_EJECT };
	    inline static constexpr uint32_t MediaPlayPause { SDLK_MEDIA_PLAY_PAUSE };
	    inline static constexpr uint32_t MediaSelect { SDLK_MEDIA_SELECT };
	    inline static constexpr uint32_t AcNew { SDLK_AC_NEW };
	    inline static constexpr uint32_t AcOpen { SDLK_AC_OPEN };
	    inline static constexpr uint32_t AcClose { SDLK_AC_CLOSE };
	    inline static constexpr uint32_t AcExit { SDLK_AC_EXIT };
	    inline static constexpr uint32_t AcSave { SDLK_AC_SAVE };
	    inline static constexpr uint32_t AcPrint { SDLK_AC_PRINT };
	    inline static constexpr uint32_t AcProperties { SDLK_AC_PROPERTIES };
	    inline static constexpr uint32_t AcSearch { SDLK_AC_SEARCH };
	    inline static constexpr uint32_t AcHome { SDLK_AC_HOME };
	    inline static constexpr uint32_t AcBack { SDLK_AC_BACK };
	    inline static constexpr uint32_t AcForward { SDLK_AC_FORWARD };
	    inline static constexpr uint32_t AcStop { SDLK_AC_STOP };
	    inline static constexpr uint32_t AcRefresh { SDLK_AC_REFRESH };
	    inline static constexpr uint32_t AcBookmarks { SDLK_AC_BOOKMARKS };
	    inline static constexpr uint32_t Softleft { SDLK_SOFTLEFT };
	    inline static constexpr uint32_t Softright { SDLK_SOFTRIGHT };
	    inline static constexpr uint32_t Call { SDLK_CALL };
	    inline static constexpr uint32_t Endcall { SDLK_ENDCALL };
	    inline static constexpr uint32_t LeftTab { SDLK_LEFT_TAB };
	    inline static constexpr uint32_t Level5Shift { SDLK_LEVEL5_SHIFT };
	    inline static constexpr uint32_t MultiKeyCompose { SDLK_MULTI_KEY_COMPOSE };
	    inline static constexpr uint32_t Lmeta { SDLK_LMETA };
	    inline static constexpr uint32_t Rmeta { SDLK_RMETA };
	    inline static constexpr uint32_t Lhyper { SDLK_LHYPER };
	    inline static constexpr uint32_t Rhyper { SDLK_RHYPER };
	};
}
