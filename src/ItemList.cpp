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

void ItemList::add(const std::string& item)
{
    Equation eq;
    eq.expression = item;
    equations.push_back(std::move(eq));
}

void ItemList::removeSelected()
{
    if (selected >= 0 && selected < static_cast<int>(equations.size()))
    {
        equations.erase(equations.begin() + selected);
        selected = -1;
        editing = -1;
    }
}

void ItemList::startEdit(int index)
{
    if (index >= 0 && index < static_cast<int>(equations.size()))
    {
        editing = index;
        editBuffer = equations[index].expression;
        cursorPos = static_cast<int>(editBuffer.length());
    }
}

void ItemList::endEdit()
{
    if (editing != -1)
    {
        equations[editing].expression = editBuffer;
        editing = -1;
    }
}

void ItemList::handleInput(const SDL_Event& e)
{
    if (editing == -1)
        return;
    if (e.type == SDL_TEXTINPUT)
    {
        editBuffer.insert(static_cast<size_t>(cursorPos), e.text.text);
        cursorPos += static_cast<int>(std::strlen(e.text.text));
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
                    cursorPos--;
                break;
            case SDLK_RIGHT:
                if (cursorPos < static_cast<int>(editBuffer.length()))
                    cursorPos++;
                break;
            case SDLK_BACKSPACE: 
                if (!editBuffer.empty() && cursorPos > 0)
                {
                    editBuffer.erase(cursorPos - 1, 1);
                    cursorPos--;
                }
                break;
            case SDLK_DELETE:
                if (cursorPos < static_cast<int>(editBuffer.length()))
                {
                    editBuffer.erase(cursorPos, 1);
                }
                break;
        }
    }
}

void ItemList::select(int index)
{
    if (selected != -1)
        equations[selected].expression = editBuffer;
    selected = index;
    if (index != -1)
        startEdit(index);
    else
        endEdit();
}

void ItemList::handleScroll(int delta)
{
    using namespace Constants;
    const auto visibleItems = static_cast<int>((WINDOW_HEIGHT - (BUTTON_HEIGHT * 2 + MARGIN * 3)) / TOTAL_HEIGHT);
    scrollOffset = std::max(0, std::min(
        static_cast<int>(scrollOffset - delta),
        static_cast<int>(equations.size()) - visibleItems
    ));
}