#include "world_view.hxx"
#include "utils.hxx"
#include "widget.hxx"
#include "window.hxx"

#include <memory>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

using glm::atan;
using glm::ceil;
using glm::cos;
using glm::floor;
using glm::length;
using glm::mat4;
using glm::sin;
using glm::sqrt;
using glm::vec3;

using mf::CameraPerspective;
using mf::WorldCamera;
using mf::WorldViewBase;

CameraPerspective::CameraPerspective(
    float fovy, float width, float height, float znear, float zfar
) :
    fovy_(fovy),
    width_(width), height_(height), znear_(znear), zfar_(zfar) {}

mat4 CameraPerspective::get() { return glm::perspective(fovy_, width_ / height_, znear_, zfar_); }

void CameraPerspective::repr(int level) {
    auto slog = std::string(' ', level);
    slog += "perspective(fovy={},width={},height={})";
    spdlog::info(slog, fovy_, width_, height_);
}

WorldCamera::WorldCamera(
    vec3 coord_pos, vec3 viewpoint, CameraPerspective perspective, float move_speed,
    float spin_speed, float zoom_speed_exp, float zoom_speed_lin, bool spin_at_viewpoint
) :                        // camera origin and target
    coord_pos_(coord_pos), //
    viewpoint_(viewpoint), //
    spin_at_viewpoint_(spin_at_viewpoint),
    // zoom mode
    camera_zoom_mode_(CAMERA_ZOOM_EXP),
    // speeds
    x_speed_(move_speed), y_speed_(move_speed), z_speed_(move_speed), //
    zoom_speed_lin_(zoom_speed_lin), zoom_speed_exp_(zoom_speed_exp), //
    pitch_speed_(spin_speed), yaw_speed_(spin_speed),
    // camera perspective
    perspective_(perspective) {
    update_angles();
    validate();
}
WorldCamera::~WorldCamera() {}

void WorldCamera::validate() {
    bool clipped = false;
    // theta range
    if (theta_ < min_theta) {
        theta_  = min_theta;
        clipped = true;
    } else if (theta_ > max_theta) {
        theta_  = max_theta;
        clipped = true;
    }
    // phi range
    if (phi_ < 0.) {
        auto N = ceil(-phi_ / (2 * pi));
        phi_ += N * (2 * pi);
        clipped = true;
    } else if (phi_ >= 2 * pi) {
        auto N = floor(phi_ / (2 * pi));
        phi_ -= N * (2 * pi);
        clipped = true;
    }
    // distance range
    if (dist_ < min_dist) {
        dist_   = min_dist;
        clipped = true;
    } else if (dist_ > max_dist) {
        dist_   = max_dist;
        clipped = true;
    }
    if (clipped) update_viewpoint();
}

void WorldCamera::update_viewpoint() {
    auto dir = vec3(sin(theta_) * sin(phi_), cos(theta_), sin(theta_) * cos(phi_));
    if (spin_at_viewpoint_) {
        coord_pos_ = viewpoint_ - dir * dist_;
    } else {
        viewpoint_ = coord_pos_ + dir * dist_;
    }
} // update_loc/>

void WorldCamera::update_angles() {
    vec3 r = viewpoint_ - coord_pos_;
    theta_ = atan(sqrt(r.x * r.x + r.z * r.z), r.y);
    phi_   = atan(r.x, r.z);
    dist_  = length(r);
}

void WorldCamera::camera_move(CAMERA_MOVE_T dir, float dt) {

    auto horizontal_spin = glm::mat3(cos(phi_), 0, -sin(phi_), 0, 1, 0, sin(phi_), 0, cos(-phi_));
    vec3 local_move(0.0f);

    switch (dir) {
        case CAMERA_MOVE_UP:
            local_move = vec3(0, 1, 0) * y_speed_ * dt;
            break;
        case CAMERA_MOVE_DOWN:
            local_move = vec3(0, -1, 0) * y_speed_ * dt;
            break;
        case CAMERA_MOVE_RIGHT:
            // spdlog::debug("right");
            local_move = vec3(1, 0, 0) * x_speed_ * dt;
            break;
        case CAMERA_MOVE_LEFT:
            local_move = vec3(-1, 0, 0) * x_speed_ * dt;
            break;
        case CAMERA_MOVE_FORWARD:
            local_move = vec3(0, 0, -1) * z_speed_ * dt;
            break;
        case CAMERA_MOVE_BACKWARD:
            local_move = vec3(0, 0, 1) * z_speed_ * dt;
            break;
        case CAMERA_SPIN_DOWN:
            theta_ += pitch_speed_ * dt * (spin_at_viewpoint_ ? -1 : 1);
            break;
        case CAMERA_SPIN_UP:
            theta_ -= pitch_speed_ * dt * (spin_at_viewpoint_ ? -1 : 1);
            break;
        case CAMERA_SPIN_RIGHT:
            phi_ += yaw_speed_ * dt * (spin_at_viewpoint_ ? -1 : 1);
            break;
        case CAMERA_SPIN_LEFT:
            phi_ -= yaw_speed_ * dt * (spin_at_viewpoint_ ? -1 : 1);
            break;
        case CAMERA_ZOOM_FORWARD:
            dt = -dt;
        case CAMERA_ZOOM_BACKWARD:
            if (camera_zoom_mode_ == CAMERA_ZOOM_EXP) {
                dist_ *= glm::exp2(zoom_speed_exp_ * dt);
            } else if (camera_zoom_mode_ == CAMERA_ZOOM_LINEAR) {
                dist_ += zoom_speed_lin_ * dt;
            }
    } // switch(dir)/>

    if (local_move != vec3(0.0f)) { // shift-like move
        local_move = horizontal_spin * local_move;
        coord_pos_ += local_move;
        viewpoint_ += local_move;
    } else { // spin-like move
        update_viewpoint();
    }
    validate(); // may be consuming
} // WorldCamera::camera_move/>

mat4 WorldCamera::lookat() { return glm::lookAt(viewpoint_, coord_pos_, vec3(0, 1, 0)); }
mat4 WorldCamera::perspective() { return perspective_.get(); }

void WorldCamera::repr(int level) {
    auto slog = std::string(' ', level);
    slog += "camera(from ({:.3f},{:.3f},{:.3f}) look at ({:.3f},{:.3f},{:.3f}), dist={:.3f})";
    spdlog::info(
        slog, viewpoint_.x, viewpoint_.y, viewpoint_.z, coord_pos_.x, coord_pos_.y, coord_pos_.z,
        dist_
    );
    perspective_.repr(level);
}

// WorldViewBase---

WorldViewBase::WorldViewBase() { type = mf::WORLD_VIEW; }
WorldViewBase::~WorldViewBase() {}

void WorldViewBase::event_at(EVENT evt, Pos at, EVENT_PARM parameter) {
    // spdlog::debug("WorldViewBase::event_at({})",(int)evt);
    if (evt == EVT_RESIZE) {
        spdlog::debug("WorldViewBase::event_at(resize)");
        on_resize(parameter.rect);
    } else if (evt == EVT_KEYBOARD && focus_) {
        auto key    = parameter.rect.x;
        auto action = parameter.rect.y;
        auto modes  = parameter.rect.w;
        if (action == GLFW_PRESS && !modes) {
            process_key_instant(key);
        }
    } else if (evt == EVT_SCROLL && focus_) { // zoom forward and backward
        camera.camera_move(CAMERA_ZOOM_FORWARD, parameter.vec2.y * 0.1);
        repr();
    } else if (evt == EVT_ROUTINE) {
        if (focus_) process_key();
        mark_dirty(false, false);
    } else if (evt == EVT_FOCUS) {
        spdlog::debug("WorldViewBase::event_at(focus)");
        focus_ = true;
        if (auto win = window_.lock()) {
            win->set_focus(shared_from_this());
        }
    } else if (evt == EVT_FOCUS_OUT) {
        focus_ = false;
    }
}

void WorldViewBase::on_resize(Rect rect) {
    spdlog::debug("WorldViewBase::on_resize({},{})", rect.w, rect.h);
    cur_rect = rect;
    // mark_dirty(false,false);// reduntant
    camera.perspective_.set_size(rect.w, rect.h);
}

void WorldViewBase::process_key_instant(int KEY_CODE) {}

void WorldViewBase::process_key() {
    dirty = true;
    // spdlog::debug("WorldView::process_key(window exists:{})",!window_.expired());
    float t_cur = glfwGetTime();
    float dt    = t_cur - t_prev;
    t_prev      = t_cur;

    if (auto winp = window_.lock()) { // get instant key state of the window

        auto win = winp->window();
        // handle up/down/right/left to move camera
        if (glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS) { // ↑
            // spdlog::debug("WorldViewBase::process_key(UP)");
            camera.camera_move(CAMERA_SPIN_UP, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS) { // ↓
            camera.camera_move(CAMERA_SPIN_DOWN, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS) { // →
            camera.camera_move(CAMERA_SPIN_RIGHT, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS) { // ←
            camera.camera_move(CAMERA_SPIN_LEFT, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) { // shift
            camera.camera_move(CAMERA_MOVE_UP, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) { // ctrl
            camera.camera_move(CAMERA_MOVE_DOWN, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) { // d
            camera.camera_move(CAMERA_MOVE_RIGHT, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) { // a
            camera.camera_move(CAMERA_MOVE_LEFT, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) { // w
            camera.camera_move(CAMERA_MOVE_FORWARD, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) { // s
            camera.camera_move(CAMERA_MOVE_BACKWARD, dt);
        }
        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) { // esc
            glfwSetWindowShouldClose(win, true);
        }
    }
}

void WorldViewBase::repr(int level) {
    WidgetBase::repr(level);
    camera.repr(level + 4);
}