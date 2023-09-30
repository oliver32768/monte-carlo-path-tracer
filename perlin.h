//
// Created by user on 9/13/2023.
//

#ifndef RAYTRACINGINONEWEEKEND_PERLIN_H
#define RAYTRACINGINONEWEEKEND_PERLIN_H

#include "rtweekend.h"

class perlin
{
public:
    perlin()
    {
        ranvec = new vec3[point_count];
        for (int i = 0; i < point_count; ++i)
        {
            ranvec[i] = unit_vector(vec3::random(-1,1));
        }

        perm_x = perlin_generate_perm();
        perm_y = perlin_generate_perm();
        perm_z = perlin_generate_perm();
    }

    ~perlin()
    {
        delete[] ranvec;
        delete[] perm_x;
        delete[] perm_y;
        delete[] perm_z;
    }

    [[nodiscard]] double noise(const point3& p) const
    {
        // Fractional portion of coordinate
        double u = p.x() - floor(p.x());
        double v = p.y() - floor(p.y());
        double w = p.z() - floor(p.z());
        // Lattice coordinate
        auto i = static_cast<int>(floor(p.x()));
        auto j = static_cast<int>(floor(p.y()));
        auto k = static_cast<int>(floor(p.z()));
        // Lattice offset vectors (achieved via dot product with "weight vector" which is a function of the fractional portion)
        vec3 c[2][2][2];

        for (int di = 0; di < 2; di++)
        {
            for (int dj = 0; dj < 2; dj++)
            {
                for (int dk = 0; dk < 2; dk++)
                {
                    // Sample from array of random unit vec3s where {x,y,z} in [-1,1]
                    // Index is a hash produced from 3 values in [0,255]
                    // The hash is a function of position in the lattice grid
                    c[di][dj][dk] = ranvec[perm_x[(i+di) & 255] ^ perm_y[(j+dj) & 255] ^ perm_z[(k+dk) & 255]];
                }
            }
        }

        // Interpolate based on fractional position
        return perlin_interp(c, u, v, w);
    }

    [[nodiscard]] double turb(const point3& p, int depth=7) const
    {
        double accum = 0.0;
        point3 temp_p = p;
        double weight = 1.0;

        for (int i = 0; i < depth; i++)
        {
            accum += weight*noise(temp_p);
            weight *= 0.5;
            temp_p *= 2;
        }

        return fabs(accum);
    }


private:
    static const int point_count = 256;
    vec3* ranvec;
    int* perm_x;
    int* perm_y;
    int* perm_z;

    static int* perlin_generate_perm()
    {
        auto p = new int[point_count];

        for (int i = 0; i < perlin::point_count; i++)
            p[i] = i;

        permute(p, point_count);

        return p;
    }

    static void permute(int* p, int n)
    {
        for (int i = n-1; i > 0; i--)
        {
            int target = random_int(0, i);
            int tmp = p[i];
            p[i] = p[target];
            p[target] = tmp;
        }
    }

    static double trilinear_interp(double c[2][2][2], double u, double v, double w)
    {
        double accum = 0.0;

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                for (int k = 0; k < 2; k++)
                {
                    // Lerp between {u,v,w} and {(1-u),(1-v),(1-w)} (in [0,1] and [1,0] respectively) based on {i,j,k}
                    // Multiply their product by the hashed real stored in c
                    // Sum over all 8 permutations of integers {i,j,k} in [0,1]
                    accum += (i*u + (1-i)*(1-u)) * (j*v + (1-j)*(1-v)) * (k*w + (1-k)*(1-w)) * c[i][j][k];
                }
            }
        }

        return accum;
    }

    static double perlin_interp(vec3 c[2][2][2], double u, double v, double w)
    {
        // Cubic hermite to reduce mach banding in noise
        double uu = u*u * (3 - 2*u);
        double vv = v*v * (3 - 2*v);
        double ww = w*w * (3 - 2*w);
        double accum = 0.0;

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                for (int k = 0; k < 2; k++)
                {
                    // Lerp(i, uu, 1-uu) * Lerp(j, vv, 1-vv) * Lerp(k, ww, 1-ww) * dot(NoiseVector, WeightVector)
                    vec3 weight_v(u-i, v-j, w-k);
                    accum += (i*uu + (1-i)*(1-uu)) * (j*vv + (1-j)*(1-vv)) * (k*ww + (1-k)*(1-ww)) * dot(c[i][j][k], weight_v);
                }
            }
        }

        return accum;
    }
};

#endif //RAYTRACINGINONEWEEKEND_PERLIN_H
