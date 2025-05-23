#pragma once
#include "Equation.hpp"
#include "Constants.hpp"
#include <vector>
#include <iterator>

class ItemList
{
private:
    std::vector<Equation> equations;
    int selected = -1;
    size_t cursorPos = 0;
    int scrollOffset = 0;
    SDL_Rect addButton{0, 0, 0, 0};
    SDL_Rect delButton{0, 0, 0, 0};

public:
    ItemList() = default;
    void updateButtonPositions(int panelX);
    int add(const std::string& item);
    void removeSelected();
    void endEdit();
    void handleInput(const SDL_Event& e, SDL_Renderer* renderer);
    void select(int index);
    void handleScroll(int delta);
    void selectColor(SDL_Renderer* renderer);
    
    std::vector<Equation>& getEquations() { return equations; }
    int getSelected() const { return selected; }
    bool isEditing() const { return selected != -1; }
    const SDL_Rect& getAddButton() const { return addButton; }
    const SDL_Rect& getDelButton() const { return delButton; }
    int getCursorPos() const { return cursorPos; }
    int getScrollOffset() const { return scrollOffset; }
};