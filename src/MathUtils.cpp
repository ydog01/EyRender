#include "MathUtils.hpp"

Point2D Point2D::operator-(const Point2D& other) const
{
    return {x - other.x, y - other.y};
}

double MathRange::xSpan() const
{
    return xMax - xMin;
}

double MathRange::ySpan() const
{
    return yMax - yMin;
}

std::string formatNumber(double value, int precision)
{
    if (std::isnan(value) || std::isinf(value))
        return "NaN";
    std::ostringstream oss;
    const double absValue = std::abs(value);
    const bool isscientific = (absValue >= 1e4 || (absValue > 0 && absValue <= 1e-4)) && absValue != 0.0;
    if (isscientific)
        oss << std::scientific << std::setprecision(std::max(1, precision - 1));
    else
        oss << std::fixed << std::setprecision(precision);
    oss << value;
    std::string s = oss.str();
    
    size_t dot_pos = s.find('.');
    if (!isscientific&&dot_pos != std::string::npos)
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

Point2D mathToScreen(const Point2D& mathPoint, const MathRange& range)
{
    return {
        (mathPoint.x - range.xMin) / range.xSpan() * 1200.0,
        (range.yMax - mathPoint.y) / range.ySpan() * Constants::WINDOW_HEIGHT
    };
}

Point2D screenToMath(int sx, int sy, const MathRange& range)
{
    return {
        range.xMin + static_cast<double>(sx) * range.xSpan() / 1200.0,
        range.yMax - static_cast<double>(sy) * range.ySpan() / Constants::WINDOW_HEIGHT
    };
}