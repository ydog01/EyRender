#pragma once
#include "ItemList.hpp"
#include "MathUtils.hpp"
#include "RenderUtils.hpp"

class MathVisualizer
{
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    MathRange currentRange{-15.0, 15.0, -10.0, 10.0};
    ItemList itemList;
    bool isRunning = true;
    bool isDragging = false;
    Point2D dragStart{0.0, 0.0};
    MathRange dragStartRange{-15.0, 15.0, -10.0, 10.0};
    int panelX = 1200;
    Uint32 cursorBlink = 0;
    int visibleItems = 0;

    size_t ffts = 2u;
    size_t lstep = 5u;
    size_t step;
    decltype(Equation::evaluator.vars->search("x")) xNode;
    decltype(Equation::evaluator.vars->search("y")) yNode;

    std::vector<std::vector<double>> cubes;

    void renderText(const std::string& text, int x, int y, int maxWidth);
    void renderPanel();
    void renderEquations();
    void handlePanelClick(const SDL_MouseButtonEvent& e);

public:
    MathVisualizer():
        xNode(nullptr),
        yNode(nullptr),
        cubes(Constants::WINDOW_HEIGHT/lstep+1,std::vector<double>(panelX/lstep+1)),
        step(lstep*ffts)
    {}
    bool init();
    void handleEvents();
    void render();
    void run();
    void cleanup();
};