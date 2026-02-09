


#ifndef INCLUDED_TASK1B_RAYTRACER
#define INCLUDED_TASK1B_RAYTRACER

#pragma once

#include <cstdint>

#include <framework/image.h>
#include <framework/rgb32f.h>

#include "task1.h"


class RayTracer
{
public:
	image2D<float3> render(int res_x, int res_y, const Scene& scene, const Camera& camera, const Pointlight* lights, std::size_t num_lights, const float3& background_color, int max_bounces, int num_threads, int ssaa_samples_per_pixel, float kernel_standard_deviation);
	image2D<std::uint32_t> tonemap(const image2D<float3>& framebuffer) const;
};

#endif  // INCLUDED_TASK1B_RAYTRACER
