#include "MathVisualizer.hpp"

bool MathVisualizer::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;
    window = SDL_CreateWindow("Math Visualizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
        return false;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        return false;
    if (TTF_Init() == -1)
        return false;
    font = TTF_OpenFont("C:/Windows/Fonts/msyh.ttc", 18);
    if (!font)
        return false;
    itemList.updateButtonPositions(panelX);

    Equation::evaluator.vars->insert("x",{});
    Equation::evaluator.vars->insert("y",{});

    xNode = Equation::evaluator.vars->search("x");
    yNode = Equation::evaluator.vars->search("y");

    return true;
}

void MathVisualizer::handleEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        itemList.handleInput(e);
        switch (e.type)
        {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.x > panelX)
                    handlePanelClick(e.button);
                else if (e.button.button == SDL_BUTTON_LEFT)
                {
                    isDragging = true;
                    dragStart = {static_cast<double>(e.button.x), static_cast<double>(e.button.y)};
                    dragStartRange = currentRange;
                    itemList.endEdit();
                }
                break;
            case SDL_MOUSEBUTTONUP: 
                if (e.button.button == SDL_BUTTON_LEFT)
                    isDragging = false;
                break;
            case SDL_MOUSEMOTION:
                if (isDragging)
                {
                    Point2D current = {static_cast<double>(e.motion.x), static_cast<double>(e.motion.y)};
                    Point2D delta = screenToMath(current.x, current.y, currentRange) - 
                                    screenToMath(dragStart.x, dragStart.y, currentRange);
                    currentRange.xMin = dragStartRange.xMin - delta.x;
                    currentRange.xMax = dragStartRange.xMax - delta.x;
                    currentRange.yMin = dragStartRange.yMin - delta.y;
                    currentRange.yMax = dragStartRange.yMax - delta.y;
                }
                break;
            case SDL_MOUSEWHEEL:
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                if (mouseX > panelX)
                    itemList.handleScroll(e.wheel.y);
                else
                {
                    const double zoomCenterX = currentRange.xMin + currentRange.xSpan()/2;
                    const double zoomCenterY = currentRange.yMin + currentRange.ySpan()/2;
                    const double zoomFactor = (e.wheel.y > 0) ? 0.9 : 1.1;
                    currentRange.xMin = zoomCenterX + (currentRange.xMin - zoomCenterX) * zoomFactor;
                    currentRange.xMax = zoomCenterX + (currentRange.xMax - zoomCenterX) * zoomFactor;
                    currentRange.yMin = zoomCenterY + (currentRange.yMin - zoomCenterY) * zoomFactor;
                    currentRange.yMax = zoomCenterY + (currentRange.yMax - zoomCenterY) * zoomFactor;
                }
                break;
        }
    }
}

void MathVisualizer::renderText(const std::string& text, int x, int y, int maxWidth)
{
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), Constants::TEXT_COLOR);
    if (!surface)
        return;
    if (surface->w > maxWidth)
    {
        float scale = static_cast<float>(maxWidth) / surface->w;
        SDL_Rect dstRect = {x, y, static_cast<int>(surface->w * scale), static_cast<int>(surface->h * scale)};
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }
    else
    {
        SDL_Rect dstRect = {x, y, surface->w, surface->h};
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void MathVisualizer::renderPanel()
{
    using namespace Constants;
    SDL_Rect panelRect = {panelX, 0, PANEL_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawColor(renderer, PANEL_COLOR.r, PANEL_COLOR.g, PANEL_COLOR.b, 255);
    SDL_RenderFillRect(renderer, &panelRect);

    const SDL_Rect& addBtn = itemList.getAddButton();
    const SDL_Rect& delBtn = itemList.getDelButton();
    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, 255);
    SDL_RenderFillRect(renderer, &addBtn);
    SDL_RenderFillRect(renderer, &delBtn);
    renderText("+ Add", addBtn.x + 10, addBtn.y + 3, addBtn.w - 20);
    renderText("- Delete", delBtn.x + 10, delBtn.y + 3, delBtn.w - 20);

    visibleItems = (WINDOW_HEIGHT - (BUTTON_HEIGHT * 2 + MARGIN * 3)) / TOTAL_HEIGHT;
    const int startIdx = itemList.getScrollOffset();
    const int endIdx = std::min(startIdx + visibleItems, static_cast<int>(itemList.getEquations().size()));

    int yPos = MARGIN;
    for (int i = startIdx; i < endIdx; ++i) 
    {
        SDL_Rect itemRect = 
        {
            panelX + MARGIN, 
            yPos, 
            PANEL_WIDTH - MARGIN * 2, 
            ITEM_HEIGHT
        };

        if (i == itemList.getSelected()) 
        {
            SDL_SetRenderDrawColor(renderer, SELECT_COLOR.r, SELECT_COLOR.g, SELECT_COLOR.b, 255);
            SDL_RenderFillRect(renderer, &itemRect);
        }

        const std::string& text = itemList.getEquations()[i].expression;

        renderText(text, itemRect.x + 5, itemRect.y + 3, itemRect.w - 10);

        if (itemList.isEditing() && i == itemList.getSelected()) 
        {
            const std::string& editingText = itemList.getEquations()[i].expression;
            const int cursorPos = itemList.getCursorPos();

            std::string beforeCursor = editingText.substr(0, cursorPos);
            int textWidth = 0;
            TTF_SizeText(font, beforeCursor.c_str(), &textWidth, nullptr);

            if (SDL_GetTicks() - cursorBlink < 500) 
            {
                SDL_SetRenderDrawColor(renderer, EDIT_COLOR.r, EDIT_COLOR.g, EDIT_COLOR.b, 255);
                SDL_RenderDrawLine(renderer, 
                    itemRect.x + 5 + textWidth,  
                    itemRect.y + 2,             
                    itemRect.x + 5 + textWidth,  
                    itemRect.y + ITEM_HEIGHT - 4 
                );
            }
        }

        SDL_SetRenderDrawColor(renderer, GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b, GRID_COLOR.a);
        SDL_Rect lineRect
        {
            panelX + MARGIN, 
            yPos + ITEM_HEIGHT, 
            PANEL_WIDTH - MARGIN * 2, 
            LINE_HEIGHT
        };
        SDL_RenderFillRect(renderer, &lineRect);

        yPos += TOTAL_HEIGHT;
    }

    if (SDL_GetTicks() - cursorBlink > 1000) 
        cursorBlink = SDL_GetTicks();
}
void MathVisualizer::renderEquations()
{
    struct tools
    {
        static size_t lerp(double a, double b, size_t size)
        {
            return static_cast<size_t>(std::round(a * size / (a - b)));
        }
        static void marching_squares(SDL_Renderer*renderer,size_t sx,size_t sy,size_t step,double v11,double v12,double v21,double v22)
        {
            const char state =
                ((v11 >= 0) ? 1 : 0) |
                ((v12 >= 0) ? 2 : 0) |
                ((v21 >= 0) ? 4 : 0) |
                ((v22 >= 0) ? 8 : 0);

            if (state == 0 || state == 0b1111)
                return;

            switch (state)
            {
            case 0b0001:
            case 0b1110:
                SDL_RenderDrawLine(renderer, sx + lerp(v11, v12, step), sy, sx, sy + lerp(v11, v21, step));
                break;
            case 0b0010:
            case 0b1101:
                SDL_RenderDrawLine(renderer, sx + lerp(v11, v12, step), sy, sx + step, sy + lerp(v12, v22, step));
                break;
            case 0b0100:
            case 0b1011:
                SDL_RenderDrawLine(renderer, sx + lerp(v21, v22, step), sy + step, sx, sy + lerp(v11, v21, step));
                break;
            case 0b1000:
            case 0b0111:
                SDL_RenderDrawLine(renderer, sx + lerp(v21, v22, step), sy + step, sx + step, sy + lerp(v12, v22, step));
                break;
            case 0b0011:
            case 0b1100:
                SDL_RenderDrawLine(renderer, sx, sy + lerp(v11, v21, step), sx + step, sy + lerp(v12, v22, step));
                break;
            case 0b1010:
            case 0b0101:
                SDL_RenderDrawLine(renderer, sx + lerp(v11, v12, step), sy, sx + lerp(v21, v22, step), sy + step);
                break;
            case 0b0110:
                SDL_RenderDrawLine(renderer, sx + lerp(v11, v12, step), sy, sx + step, sy + lerp(v12, v22, step));
                SDL_RenderDrawLine(renderer, sx + lerp(v21, v22, step), sy + step, sx, sy + lerp(v11, v21, step));
                break;
            case 0b1001:
                SDL_RenderDrawLine(renderer, sx + lerp(v11, v12, step), sy, sx, sy + lerp(v11, v21, step));
                SDL_RenderDrawLine(renderer, sx + lerp(v21, v22, step), sy + step, sx + step, sy + lerp(v12, v22, step));
                break;
            }
        }
    };

    const size_t rows = cubes.size();
    const size_t cols = cubes.front().size();
    Point2D temp_point;

    for (Equation &eq : itemList.getEquations())
    {
        if(eq.type==RelationalOperator::INVALID)
            continue;

        try
        {
            for (size_t ypos = 0, y = 0; ypos < rows; ypos++, y += lstep)
            {
                for (size_t xpos = 0, x = 0; xpos < cols; xpos++, x += lstep)
                {
                    if (xpos % ffts || ypos % ffts)
                    {
                        cubes[ypos][xpos] = std::numeric_limits<double>::max();
                        continue;
                    }
                    temp_point = screenToMath(x, y, currentRange);
                    xNode->data->value = temp_point.x;
                    yNode->data->value = temp_point.y;
                    cubes[ypos][xpos] = Equation::evaluator.evaluate(eq.value);
                }
            }
            SDL_SetRenderDrawColor(renderer,eq.color.r,eq.color.g,eq.color.b,eq.color.a);
            {
                for (size_t ypos = 0, y = 0; ypos < rows-ffts; ypos+=ffts, y += step)
                {
                    for (size_t xpos = 0, x = 0; xpos < cols-ffts; xpos+=ffts, x += step)
                    {
                        const char state =
                            ((cubes[ypos][xpos] >= 0) ? 1 : 0) |
                            ((cubes[ypos][xpos + ffts] >= 0) ? 2 : 0) |
                            ((cubes[ypos + ffts][xpos] >= 0) ? 4 : 0) |
                            ((cubes[ypos + ffts][xpos + ffts] >= 0) ? 8 : 0);
    
                        if (state == 0 || state == 0b1111)
                            continue;
    
                        for (size_t lypos = ypos, count_y = 0, ly = y; count_y <= ffts; count_y++, lypos++, ly += lstep)
                            for (size_t lxpos = xpos, count_x = 0, lx = x; count_x <= ffts; count_x++, lxpos++, lx += lstep)
                            {
                                if(cubes[lypos][lxpos]==std::numeric_limits<double>::max())
                                {
                                    temp_point = screenToMath(lx,ly,currentRange);
                                    xNode->data->value = temp_point.x;
                                    yNode->data->value = temp_point.y;
                                    cubes[lypos][lxpos] = Equation::evaluator.evaluate(eq.value);
                                }
                                if(count_x&&count_y)
                                    tools::marching_squares(renderer,lx-lstep,ly-lstep,lstep,cubes[lypos-1][lxpos-1],cubes[lypos-1][lxpos],cubes[lypos][lxpos-1],cubes[lypos][lxpos]);
                            }
                    }
                }
            }   
        }
        catch(...)
        {
            eq.type = RelationalOperator::INVALID;
            continue;
        }
    }
}

void MathVisualizer::render()
{
    using namespace Constants;
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 255);
    SDL_RenderClear(renderer);

    SDL_Rect graphArea{0, 0, panelX, WINDOW_HEIGHT};
    SDL_RenderSetClipRect(renderer, &graphArea);
    drawCoordinateGrid(renderer, font, currentRange);
    renderEquations();
    SDL_RenderSetClipRect(renderer, nullptr);

    renderPanel();
    SDL_RenderPresent(renderer);
}

void MathVisualizer::run()
{
    const Uint32 targetDelay = 1000 / 60;
    Uint32 frameStart, frameTime;
    
    while (isRunning)
    {
        frameStart = SDL_GetTicks();
        handleEvents();
        render();
        
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < targetDelay)
            SDL_Delay(targetDelay - frameTime);
    }
}

void MathVisualizer::cleanup()
{
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void MathVisualizer::handlePanelClick(const SDL_MouseButtonEvent& e)
{
    SDL_Point mouse{e.x, e.y};
    const SDL_Rect& addBtn = itemList.getAddButton();
    const SDL_Rect& delBtn = itemList.getDelButton();

    if (SDL_PointInRect(&mouse, &addBtn))
    {
        itemList.add(std::string());
        itemList.select(itemList.getEquations().size() - 1);
    }
    else if (SDL_PointInRect(&mouse, &delBtn))
    {
        itemList.removeSelected();
    }
    else
    {
        const int listStartY = Constants::MARGIN;
        const int listEndY = Constants::MARGIN + visibleItems * Constants::TOTAL_HEIGHT;
        if (mouse.y >= listStartY && mouse.y < listEndY)
        {
            int itemIndex = itemList.getScrollOffset() + (mouse.y - listStartY) / Constants::TOTAL_HEIGHT;
            itemList.select(itemIndex < itemList.getEquations().size() ? itemIndex : -1);
        }
        else
        {
            itemList.select(-1);
        }
    }
}