#pragma once

#include "scene_pipeline.hxx"

namespace hmk4_models {
    class Cloud : public CloudModelBase {
        float pix_per_m_;

        public:
        Cloud(
            vec3 aabb_min = vec3(-16000, 1000, -16000), vec3 aabb_max = vec3(16000, 1400, 16000),
            float pix_per_m = 1. / 80., int seed1 = 11, int seed2 = 1145
        );
        void activate_cloud_sampler(std::shared_ptr<ShaderProgram> prog, int at) override;
    };
} // namespace hmk4_models