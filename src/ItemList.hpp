#pragma once
#include "Equation.hpp"
#include "Constants.hpp"
#include <vector>

class ItemList
{
private:
    std::vector<Equation> equations;
    int selected = -1;
    int cursorPos = 0;
    int scrollOffset = 0;
    SDL_Rect addButton{0, 0, 0, 0};
    SDL_Rect delButton{0, 0, 0, 0};

public:
    ItemList() = default;
    void updateButtonPositions(int panelX);
    void add(const std::string& item);
    void removeSelected();
    void endEdit();
    void handleInput(const SDL_Event& e);
    void select(int index);
    void handleScroll(int delta);
    
    const std::vector<Equation>& getEquations() const { return equations; }
    int getSelected() const { return selected; }
    bool isEditing() const { return selected != -1; }
    const SDL_Rect& getAddButton() const { return addButton; }
    const SDL_Rect& getDelButton() const { return delButton; }
    int getCursorPos() const { return cursorPos; }
    int getScrollOffset() const { return scrollOffset; }
};