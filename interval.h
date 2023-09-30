//
// Created by user on 9/10/2023.
//

#ifndef RAYTRACINGINONEWEEKEND_INTERVAL_H
#define RAYTRACINGINONEWEEKEND_INTERVAL_H

class interval {
public:
    double min, max;

    interval() : min(+infinity), max(-infinity) {} // Default interval is empty

    interval(double _min, double _max) : min(_min), max(_max) {}

    interval(const interval& a, const interval& b) : min(std::fmin(a.min, b.min)), max(std::fmax(a.max, b.max)) {}

    [[nodiscard]] bool contains(double x) const
    {
        return min <= x && x <= max;
    }

    [[nodiscard]] bool surrounds(double x) const
    {
        return min < x && x < max;
    }

    [[nodiscard]] double clamp(double x) const
    {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    [[nodiscard]] double size() const
    {
        return max - min;
    }

    [[nodiscard]] interval expand(double delta) const
    {
        auto padding = delta / 2;
        return {min - padding, max + padding};
    }

    static const interval empty, universe;
};

interval operator+(const interval& ival, double displacement)
{
    return {ival.min + displacement, ival.max + displacement};
}

interval operator+(double displacement, const interval& ival)
{
    return ival + displacement;
}

const interval interval::empty   (+infinity, -infinity);
const interval interval::universe(-infinity, +infinity);

#endif //RAYTRACINGINONEWEEKEND_INTERVAL_H
