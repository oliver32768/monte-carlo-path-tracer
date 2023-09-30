//
// Created by user on 9/12/2023.
//

#ifndef RAYTRACINGINONEWEEKEND_AABB_H
#define RAYTRACINGINONEWEEKEND_AABB_H

#include "rtweekend.h"

class aabb
{
public:
    interval x, y, z;

    aabb() = default; // The default AABB is empty, since intervals are empty by default.

    aabb(const interval& ix, const interval& iy, const interval& iz) : x(ix), y(iy), z(iz) {}

    aabb(const point3& a, const point3& b)
    {
        // Treat the two points a and b as extrema for the bounding box, so we don't require a
        // particular minimum/maximum coordinate order.
        x = interval(std::fmin(a[0], b[0]), std::fmax(a[0], b[0]));
        y = interval(std::fmin(a[1], b[1]), std::fmax(a[1], b[1]));
        z = interval(std::fmin(a[2], b[2]), std::fmax(a[2], b[2]));
    }

    aabb(const aabb& box0, const aabb& box1)
    {
        x = interval(box0.x, box1.x);
        y = interval(box0.y, box1.y);
        z = interval(box0.z, box1.z);
    }

    [[nodiscard]] const interval& axis(int n) const
    {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    [[nodiscard]] bool hit(const ray& r, interval ray_t) const
    {
        for (int i = 0; i < 3; ++i)
        {
            double invD = 1 / r.direction()[i];
            double orig = r.origin()[i];

            double t0 = (axis(i).min - orig) * invD;
            double t1 = (axis(i).max - orig) * invD;

            if (invD < 0) std::swap(t0, t1); // This is the only way (t0 < t1) holds

            if (t0 > ray_t.min) ray_t.min = t0;
            if (t1 < ray_t.max) ray_t.max = t1;

            if (ray_t.max <= ray_t.min) return false;
        }

        return true;
    }

    [[nodiscard]] aabb pad() const
    {
        // Return an AABB that has no side narrower than some delta, padding if necessary.
        double delta = 0.0001;
        interval new_x = (x.size() >= delta) ? x : x.expand(delta);
        interval new_y = (y.size() >= delta) ? y : y.expand(delta);
        interval new_z = (z.size() >= delta) ? z : z.expand(delta);

        return {new_x, new_y, new_z};
    }
};

aabb operator+(const aabb& bbox, const vec3& offset)
{
    return {bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z()};
}

aabb operator+(const vec3& offset, const aabb& bbox)
{
    return bbox + offset;
}

#endif //RAYTRACINGINONEWEEKEND_AABB_H
