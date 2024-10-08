#pragma once

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#undef __linux__
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
