//
// Created by user on 9/14/2023.
//

#ifndef RAYTRACINGINONEWEEKEND_QUAD_H
#define RAYTRACINGINONEWEEKEND_QUAD_H

#include "rtweekend.h"
#include "hittable.h"
#include <cmath>

class quad : public hittable
{
public:
    quad(const point3& _Q, const vec3& _u, const vec3& _v, shared_ptr<material> m)
            : Q(_Q), u(_u), v(_v), mat(m)
    {
        vec3 n = cross(u, v); // normal to the plane containing the quad
        normal = unit_vector(n);
        D = dot(normal, Q); // Q is on the plane by construction, so use it to solve for D
        w = n / dot(n,n);

        set_bounding_box();
    }

    virtual void set_bounding_box()
    {
        point3 q = Q;
        point3 qu = Q + u;
        point3 qv = Q + v;
        point3 quv = Q + u + v;

        double min_x = vmin(q.x(), qu.x(), qv.x(), quv.x());
        double min_y = vmin(q.y(), qu.y(), qv.y(), quv.y());
        double min_z = vmin(q.z(), qu.z(), qv.z(), quv.z());

        double max_x = vmax(q.x(), qu.x(), qv.x(), quv.x());
        double max_y = vmax(q.y(), qu.y(), qv.y(), quv.y());
        double max_z = vmax(q.z(), qu.z(), qv.z(), quv.z());

        vec3 min(min_x, min_y, min_z);
        vec3 max(max_x, max_y, max_z);

        bbox = aabb(min, max).pad();
    }

    [[nodiscard]] aabb bounding_box() const override { return bbox; }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        // t = D - dot(n, P) / dot(n, d)
        double denom = dot(normal, r.direction());

        // No hit if the ray is parallel to the plane.
        if (fabs(denom) < 1e-8)
            return false;

        // Return false if the hit point parameter t is outside the ray interval.
        double t = (D - dot(normal, r.origin())) / denom;

        if (!ray_t.contains(t))
            return false;

        // Determine the hit point lies within the planar shape using its plane coordinates.
        point3 intersection = r.at(t);
        vec3 planar_hitpt_vector = intersection - Q;
        double alpha = dot(w, cross(planar_hitpt_vector, v));
        double beta = dot(w, cross(u, planar_hitpt_vector));

        if (!is_interior(alpha, beta, rec))
            return false;

        // Ray hits the 2D shape; set the rest of the hit record and return true.

        rec.t = t;
        rec.p = intersection;
        rec.mat = mat;
        rec.set_face_normal(r, normal);

        return true;
    }

    virtual bool is_interior(double a, double b, hit_record& rec) const
    {
        // Given the hit point in plane coordinates, return false if it is outside the
        // primitive, otherwise set the hit record UV coordinates and return true.
        if ((a < 0) || (1 < a) || (b < 0) || (1 < b))
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }

protected:
    point3 Q;
    vec3 u, v;
    shared_ptr<material> mat;
    aabb bbox;
    vec3 normal;
    double D; // dot(normal, p) = D for all p in the plane of the quad
    vec3 w; // caching constant term n/dot(n, n) in solving for alpha and beta in ray plane-basis intersection
};

// Returns the 3D box (six sides) that contains the two opposite vertices a & b.
inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat)
{
    // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

    auto sides = make_shared<hittable_list>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    auto min = point3(fmin(a.x(), b.x()), fmin(a.y(), b.y()), fmin(a.z(), b.z()));
    auto max = point3(fmax(a.x(), b.x()), fmax(a.y(), b.y()), fmax(a.z(), b.z()));

    auto dx = vec3(max.x() - min.x(), 0, 0);
    auto dy = vec3(0, max.y() - min.y(), 0);
    auto dz = vec3(0, 0, max.z() - min.z());

    sides->add(make_shared<quad>(point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // front
    sides->add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // right
    sides->add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // back
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // left
    sides->add(make_shared<quad>(point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // top
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // bottom

    return sides;
}

#endif //RAYTRACINGINONEWEEKEND_QUAD_H
