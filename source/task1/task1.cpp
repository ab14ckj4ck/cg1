
#include "task1.h"

#include "Scene.h"
#include <limits>

constexpr float epsilon = 0.001f;

bool intersectRaySphere(const float3& p, const float3& d, const float3& c, float r, float& t) {
    // TODO: implement intersection test between a ray and a sphere.
    // If the ray given by start point p and direction d intersects the sphere
    // (given by its center c and radius r), assign the ray parameter
    // that corresponds to the first point of intersection to t and return true.
    // Otherwise, return false.
    //

    using namespace math;

    const float A = dot(d, d);
    const float B = 2.f * dot(p - c, d);
    const float C = dot(p - c, p - c) - pow(r, 2);

    const float discriminant = B * B - 4 * A * C;

    if (discriminant < 0)
        return false;

    const float t1 = (-B - sqrt(discriminant)) / (2 * A);
    const float t2 = (-B + sqrt(discriminant)) / (2 * A);

    if (t1 > 0)
        return (t = t1), true;
    if (t2 > 0)
        return (t = t2), true;

    return false;
}

float3 shade(const float3& p,
             const float3& d,
             const HitPoint& hit,
             const Scene& scene,
             const Pointlight* lights,
             std::size_t num_lights) {
    // TODO: implement blinn-phong shading.
    // hit represents the surface point to be shaded.
    // hit.position, hit.normal, and hit.k_d and hit.k_s contain the position,
    // surface normal, and diffuse and specular reflection coefficients,
    // hit.m the specular power.
    // lights is a pointer to the first element of an array of num_lights
    // point light sources to consider.
    // Each light contains a member to give its position and color.
    // Return the shaded color.

    // To implement shadows, use scene.intersectsRay(p, d, t_min, t_max) to test
    // whether a ray given by start point p and direction d intersects any
    // object on the section between t_min and t_max.

    using namespace math;

    const float3 n = hit.normal;
    const float3 v = normalize(p - hit.position);

    float3 color_sum = {0.f, 0.f, 0.f};

    for (int i = 0; i < num_lights; ++i) {

        // for each light, getting c_d and c_s -> summing up -> superposition
        const float3 unorm_l = lights[i].position - hit.position;
        const float3 l = normalize(unorm_l);
        const float3 h = normalize(v + l);

        // checking for objects between hit and light source -> shadows
        const float3 shifted_origin = hit.position + epsilon * n;
        if (scene.intersectsRay(shifted_origin, l, epsilon, length(unorm_l)))
            continue;

        // getting angles theta and alpha
        const float cos_theta = dot(n, l);
        const float cos_alpha = dot(h, n);

        // getting diffuse and specular light reflections
        const float3 c_d = hit.k_d * max(cos_theta, 0.f);
        const float3 c_s = (cos_theta > 0.f) ? hit.k_s * pow(max(cos_alpha, 0.f), hit.m) : float3{0.f, 0.f, 0.f};

        // Physical: The delivered, and finally shown, color is that color of the light, that the material reflects.
        const float3 color = c_d + c_s;
        color_sum += {
            color.x * lights[i].color.x,
            color.y * lights[i].color.y,
            color.z * lights[i].color.z,
        };
    }

    return color_sum;
}

void render(image2D<float3>& framebuffer,
            int left,
            int top,
            int right,
            int bottom,
            const Scene& scene,
            const Camera& camera,
            const Pointlight* lights,
            std::size_t num_lights,
            const float3& background_color,
            int max_bounces,
            int ssaa_samples_per_pixel,
            float std_dev) {
    // TODO: implement raytracing, render the given portion of the framebuffer.
    // left, top, right, and bottom specify the part of the image to compute
    // (inclusive left, top and exclusive right and bottom).
    // camera.eye, camera.lookat, and camera.up specify the position and
    // orientation of the camera, camera.w_s the width of the image plane,
    // and camera.f the focal length to use.
    // Use scene.findClosestHit(p, d) to find the closest point where the ray
    // hits an object.
    // The method returns an std::optional<HitPoint>.
    // If an object is hit, call the function shade to compute the color value
    // of the HitPoint illuminated by the given array of lights.
    // If the ray does not hit an object, use background_color.

    // SSAA:
    // if ssaa_samples per pixel is larger than 1,
    // sample multiple subpixels for each pixel.
    // In addition, apply a Gaussian kernel with the mean as the (large) pixel,
    // and the standard deviation as given by std_dev

    // BONUS: extend your implementation to recursive ray tracing.
    // max_bounces specifies the maximum number of secondary rays to trace.

    using namespace math;

    auto getLength = [&](const float3 a) -> float { return sqrt(pow(a.x, 2) + pow(a.y, 2) + pow(a.z, 2)); };

    bool gaussian = true;

    // computing the camera coord's uvw (cam looking -w)
    const float3 w_nominator = camera.eye - camera.lookat;
    const float w_denominator = getLength(camera.eye - camera.lookat);
    const float3 w = {w_nominator.x / w_denominator, w_nominator.y / w_denominator, w_nominator.z / w_denominator};

    const float3 u_nom = cross(camera.up, w);
    const float u_denom = getLength(cross(camera.up, w));
    const float3 u = {u_nom.x / u_denom, u_nom.y / u_denom, u_nom.z / u_denom};

    const float3 v = cross(w, u);

    // #of pixels in the frambuffer
    const float rx = width(framebuffer);
    const float ry = height(framebuffer);
    const float ws = camera.w_s;
    const float hs = ws * ry / rx;

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {

            float3 color = background_color;

            // "normal" ray tracing
            if (ssaa_samples_per_pixel <= 1) {

                //getting the center of a pixel in the pixelraster (staying in uvw space)
                const float px = ((x + 0.5f) / rx - 0.5f) * ws;
                const float py = -(((y + 0.5f) / ry - 0.5f) * hs);

                // calculating the vector d with camera coord's and converting it into world space for findClosestHit
                // by multiplying the pixels coordinates with the basis vectors of the camera system.
                float3 d = normalize(px * u + py * v - camera.f * w);

                if (auto hit = scene.findClosestHit(camera.eye, d)) {
                    color = shade(camera.eye, d, *hit, scene, lights, num_lights);
                }
            }
            // SSAA - with gaussian kernel (gaussian) and with mean (!gaussian)
            else {
                int n = sqrt(ssaa_samples_per_pixel);

                // setting up sums for gaussian:
                //      w_c_sum => weighted_color_sum: Sum(Ci * wi)
                //      w_sum => weight_sum: Sum(wi)
                // mean:
                //      c_sum => color sum: Sum(Ci)
                float w_sum = 0.f;
                float3 w_c_sum = {0.f, 0.f, 0.f};
                float3 c_sum = {0.f, 0.f, 0.f};

                // breaking down the pixel to a smaller sub_pixel grid of n² parts
                for (int sy = 0; sy < n; ++sy) {
                    for (int sx = 0; sx < n; ++sx) {

                        // getting the x coordinate of the sub_pixel in the image grid (!sub_grid)
                        float sub_x = x + (sx + 0.5f) / n;
                        float sub_y = y + (sy + 0.5f) / n;

                        // calc gaussian distance and weight according to pdf
                        float dx = sub_x - (x + 0.5f);
                        float dy = sub_y - (y + 0.5f);
                        const float di = sqrt(dx * dx + dy * dy);
                        const float wi = std::exp(-(di * di) / (std_dev * std_dev));

                        // shifting the coordinates from image grid to camera coordinates to calc d again
                        const float sub_d_x = ((sub_x / rx) - 0.5f) * ws;
                        const float sub_d_y = -(((sub_y / ry) - 0.5f) * hs);

                        // calculating the vector d with camera coord's and converting it into world space for findClosestHit
                        // by multiplying the pixels coordinates with the basis vectors of the camera system.
                        float3 sub_d = normalize(sub_d_x * u + sub_d_y * v - camera.f * w);
                        float3 temp_color = background_color;

                        if (auto hit = scene.findClosestHit(camera.eye, sub_d)) {
                            temp_color = shade(camera.eye, sub_d, *hit, scene, lights, num_lights);
                        }

                        if (!gaussian) {
                            c_sum += temp_color;
                        } else {
                            w_sum += wi;
                            w_c_sum += temp_color * wi;
                        }
                    }
                }

                if (!gaussian) {
                    // C = 1/n * Sum(Ci)
                    color = {
                        c_sum.x / ssaa_samples_per_pixel,
                        c_sum.y / ssaa_samples_per_pixel,
                        c_sum.z / ssaa_samples_per_pixel,
                    };
                } else {
                    // C = Sum(Ci * wi) / Sum(wi)
                    color = {
                        w_c_sum.x / w_sum,
                        w_c_sum.y / w_sum,
                        w_c_sum.z / w_sum,
                    };
                }
            }

            framebuffer(x, y) = color;
        }
    }
}