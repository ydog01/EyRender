#include "MathVisualizer.hpp"

int main(int argc, char* argv[])
{
    MathVisualizer app;
    if (!app.init())
        return -1;
    app.run();
    app.cleanup();
    return 0;
}