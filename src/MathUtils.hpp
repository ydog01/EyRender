#pragma once
#include <cmath>
#include <sstream>
#include <iomanip>
#include "Constants.hpp"

struct Point2D
{
    double x = 0.0;
    double y = 0.0;
    Point2D operator-(const Point2D& other) const;
};

struct MathRange
{
    double xMin = -15.0, xMax = 15.0;
    double yMin = -10.0, yMax = 10.0;
    double xSpan() const;
    double ySpan() const;
};

std::string formatNumber(double value, int precision);
Point2D mathToScreen(const Point2D& mathPoint, const MathRange& range);
Point2D screenToMath(int sx, int sy, const MathRange& range);