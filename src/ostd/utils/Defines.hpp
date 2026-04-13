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

#if defined(_WIN32) || defined(_WIN64)
	#define WINDOWS_OS
	#ifdef _WIN32_WINNT
		#undef _WIN32_WINNT
	#endif
	#define _WIN32_WINNT 0x0501
#elif defined(__APPLE__)
	#define MAC_OS
#else
	#define LINUX_OS
#endif

//Basic constants
#if !defined(BUILD_NR)
	#define BUILD_NR 0
#endif
#if !defined(MAJ_V)
	#define MAJ_V 0
#endif
#if !defined(MIN_V)
	#define MIN_V 0
#endif
#if !defined(VERSION_STR)
	#define VERSION_STR "0.0.0"
#endif
#if !defined(NULL)
	#define NULL 0
#endif

#ifndef OX_DEBUG_BUILD
	#define _DEBUG(n)
	#ifndef OX_RELEASE_BUILD
		#define OX_RELEASE_BUILD
	#endif
#else
	#define _DEBUG(n) std::cout << (int)n << "\n";
#endif
