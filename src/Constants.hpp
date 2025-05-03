#pragma once
#include <SDL.h>

namespace Constants
{
    constexpr int PANEL_WIDTH = 300;
    constexpr int BUTTON_HEIGHT = 30;
    constexpr int ITEM_HEIGHT = 35;
    constexpr int LINE_HEIGHT = 2;
    constexpr int TOTAL_HEIGHT = ITEM_HEIGHT + LINE_HEIGHT;
    constexpr int MARGIN = 10;
    const int WINDOW_WIDTH = 1500;
    const int WINDOW_HEIGHT = 800;

    const SDL_Color BACKGROUND_COLOR = {40, 40, 40, 255};
    const SDL_Color GRID_COLOR = {80, 80, 80, 255};
    const SDL_Color AXIS_COLOR = {200, 200, 200, 255};
    const SDL_Color TEXT_COLOR = {255, 255, 255, 255};
    const SDL_Color PANEL_COLOR = {60, 60, 60, 255};
    const SDL_Color BUTTON_COLOR = {80, 80, 160, 255};
    const SDL_Color SELECT_COLOR = {100, 100, 200, 255};
    const SDL_Color EDIT_COLOR = {150, 150, 250, 255};
}