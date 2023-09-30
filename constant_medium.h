//
// Created by user on 9/15/2023.
//

#ifndef RAYTRACINGINONEWEEKEND_CONSTANT_MEDIUM_H
#define RAYTRACINGINONEWEEKEND_CONSTANT_MEDIUM_H

#include "rtweekend.h"

#include "hittable.h"
#include "material.h"
#include "texture.h"

using color = vec3;

class constant_medium : public hittable
{
public:
    constant_medium(shared_ptr<hittable> b, double d, shared_ptr<texture> a)
            : boundary(b), neg_inv_density(-1/d), phase_function(make_shared<isotropic>(a))
    {}

    constant_medium(shared_ptr<hittable> b, double d, color c)
            : boundary(b), neg_inv_density(-1/d), phase_function(make_shared<isotropic>(c))
    {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        // Print occasional samples when debugging. To enable, set enableDebug true.
        const bool enableDebug = false;
        const bool debugging = enableDebug && random_double() < 0.00001;

        hit_record rec1, rec2;

        // Entry point to volume
        if (!boundary->hit(r, interval::universe, rec1))
            return false;

        // Exit point from volume
        if (!boundary->hit(r, interval(rec1.t+0.0001, infinity), rec2))
            return false;

        if (debugging) std::clog << "\nray_tmin=" << rec1.t << ", ray_tmax=" << rec2.t << '\n';

        // Extend ray
        if (rec1.t < ray_t.min) rec1.t = ray_t.min;
        if (rec2.t > ray_t.max) rec2.t = ray_t.max;

        // Discard degenerate volumes
        if (rec1.t >= rec2.t)
            return false;

        // Ammend degenerate ray extension
        if (rec1.t < 0)
            rec1.t = 0;

        // Model constant medium as having a random chance to scatter rays proportional to delta(len)
        // If the solution is beyond the volume endpoint, it didn't scatter
        auto ray_length = r.direction().length();
        auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length; // I think this is [0,1)?
        auto hit_distance = neg_inv_density * log(random_double()); // Select random point on -1/k * log([0,1))

        // Didn't scatter
        if (hit_distance > distance_inside_boundary)
            return false;

        rec.t = rec1.t + hit_distance / ray_length;
        rec.p = r.at(rec.t);

        if (debugging)
        {
            std::clog << "hit_distance = " <<  hit_distance << '\n'
                      << "rec.t = " <<  rec.t << '\n'
                      << "rec.p = " <<  rec.p << '\n';
        }

        rec.normal = vec3(1,0,0);  // arbitrary
        rec.front_face = true;     // also arbitrary
        rec.mat = phase_function;

        return true;
    }

    [[nodiscard]] aabb bounding_box() const override { return boundary->bounding_box(); }

private:
    shared_ptr<hittable> boundary;
    double neg_inv_density;
    shared_ptr<material> phase_function;
};

#endif //RAYTRACINGINONEWEEKEND_CONSTANT_MEDIUM_H
