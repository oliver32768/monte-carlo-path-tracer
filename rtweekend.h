//
// Created by user on 9/10/2023.
//

#ifndef RAYTRACINGINONEWEEKEND_RTWEEKEND_H
#define RAYTRACINGINONEWEEKEND_RTWEEKEND_H

#include <exception>
#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <chrono>
#include <cassert>
#include <thread>
#include <functional>
#include <syncstream>
#include <omp.h>

// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Const globals
const double epsilon = std::numeric_limits<float>::epsilon();
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Non-const globals
std::random_device rd;
std::mt19937 gen(rd());

// Utility Functions
inline double degrees_to_radians(double degrees)
{
    return degrees * pi / 180.0;
}

inline double random_double()
{
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(gen);
}

inline double random_double(double min, double max)
{
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(gen);
}

inline int random_int(int min, int max)
{
    std::uniform_int_distribution<> distribution(min, max);
    return distribution(gen);
}

template <typename Head0, typename Head1, typename... Tail>
constexpr auto vmin(const Head0 &head0, const Head1 &head1, const Tail &... tail)
{
    if constexpr (sizeof...(tail) == 0)
    {
        return head0 < head1 ? head0 : head1;
    }
    else
    {
        return vmin(vmin(head0, head1), tail...);
    }
}

template <typename Head0, typename Head1, typename... Tail>
constexpr auto vmax(const Head0 &head0, const Head1 &head1, const Tail &... tail)
{
    if constexpr (sizeof...(tail) == 0)
    {
        return head0 < head1 ? head1 : head0;
    }
    else
    {
        return vmax(vmax(head0, head1), tail...);
    }
}

// Common Headers
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif //RAYTRACINGINONEWEEKEND_RTWEEKEND_H
