//
// Created by user on 9/14/2023.
//

#ifndef RAYTRACINGINONEWEEKEND_TRIANGLE_H
#define RAYTRACINGINONEWEEKEND_TRIANGLE_H

#include "quad.h"

class triangle : public quad
{
public:
    triangle(const point3& _Q, const vec3& _u, const vec3& _v, shared_ptr<material> m)
            : quad(_Q, _u, _v, m)
    {
        set_bounding_box();
    }

    void set_bounding_box() override
    {
        point3 q = Q;
        point3 qu = Q + u;
        point3 qv = Q + v;

        double min_x = vmin(q.x(), qu.x(), qv.x());
        double min_y = vmin(q.y(), qu.y(), qv.y());
        double min_z = vmin(q.z(), qu.z(), qv.z());

        double max_x = vmax(q.x(), qu.x(), qv.x());
        double max_y = vmax(q.y(), qu.y(), qv.y());
        double max_z = vmax(q.z(), qu.z(), qv.z());

        vec3 min(min_x, min_y, min_z);
        vec3 max(max_x, max_y, max_z);

        bbox = aabb(min, max).pad();
    }

    bool is_interior(double a, double b, hit_record& rec) const override
    {
        // Given the hit point in plane coordinates, return false if it is outside the
        // primitive, otherwise set the hit record UV coordinates and return true.
        if (a < 0 || b < 0 || 1 < a + b)
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }
};

#endif //RAYTRACINGINONEWEEKEND_TRIANGLE_H
