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

#pragma once

// osnd - a small, deliberately minimal audio module (sibling to ostd/ogfx). Today it's just
// what PianoRoll needs: decode a whole audio file to PCM (WAV via SDL3, MP3 via vendored
// dr_mp3) and play it back with sample-accurate seeking. Meant to grow in place as new needs
// come up (e.g. another decoder, streaming for very large files) rather than being redesigned.

#include <osnd/audio/AudioClip.hpp>
#include <osnd/audio/AudioPlayer.hpp>
