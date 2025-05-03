#pragma once
#include "MathUtils.hpp"
#include <SDL_ttf.h>

void drawCoordinateGrid(SDL_Renderer* renderer, TTF_Font* font, const MathRange& range);