#include "ItemList.hpp"

void ItemList::updateButtonPositions(int panelX)
{
    using namespace Constants;
    addButton = {
        panelX + MARGIN,
        WINDOW_HEIGHT - BUTTON_HEIGHT * 2 - MARGIN * 2,
        PANEL_WIDTH - MARGIN * 2,
        BUTTON_HEIGHT
    };
    delButton = {
        panelX + MARGIN,
        WINDOW_HEIGHT - BUTTON_HEIGHT - MARGIN,
        PANEL_WIDTH - MARGIN * 2,
        BUTTON_HEIGHT
    };
}

int ItemList::add(const std::string& item)
{
    Equation eq;
    eq.expression = item;
    if (selected == -1)
    {
        equations.push_back(std::move(eq));
        return equations.size()-1;
    }
    equations.insert(equations.begin() + selected + 1u, std::move(eq));
    return selected+1;
}

void ItemList::removeSelected()
{
    if (selected >= 0 && selected < static_cast<int>(equations.size()))
    {
        equations.erase(equations.begin() + selected);
        selected--;
        cursorPos = 0;
    }
}

void ItemList::endEdit()
{
    if (selected == -1)
        return;

    Equation& eq = equations[selected];
    eq.value.clear();
    eq.type = RelationalOperator::INVALID;

    selected = -1;
    cursorPos = 0;

    if (eq.expression.empty())
        return;

    size_t pos = 0;
    while (eq.type == RelationalOperator::INVALID)
    {
        if (pos == eq.expression.size()-1)
            break;

        switch (eq.expression[pos])
        {
            case '=': 
                eq.type = RelationalOperator::EQUAL;
                break;
            case '<':
                if (eq.expression[pos+1] == '=')
                    eq.type = RelationalOperator::LESS_THAN_OR_EQUAL;
                else
                    eq.type = RelationalOperator::LESS_THAN;
                break;
            case '>':
                if (eq.expression[pos+1] == '=')
                    eq.type = RelationalOperator::GREATER_THAN_OR_EQUAL;
                else
                    eq.type = RelationalOperator::GREATER_THAN;
                break;
            case '!':
                if (eq.expression[pos+1] == '=')
                    eq.type = RelationalOperator::NOT_EQUAL;
                break;
            default:
                pos++;
                break;
        }
    }

    if (eq.type == RelationalOperator::INVALID)
        return;

    try 
    {
        Equation::evaluator.parse(eq.value, eq.expression.substr(0,pos));
        pos++;
        if (eq.type == RelationalOperator::NOT_EQUAL || eq.type == RelationalOperator::GREATER_THAN_OR_EQUAL || eq.type == RelationalOperator::LESS_THAN_OR_EQUAL)
            pos++;
            
        Equation::evaluator.parse(eq.value,eq.expression.substr(pos));
        
        eq.value.index.push_back('f');
        eq.value.funcs.push_back(Equation::evaluator.infix_ops->search("-")->data);
    }
    catch (...)
    {
        eq.value.clear();
        eq.type = RelationalOperator::INVALID;
    }
}

void ItemList::handleInput(const SDL_Event& e, SDL_Renderer* renderer)
{
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_INSERT)
    {
        select(add(std::string()));
        return;
    }

    if (selected == -1)
    {
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
            select(0);
        return;
    }

    Equation& eq = equations[selected];
    if (e.type == SDL_TEXTINPUT)
    {
        eq.expression.insert(cursorPos, e.text.text);
        cursorPos += strlen(e.text.text);
    }
    else if (e.type == SDL_KEYDOWN)
    {
        switch (e.key.keysym.sym)
        {
            case SDLK_RETURN:
                endEdit();
                break;
            case SDLK_LEFT:
                if (cursorPos > 0)
                {
                    size_t size = eq.expression[cursorPos - 1u] < 0 ? 3u : 1u;
                    cursorPos-=size;
                }
                break;
            case SDLK_RIGHT:
                if (cursorPos < static_cast<int>(eq.expression.size()))
                {
                    size_t size = eq.expression[cursorPos - 1u] < 0 ? 3u : 1u;
                    cursorPos+=size;
                }
                break;
            case SDLK_UP:
                if (selected > 0)
                    select(selected - 1);
                break;
            case SDLK_DOWN:
                if (selected < equations.size()-1)
                    select(selected + 1);
                break;
            case SDLK_BACKSPACE:
                if (cursorPos > 0)
                {
                    size_t size = eq.expression[cursorPos - 1u] < 0 ? 3u : 1u;
                    eq.expression.erase(cursorPos - size,size);
                    cursorPos-=size;
                }
                break;
            case SDLK_DELETE:
                removeSelected();
                break;
            case SDLK_HOME:
                cursorPos = 0;
                break;
            case SDLK_END:
                cursorPos = eq.expression.size();
                break;
            case SDLK_q:
                if(e.key.keysym.mod & KMOD_CTRL)
                    selectColor(renderer);
                break;
        }
    }
}
void ItemList::selectColor(SDL_Renderer*renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_Rect mask = { 0, 0, Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT };
    SDL_RenderFillRect(renderer, &mask);

    SDL_Rect colorRect{(Constants::WINDOW_WIDTH>>1)-153,(Constants::WINDOW_HEIGHT >> 1) - 123,256,256};
    SDL_Rect rRect{ colorRect.x+270,colorRect.y,50,256 };
    SDL_Rect MainRect{colorRect.x-10,colorRect.y-40,rRect.x-colorRect.x+rRect.w+20,306 };
    SDL_Rect LastRect{ MainRect.x + 10,MainRect.y + 10,((MainRect.w - 20)>>1)-5,20 };
    SDL_Rect currentRect{ LastRect.x+LastRect.w+10,MainRect.y + 10,((MainRect.w - 20) >> 1) - 5,20 };

    const Uint32 targetDelay = 1000 / 60;
    Uint32 frameStart, frameTime;

    Uint8 r=0, g=0, b=0 ,lg=0,lb=0;

    SDL_Event e;
    bool isRunning = true;

    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 105);
    SDL_RenderFillRect(renderer, &MainRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_Point mouse;

    while (isRunning)
    {
        frameStart = SDL_GetTicks();

        SDL_GetMouseState(&mouse.x, &mouse.y);
        if (SDL_PointInRect(&mouse, &colorRect))
        {
            lg = mouse.y - colorRect.y;
            lb = mouse.x - colorRect.x;
        }
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE|| e.key.keysym.sym == SDLK_RETURN)
                    isRunning = false;
                if (e.key.keysym.sym == SDLK_RETURN)
                    equations[selected].color = { r,g,b };
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (SDL_PointInRect(&mouse, &colorRect))
                    g = lg, b = lb;
                else if (SDL_PointInRect(&mouse, &rRect))
                    r = mouse.y - rRect.y;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, r, lg, lb, 255);
        SDL_RenderFillRect(renderer, &currentRect);

        SDL_SetRenderDrawColor(renderer, r, g, b,255);
        SDL_RenderFillRect(renderer, &LastRect);

        Uint16 cx, cy=0;
        for (int y = colorRect.y; cy <= 255; cy++, y++)
        {
            cx = 0;
            for (int x = colorRect.x; cx <= 255; cx++, x++)
            {
                SDL_SetRenderDrawColor(renderer, r, static_cast<Uint8>(cy), static_cast<Uint8>(cx),255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
        cy = 0;
        for (int y = rRect.y; cy <= 255; cy++, y++)
        {
            SDL_SetRenderDrawColor(renderer, static_cast<Uint8>(cy), g, b,255);
            SDL_RenderDrawLine(renderer, rRect.x, y, rRect.x + rRect.w, y);
        }

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < targetDelay)
            SDL_Delay(targetDelay - frameTime);
    }

}

void ItemList::select(int index)
{
    if (selected != -1)
        endEdit();

    if (index >= 0 && index < static_cast<int>(equations.size()))
    {
        selected = index;
        cursorPos = static_cast<int>(equations[selected].expression.size());
    }
}

void ItemList::handleScroll(int delta)
{
    using namespace Constants;
    int visibleItems = (WINDOW_HEIGHT - (BUTTON_HEIGHT << 1 + MARGIN * 3)) / TOTAL_HEIGHT;
    scrollOffset = std::max(0, std::min(scrollOffset - delta, static_cast<int>(equations.size()) - visibleItems));
}