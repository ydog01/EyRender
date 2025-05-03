#include "ItemList.hpp"
#include <iterator>

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
            
        auto temp = Equation::evaluator.parse(eq.expression.substr(pos));

        eq.value.consts.insert(eq.value.consts.end(), temp.consts.begin(), temp.consts.end());
        eq.value.funcs.insert(eq.value.funcs.end(), temp.funcs.begin(), temp.funcs.end());
        eq.value.vars.insert(eq.value.vars.end(), temp.vars.begin(), temp.vars.end());
        eq.value.index += temp.index;
        eq.value.index.push_back('f');
        eq.value.funcs.push_back(Equation::evaluator.infix_ops->search("-")->data);
    }
    catch (...)
    {
        eq.value.clear();
        eq.type = RelationalOperator::INVALID;
    }
}

void ItemList::handleInput(const SDL_Event& e)
{
    if (selected == -1)
        return;

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
                    cursorPos--;
                break;
            case SDLK_RIGHT:
                if (cursorPos < static_cast<int>(eq.expression.size()))
                    cursorPos++;
                break;
            case SDLK_BACKSPACE:
                if (cursorPos > 0)
                {
                    eq.expression.erase(cursorPos - 1, 1);
                    cursorPos--;
                }
                break;
            case SDLK_DELETE:
                if (cursorPos < static_cast<int>(eq.expression.size()))
                    eq.expression.erase(cursorPos, 1);
                break;
        }
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
    int visibleItems = (WINDOW_HEIGHT - (BUTTON_HEIGHT * 2 + MARGIN * 3)) / TOTAL_HEIGHT;
    scrollOffset = std::max(0, std::min(scrollOffset - delta, static_cast<int>(equations.size()) - visibleItems));
}