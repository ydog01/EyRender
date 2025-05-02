#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#undef main

constexpr int PANEL_WIDTH = 300;
constexpr int BUTTON_HEIGHT = 30;
constexpr int ITEM_HEIGHT = 35;
constexpr int LINE_HEIGHT = 2;
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

struct Point2D
{
    double x = 0.0;
    double y = 0.0;

    Point2D operator-(const Point2D &other) const
    {
        return {x - other.x, y - other.y};
    }
};

std::string formatNumber(double value, int precision)
{
    if (std::isnan(value) || std::isinf(value))
        return "NaN";

    std::ostringstream oss;
    const double absValue = std::abs(value);
    if ((absValue >= 1e4 || (absValue > 0 && absValue <= 1e-4)) && absValue != 0.0)
    {
        oss << std::scientific << std::setprecision(std::max(1, precision - 1));
    }
    else
    {
        oss << std::fixed << std::setprecision(precision);
    }
    oss << value;
    std::string s = oss.str();

    size_t dot_pos = s.find('.');
    if (dot_pos != std::string::npos)
    {
        s = s.substr(0, s.find_last_not_of('0', s.size() - 1) + 1);
        if (s.back() == '.')
            s.pop_back();
    }

    size_t e_pos = s.find("e+00");
    if (e_pos != std::string::npos)
        s.replace(e_pos, 4, "e");
    if ((e_pos = s.find("e0")) != std::string::npos)
        s.erase(e_pos + 1, 1);

    return s;
}

struct MathRange
{
    double xMin = -15.0;
    double xMax = 15.0;
    double yMin = -10.0;
    double yMax = 10.0;

    [[nodiscard]] double xSpan() const { return xMax - xMin; }
    [[nodiscard]] double ySpan() const { return yMax - yMin; }
};

enum class EquationType
{
    EXPLICIT,
    IMPLICIT,
    INEQUALITY
};

struct Equation
{
    std::string expression;
    EquationType type = EquationType::EXPLICIT;
    bool valid = false;
};

class ItemList
{
private:
    std::vector<Equation> equations;
    int selected = -1;
    int editing = -1;
    int cursorPos = 0;
    int scrollOffset = 0;
    SDL_Rect addButton{0, 0, 0, 0};
    SDL_Rect delButton{0, 0, 0, 0};
    std::string editBuffer;

public:
    ItemList() = default;

    void updateButtonPositions(int panelX)
    {
        addButton = {
            panelX + MARGIN,
            WINDOW_HEIGHT - BUTTON_HEIGHT * 2 - MARGIN * 2,
            PANEL_WIDTH - MARGIN * 2,
            BUTTON_HEIGHT};

        delButton = {
            panelX + MARGIN,
            WINDOW_HEIGHT - BUTTON_HEIGHT - MARGIN,
            PANEL_WIDTH - MARGIN * 2,
            BUTTON_HEIGHT};
    }

    void add(const std::string &item)
    {
        Equation eq;
        eq.expression = item;
        equations.push_back(std::move(eq));
    }

    void removeSelected()
    {
        if (selected >= 0 && selected < static_cast<int>(equations.size()))
        {
            equations.erase(equations.begin() + selected);
            selected = -1;
            editing = -1;
        }
    }

    void startEdit(int index)
    {
        if (index >= 0 && index < static_cast<int>(equations.size()))
        {
            editing = index;
            editBuffer = equations[index].expression;
            cursorPos = static_cast<int>(editBuffer.length());
        }
    }

    void endEdit()
    {
        if (editing != -1)
        {
            equations[editing].expression = editBuffer;
            editing = -1;
        }
    }

    void handleInput(const SDL_Event &e)
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
                    editBuffer.erase(static_cast<size_t>(cursorPos) - 1, 1);
                    cursorPos--;
                }
                break;
            case SDLK_DELETE:
                if (cursorPos < static_cast<int>(editBuffer.length()))
                {
                    editBuffer.erase(static_cast<size_t>(cursorPos), 1);
                }
                break;
            }
        }
    }

    void select(int index)
    {
        if (selected != -1)
            equations[selected].expression = editBuffer;
        selected = index;
        if (index != -1)
            startEdit(index);
        else
            endEdit();
    }

    void handleScroll(int delta)
    {
        const auto visibleItems = static_cast<int>(
            (WINDOW_HEIGHT - (BUTTON_HEIGHT * 2 + MARGIN * 3)) / ITEM_HEIGHT);
        scrollOffset = std::max(0, std::min(
                                       static_cast<int>(static_cast<int64_t>(scrollOffset) - delta),
                                       static_cast<int>(equations.size()) - visibleItems));
    }

    [[nodiscard]] const std::vector<Equation> &getEquations() const { return equations; }
    [[nodiscard]] int getSelected() const { return selected; }
    [[nodiscard]] bool isEditing() const { return editing != -1; }
    [[nodiscard]] const SDL_Rect &getAddButton() const { return addButton; }
    [[nodiscard]] const SDL_Rect &getDelButton() const { return delButton; }
    [[nodiscard]] const std::string &getEditBuffer() const { return editBuffer; }
    [[nodiscard]] int getCursorPos() const { return cursorPos; }
    [[nodiscard]] int getScrollOffset() const { return scrollOffset; }
};

Point2D mathToScreen(const Point2D &mathPoint, const MathRange &range)
{
    return {
        (mathPoint.x - range.xMin) / range.xSpan() * 1200.0,
        (range.yMax - mathPoint.y) / range.ySpan() * static_cast<double>(WINDOW_HEIGHT)};
}

Point2D screenToMath(int sx, int sy, const MathRange &range)
{
    return {
        range.xMin + static_cast<double>(sx) * range.xSpan() / 1200.0,
        range.yMax - static_cast<double>(sy) * range.ySpan() / static_cast<double>(WINDOW_HEIGHT)};
}

void drawCoordinateGrid(SDL_Renderer *renderer, TTF_Font *font, const MathRange &range)
{
    const double baseGridSize = std::pow(10.0, std::floor(std::log10(range.xSpan())));
    const double gridSize = baseGridSize / 2.0;
    const int precision = std::max(0, 3 - static_cast<int>(std::log10(gridSize)));

    SDL_SetRenderDrawColor(renderer, GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b, GRID_COLOR.a);
    for (double x = std::ceil(range.xMin / gridSize) * gridSize; x <= range.xMax; x += gridSize)
    {
        if (std::abs(x) < 1e-10)
            continue;
        const Point2D p1 = mathToScreen({x, range.yMin}, range);
        const Point2D p2 = mathToScreen({x, range.yMax}, range);
        SDL_RenderDrawLine(renderer,
                           static_cast<int>(std::round(p1.x)),
                           static_cast<int>(std::round(p1.y)),
                           static_cast<int>(std::round(p2.x)),
                           static_cast<int>(std::round(p2.y)));
    }

    for (double y = std::ceil(range.yMin / gridSize) * gridSize; y <= range.yMax; y += gridSize)
    {
        if (std::abs(y) < 1e-10)
            continue;
        const Point2D p1 = mathToScreen({range.xMin, y}, range);
        const Point2D p2 = mathToScreen({range.xMax, y}, range);
        SDL_RenderDrawLine(renderer,
                           static_cast<int>(std::round(p1.x)),
                           static_cast<int>(std::round(p1.y)),
                           static_cast<int>(std::round(p2.x)),
                           static_cast<int>(std::round(p2.y)));
    }

    SDL_SetRenderDrawColor(renderer, AXIS_COLOR.r, AXIS_COLOR.g, AXIS_COLOR.b, AXIS_COLOR.a);
    const Point2D xStart = mathToScreen({range.xMin, 0}, range);
    const Point2D xEnd = mathToScreen({range.xMax, 0}, range);
    const Point2D yStart = mathToScreen({0, range.yMin}, range);
    const Point2D yEnd = mathToScreen({0, range.yMax}, range);
    SDL_RenderDrawLine(renderer,
                       static_cast<int>(std::round(xStart.x)),
                       static_cast<int>(std::round(xStart.y)),
                       static_cast<int>(std::round(xEnd.x)),
                       static_cast<int>(std::round(xEnd.y)));
    SDL_RenderDrawLine(renderer,
                       static_cast<int>(std::round(yStart.x)),
                       static_cast<int>(std::round(yStart.y)),
                       static_cast<int>(std::round(yEnd.x)),
                       static_cast<int>(std::round(yEnd.y)));

    SDL_Color textColor = TEXT_COLOR;

    for (double x = std::ceil(range.xMin / gridSize) * gridSize; x <= range.xMax; x += gridSize)
    {
        if (std::abs(x) < 1e-10)
            continue;

        const Point2D p = mathToScreen({x, 0}, range);
        const std::string label = formatNumber(x, precision);

        SDL_Surface *surface = TTF_RenderUTF8_Blended(font, label.c_str(), textColor);
        if (!surface)
            continue;

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        const SDL_Rect rect = {
            static_cast<int>(std::round(p.x)) - surface->w / 2, 
            static_cast<int>(std::round(p.y)) + 5,              
            surface->w,
            surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    for (double y = std::ceil(range.yMin / gridSize) * gridSize; y <= range.yMax; y += gridSize)
    {
        if (std::abs(y) < 1e-10)
            continue;

        const Point2D p = mathToScreen({0, y}, range);
        const std::string label = formatNumber(y, precision);

        SDL_Surface *surface = TTF_RenderUTF8_Blended(font, label.c_str(), textColor);
        if (!surface)
            continue;

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        const SDL_Rect rect = {
            static_cast<int>(std::round(p.x)) - surface->w - 5, 
            static_cast<int>(std::round(p.y)) - surface->h / 2, 
            surface->w,
            surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}

class MathVisualizer
{
private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    TTF_Font *font = nullptr;
    MathRange currentRange{-15.0, 15.0, -10.0, 10.0};
    ItemList itemList;
    bool isRunning = true;
    bool isDragging = false;
    Point2D dragStart{0.0, 0.0};
    MathRange dragStartRange{-15.0, 15.0, -10.0, 10.0};
    int panelX = 1200;
    Uint32 cursorBlink = 0;
    int visibleItems = 0;

    void renderText(const std::string &text, int x, int y, int maxWidth)
    {
        SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text.c_str(), TEXT_COLOR);
        if (!surface)
            return;

        if (surface->w > maxWidth)
        {
            const float scale = static_cast<float>(maxWidth) / surface->w;
            const SDL_Rect dstRect = {x, y,
                                      static_cast<int>(surface->w * scale),
                                      static_cast<int>(surface->h * scale)};
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        else
        {
            const SDL_Rect dstRect = {x, y, surface->w, surface->h};
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }

    void renderPanel()
    {
        SDL_Rect panelRect = {panelX, 0, PANEL_WIDTH, WINDOW_HEIGHT};
        SDL_SetRenderDrawColor(renderer, PANEL_COLOR.r, PANEL_COLOR.g, PANEL_COLOR.b, 255);
        SDL_RenderFillRect(renderer, &panelRect);

        const SDL_Rect &addBtn = itemList.getAddButton();
        const SDL_Rect &delBtn = itemList.getDelButton();
        SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, 255);
        SDL_RenderFillRect(renderer, &addBtn);
        SDL_RenderFillRect(renderer, &delBtn);
        renderText("+ Add", addBtn.x + 10, addBtn.y + 3, addBtn.w - 20);
        renderText("- Delete", delBtn.x + 10, delBtn.y + 3, delBtn.w - 20);

        visibleItems = (WINDOW_HEIGHT - (BUTTON_HEIGHT * 2 + MARGIN * 3)) / ITEM_HEIGHT;
        const int startIdx = itemList.getScrollOffset();
        const int endIdx = std::min(std::max(startIdx + visibleItems - 2, 0), static_cast<int>(itemList.getEquations().size()));

        int yPos = MARGIN;
        for (int i = startIdx; i < endIdx; ++i)
        {
            SDL_Rect itemRect = {panelX + MARGIN, yPos, PANEL_WIDTH - MARGIN * 2, ITEM_HEIGHT};

            if (i == itemList.getSelected())
            {
                SDL_SetRenderDrawColor(renderer, SELECT_COLOR.r, SELECT_COLOR.g, SELECT_COLOR.b, 255);
                SDL_RenderFillRect(renderer, &itemRect);
            }

            const std::string &text = itemList.isEditing() && i == itemList.getSelected() ? itemList.getEditBuffer() : itemList.getEquations()[i].expression;

            renderText(text, itemRect.x + 5, itemRect.y + 3, itemRect.w - 10);

            if (itemList.isEditing() && i == itemList.getSelected())
            {
                const std::string beforeCursor = text.substr(0, static_cast<size_t>(itemList.getCursorPos()));
                int textWidth;
                TTF_SizeText(font, beforeCursor.c_str(), &textWidth, nullptr);
                const int cursorX = itemRect.x + 5 + textWidth;

                if (SDL_GetTicks() - cursorBlink < 500)
                {
                    SDL_SetRenderDrawColor(renderer, EDIT_COLOR.r, EDIT_COLOR.g, EDIT_COLOR.b, 255);
                    SDL_RenderDrawLine(renderer, cursorX, itemRect.y + 2, cursorX, itemRect.y + ITEM_HEIGHT - 4);
                }
            }

            SDL_SetRenderDrawColor(renderer, GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b, GRID_COLOR.a);
            const SDL_Rect lineRect{panelX + MARGIN, yPos + ITEM_HEIGHT, PANEL_WIDTH - (MARGIN << 1), LINE_HEIGHT};
            SDL_RenderFillRect(renderer, &lineRect);

            yPos += ITEM_HEIGHT + LINE_HEIGHT;
        }

        if (SDL_GetTicks() - cursorBlink > 1000)
        {
            cursorBlink = SDL_GetTicks();
        }
    }

    void handlePanelClick(const SDL_MouseButtonEvent &e)
    {
        const SDL_Point mouse{e.x, e.y};
        const SDL_Rect &addBtn = itemList.getAddButton();
        const SDL_Rect &delBtn = itemList.getDelButton();

        if (SDL_PointInRect(&mouse, &addBtn))
        {
            itemList.add(std::string());
            itemList.select(static_cast<int>(itemList.getEquations().size()) - 1);
        }
        else if (SDL_PointInRect(&mouse, &delBtn))
        {
            itemList.removeSelected();
        }
        else
        {
            const int listStartY = MARGIN;
            const int listEndY = MARGIN + visibleItems * ITEM_HEIGHT;

            if (mouse.y >= listStartY && mouse.y < listEndY)
            {
                const int itemIndex = itemList.getScrollOffset() + (mouse.y - listStartY) / ITEM_HEIGHT;
                if (itemIndex < static_cast<int>(itemList.getEquations().size()))
                {
                    itemList.select(itemIndex);
                }
                else
                {
                    itemList.select(-1);
                }
            }
            else
            {
                itemList.select(-1);
            }
        }
    }

public:
    MathVisualizer() = default;

    bool init()
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
            return false;

        window = SDL_CreateWindow("Math Visualizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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
        return true;
    }

    void handleEvents()
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
                {
                    handlePanelClick(e.button);
                }
                else
                {
                    if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        isDragging = true;
                        dragStart = {static_cast<double>(e.button.x), static_cast<double>(e.button.y)};
                        dragStartRange = currentRange;
                        itemList.endEdit();
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    isDragging = false;
                }
                break;

            case SDL_MOUSEMOTION:
                if (isDragging)
                {
                    const Point2D current = {static_cast<double>(e.motion.x), static_cast<double>(e.motion.y)};
                    const Point2D delta = screenToMath(static_cast<int>(current.x), static_cast<int>(current.y), currentRange) -
                                          screenToMath(static_cast<int>(dragStart.x), static_cast<int>(dragStart.y), currentRange);
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
                {
                    itemList.handleScroll(e.wheel.y);
                }
                else
                {
                    const double zoomCenterX = currentRange.xMin + currentRange.xSpan() / 2.0;
                    const double zoomCenterY = currentRange.yMin + currentRange.ySpan() / 2.0;
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

    void render()
    {
        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 255);
        SDL_RenderClear(renderer);

        SDL_Rect graphArea{0, 0, 1200, WINDOW_HEIGHT};
        SDL_RenderSetClipRect(renderer, &graphArea);
        drawCoordinateGrid(renderer, font, currentRange);
        SDL_RenderSetClipRect(renderer, nullptr);

        renderPanel();
        SDL_RenderPresent(renderer);
    }

    void run()
    {
        while (isRunning)
        {
            handleEvents();
            render();
            SDL_Delay(16);
        }
    }

    void cleanup()
    {
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    }
};

int main(int argc, char *argv[])
{
    MathVisualizer app;
    if (!app.init())
        return -1;
    app.run();
    app.cleanup();
    return 0;
}