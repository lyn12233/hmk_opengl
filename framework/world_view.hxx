#pragma once

#include "buffer_objects.hxx"
#include "utils.hxx"
#include "widget.hxx"
#include "window.hxx"

#include <memory>

#include <glm/glm.hpp>

using glm::mat4;
using glm::vec3;

/*
    coord convention:
        model_coord->model_mat->world_coord->lookat_mat->camera_coord->prospective_mat
            -> projection_coord -> divide by w -> triming+depth_test -> uv
        all coords in WorldCamera is defined in world_coord
        from screen, y-axis points upward, theta(aka. pitch)=atan(y,sqrt(x**2+z**2))
*/

namespace mf {
    class CameraPerspective {
        public:
        CameraPerspective(
            float fovy = pi / 4, float width = 800, float height = 600, float znear = 1e-2,
            float zfar = 1e4
        );
        float fovy_;
        float width_;
        float height_;
        float znear_;
        float zfar_;

        mat4 get();

        inline void set_size(GLuint w, GLuint h) {
            width_  = w;
            height_ = h;
        }

        constexpr static float pi = glm::pi<float>();

        void repr(int level = 0);
    };

    enum CAMERA_MOVE_T {
        CAMERA_MOVE_UP,
        CAMERA_MOVE_DOWN,
        CAMERA_MOVE_RIGHT,
        CAMERA_MOVE_LEFT,
        CAMERA_MOVE_FORWARD,
        CAMERA_MOVE_BACKWARD,
        CAMERA_ZOOM_FORWARD,
        CAMERA_ZOOM_BACKWARD,
        CAMERA_SPIN_UP,
        CAMERA_SPIN_DOWN,
        CAMERA_SPIN_RIGHT,
        CAMERA_SPIN_LEFT,
    };

    class WorldCamera {
        public:
        WorldCamera(
            vec3 coord_pos = vec3(0, 0, 0), vec3 viewpoint = vec3(10, 0, 0), float move_speed = 10,
            float spin_speed = 10, float zoom_speed_exp = 6, float zoom_speed_lin = 10,
            CameraPerspective perspective = CameraPerspective()
        );
        ~WorldCamera();

        // position
        vec3  coord_pos_; // also center, position of camera coordinate
        vec3  viewpoint_; // also eye, where camera looks to camera coord center
        float dist_, theta_ /*pitch angle*/, phi_ /*orient/yaw angle*/;
        // prospective
        CameraPerspective perspective_;

        enum camera_zoom_mode_t {
            CAMERA_ZOOM_LINEAR,
            CAMERA_ZOOM_EXP,
        };
        camera_zoom_mode_t camera_zoom_mode_;

        // speed attributes
        float x_speed_, y_speed_, z_speed_;
        float zoom_speed_lin_, zoom_speed_exp_, pitch_speed_, yaw_speed_;

        // const
        constexpr static float pi        = glm::pi<float>();
        constexpr static float min_theta = 1e-4 / 180 * pi;
        constexpr static float max_theta = (180 - 1e-4) / 180 * pi;
        constexpr static float min_dist  = 1e-4;
        constexpr static float max_dist  = 1e4;

        void validate();

        // move mechanisms
        // set viewpoint/angles accordingly
        void update_viewpoint();
        void update_angles();

        // move camera
        void camera_move(CAMERA_MOVE_T dir, float dt);

        // gen world to viewpoint matrix
        mat4        lookat();
        inline auto world2view() { return lookat(); }

        // gen projection matrix
        mat4        perspective();
        inline auto view2clip() { return perspective(); }

        // gen world to projection matrix
        mat4 world2clip() { return perspective() * lookat(); }

        void repr(int level = 0);
    };

    class WorldViewBase : public WidgetBase {
        public:
        WorldViewBase();
        ~WorldViewBase();

        void event_at(EVENT evt, Pos at, EVENT_PARM parameter) override;

        void on_resize(Rect rect);
        void process_key_instant(int KEY_CODE);
        void process_key();

        void repr(int level = 0) override;

        float t_prev;

        WorldCamera camera;
    };
} // namespace mf