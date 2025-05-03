#include "RenderUtils.hpp"

void drawCoordinateGrid(SDL_Renderer* renderer, TTF_Font* font, const MathRange& range)
{
    using namespace Constants;
    const double baseGridSize = std::pow(10.0, std::floor(std::log10(range.xSpan())));
    const double gridSize = baseGridSize / 2.0;
    const int precision = std::max(0, 3 - static_cast<int>(std::log10(gridSize)));

    SDL_SetRenderDrawColor(renderer, GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b, GRID_COLOR.a);
    for (double x = std::ceil(range.xMin / gridSize) * gridSize; x <= range.xMax; x += gridSize)
    {
        if (std::abs(x) < 1e-10)
            continue;
        Point2D p1 = mathToScreen({x, range.yMin}, range);
        Point2D p2 = mathToScreen({x, range.yMax}, range);
        SDL_RenderDrawLine(renderer, round(p1.x), round(p1.y), round(p2.x), round(p2.y));
    }

    for (double y = std::ceil(range.yMin / gridSize) * gridSize; y <= range.yMax; y += gridSize)
    {
        if (std::abs(y) < 1e-10)
            continue;
        Point2D p1 = mathToScreen({range.xMin, y}, range);
        Point2D p2 = mathToScreen({range.xMax, y}, range);
        SDL_RenderDrawLine(renderer, round(p1.x), round(p1.y), round(p2.x), round(p2.y));
    }

    SDL_SetRenderDrawColor(renderer, AXIS_COLOR.r, AXIS_COLOR.g, AXIS_COLOR.b, AXIS_COLOR.a);
    Point2D xStart = mathToScreen({range.xMin, 0}, range);
    Point2D xEnd = mathToScreen({range.xMax, 0}, range);
    Point2D yStart = mathToScreen({0, range.yMin}, range);
    Point2D yEnd = mathToScreen({0, range.yMax}, range);
    SDL_RenderDrawLine(renderer, xStart.x, xStart.y, xEnd.x, xEnd.y);
    SDL_RenderDrawLine(renderer, yStart.x, yStart.y, yEnd.x, yEnd.y);

    SDL_Color textColor = TEXT_COLOR;
    for (double x = std::ceil(range.xMin / gridSize) * gridSize; x <= range.xMax; x += gridSize)
    {
        if (std::abs(x) < 1e-10)
            continue;
        Point2D p = mathToScreen({x, 0}, range);
        std::string label = formatNumber(x, precision);
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, label.c_str(), textColor);
        if (!surface)
            continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {static_cast<int>(p.x) - surface->w/2, static_cast<int>(p.y) + 5, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    for (double y = std::ceil(range.yMin / gridSize) * gridSize; y <= range.yMax; y += gridSize)
    {
        if (std::abs(y) < 1e-10)
            continue;
        Point2D p = mathToScreen({0, y}, range);
        std::string label = formatNumber(y, precision);
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, label.c_str(), textColor);
        if (!surface)
            continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {static_cast<int>(p.x) - surface->w -5, static_cast<int>(p.y) - surface->h/2, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}