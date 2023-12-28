#ifndef __DEFINES_HPP__
#define __DEFINES_HPP__

#if defined(_WIN32) || defined(_WIN64)
	#define WINDOWS_OS
	#ifdef _WIN32_WINNT
		#undef _WIN32_WINNT
	#endif
	#define _WIN32_WINNT 0x0501
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

#define PI 3.1415926535898f
#define TWO_PI PI * 2.0f
#define HALF_PI PI / 2.0f
#define QUARTER_PI PI / 4.0f
#define DEG_TO_RAD(deg) (float)(deg * (PI / 180.0f))
#define RAD_TO_DEG(rad) (float)(rad * (180.0f / PI))

//Game Time constants
#define TM_R_SECONDS_FOR_G_MINUTE 1
#define TM_G_MINUTES_FOR_G_HOUR 60
#define TM_G_HOURS_FOR_G_DAY 24
#define TM_G_DAYS_FOR_G_LONG_MONTH 31
#define TM_G_DAYS_FOR_G_MEDIUM_MONTH 30
#define TM_G_DAYS_FOR_G_SHORT_MONTH 28

//Macro functions
#define ERROR_DATA() String(CPP_STR(__LINE__)), String(__FILE__)
#define STR_BOOL(b) (b ? "true" : "false")
#define INT_BOOL(i) (i == 0 ? false : true)
#define ZERO(n) (n > 0 ? n : 0)
#define FRAND() ((float)(rand() % 10000)) / 10000.0f
#define RANDOM(min, max) rand() % (max - min + 1) + min
#define LERP(n1, n2, f) (n2 - n1) * f + n1
#define CAP(n, max) (n > max ? max : n)
#define CAPD(n, min) (n < min ? min : n)
#define CAPB(n, min, max) (n < min ? min : (n > max ? max : n))
#define MAX(n1, n2) std::max(n1, n2)
#define MIN(n1, n2) std::min(n1, n2)
#define PROPORTION(w, x, y) ((x * w) / y)
#define CONVERT_1D_2D(i, width) IPoint(i % width, i / width)
#define CONVERT_2D_1D(x, y, width) (x + width * y)
#define PRINT(data) std::cout << data
#define PRINTLN(data) std::cout << data << "\n"
#define NEWLN() std::cout << "\n"
#define STDVEC_CONTAINS(vec, elem) (std::find(vec.begin(), vec.end(), elem) != vec.end())

//Memory management macros
#define new_sh(type) std::make_shared<type>
#define sh_ptr(type) std::shared_ptr<type>
#define new_un(type) std::make_unique<type>
#define un_ptr(type) std::unique_ptr<type>

#define OX_NO_ERROR 					0x00000000
#define OX_WINDOW_ERR_MASK 				0x00000000
#define OX_SHADER_ERR_MASK 				0x00001000
#define OX_GLBUFFERS_ERR_MASK 			0x00002000
#define OX_TEXTURE_ERR_MASK 			0x00003000
#define OX_GFX_APPLICATION_2D_ERR_MASK 	0x00004000
#define OX_BITMAPFONT_ERR_MASK 			0x00005000
#define OX_RENDERER2D_ERR_MASK 			0x00006000
#define OX_RENDERTARGET_ERR_MASK 		0x00007000
#define OX_RENDERER2D_TEXT_ERR_MASK 	0x00008000

#endif
