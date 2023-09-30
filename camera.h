//
// Created by user on 9/10/2023.
//

#ifndef RAYTRACINGINONEWEEKEND_CAMERA_H
#define RAYTRACINGINONEWEEKEND_CAMERA_H

#include "rtweekend.h"
#include "color.h"
#include "hittable.h"
#include "material.h"

class camera
{
public:
    /* Public Camera Parameters Here */
    double aspect_ratio = 1.0; // Ratio of image width over height
    int image_width = 100; // Rendered image width in pixel count
    int samples_per_pixel = 10; // Count of random samples for each pixel
    int max_depth = 10; // Maximum number of ray bounces into scene
    color background; // Scene background color

    double vfov = 90; // Vertical view angle (field of view)
    point3 lookfrom = point3(0,0,-1); // Point camera is looking from
    point3 lookat = point3(0,0,0); // Point camera is looking at
    vec3 vup = vec3(0,1,0); // Camera-relative "up" direction

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10; // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable& world, std::ofstream& img)
    {
        initialize();

        size_t n_comp = 3;
        size_t buf_sz = image_width * image_height * n_comp;
        int* img_buf = new int[buf_sz];
        img << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        std::atomic<int> completed_scanlines;

        auto start = std::chrono::system_clock::now();
        auto cstart = std::chrono::system_clock::to_time_t(start);
        std::clog << "Started pathtrace at " << std::ctime(&cstart) << std::endl;

        #pragma omp parallel for default(none) shared(completed_scanlines, std::clog, img_buf, world, n_comp)
        for (int j = 0; j < image_height; ++j)
        {
            for (int i = 0; i < image_width; ++i)
            {
                color pixel_color(0,0,0);
                for (int sample = 0; sample < samples_per_pixel; ++sample)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }

                size_t idx = ((j * image_width) + i) * n_comp;
                write_color_mt(img_buf, idx, pixel_color, samples_per_pixel);
            }

            // Increment the atomic counter
            completed_scanlines++;

            #pragma omp critical
            {
                std::clog << "\rScanlines completed: " << completed_scanlines << "/" << image_height << std::flush;
            }
        }

        auto end = std::chrono::system_clock::now();
        auto cend = std::chrono::system_clock::to_time_t(end);
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::clog << "\n\nFinished pathtrace at " << std::ctime(&cend) << "Elapsed time: " << elapsed_seconds.count() << "s" << std::endl;

        for (size_t i = 0; i < buf_sz; i += n_comp)
        {
            img << img_buf[i] << ' ' << img_buf[i+1] << ' ' << img_buf[i+2] << '\n';
        }

        delete[] img_buf;
        std::clog << "\rDone.                 \n";
    }
private:
    /* Private Camera Variables Here */
    int image_height; // Rendered image height
    point3 center; // Camera center
    point3 pixel00_loc; // Location of pixel 0, 0
    vec3 pixel_delta_u; // Offset to pixel to the right
    vec3 pixel_delta_v; // Offset to pixel below
    vec3 u, v, w; // Camera frame basis vectors
    vec3 defocus_disk_u;  // Defocus disk horizontal radius
    vec3 defocus_disk_v;  // Defocus disk vertical radius

    void initialize()
    {
        // Calculate the image height, and ensure that it's at least 1.
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        center = lookfrom;

        // Determine viewport dimensions.
        double theta = degrees_to_radians(vfov);
        double h = tan(theta/2);
        double viewport_height = 2 * h * focus_dist;
        double viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        point3 viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        double defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    [[nodiscard]] ray get_ray(int i, int j) const
    {
        // Get a randomly-sampled camera ray for the pixel at location i,j, originating from
        // the camera defocus disk.

        point3 pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
        point3 pixel_sample = pixel_center + pixel_sample_square();

        point3 ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        vec3 ray_direction = pixel_sample - ray_origin;
        double ray_time = random_double();

        return {ray_origin, ray_direction, ray_time};
    }

    [[nodiscard]] point3 defocus_disk_sample() const
    {
        // Returns a random point in the camera defocus disk.
        vec3 p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    [[nodiscard]] vec3 pixel_sample_square() const
    {
        // Returns a random point in the square surrounding a pixel at the origin.
        double px = -0.5 + random_double();
        double py = -0.5 + random_double();
        return (px * pixel_delta_u) + (py * pixel_delta_v);
    }

    [[nodiscard]] color ray_color(const ray& r, int depth, const hittable& world) const
    {
        hit_record rec;

        if (depth <= 0) // Recursive depth limit
        {
            // If we've exceeded the ray bounce limit, no more light is gathered.
            return {0, 0, 0};
        }

        if (!world.hit(r, interval(epsilon, infinity), rec))
        {
            return background;
        }

        ray scattered;
        color attenuation;
        color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

        if (!rec.mat->scatter(r, rec, attenuation, scattered))
        {
            return color_from_emission;
        }

        color color_from_scatter = attenuation * ray_color(scattered, depth-1, world);

        return color_from_emission + color_from_scatter;
    }
};

#endif //RAYTRACINGINONEWEEKEND_CAMERA_H
