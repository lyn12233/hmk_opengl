#pragma once

#include <glm/glm.hpp>

// for FrameBuffer
#define DEFAULT_FBO_WIDTH 800
#define DEFAULT_FBO_HEIGHT 600

// for GlfwInst
#define DEFAULT_GL_VERSION "4.0"

// for DrawableFrame
#define DEFAULT_CLEAR_COLOR {0, 0, 0, 0}
#define DEFAULT_SCREEN_SCALING 1.5
#define DEFAULT_GBUFFER_SCALING 1.5

// for BoxSizer
#define DEFAULT_SIZER_BORDER 10

// for AsciiTex
#define DEFAULT_TTF_PATH "C:/Windows/Fonts/cour.ttf"

// for TextCtrl
#define DEFAULT_TC_WIDTH 100
#define DEFAULT_TC_HEIGHT 24
#define DEFAULT_TC_FONTSIZE 24
#define DEFAULT_TC_BKGD {0, 0, 0, 1}
#define DEFAULT_TC_FRGD {1, 1, 1, 1}
#define DEFAULT_TC_BKGD_INV {1, 1, 1, 1}
#define DEFAULT_TC_FRGD_INV {0, 0, 0, 1}

// button
#define DEFAULT_BTN_BKGD DEFAULT_TC_BKGD_INV
#define DEFAULT_BTN_FRGD DEFAULT_TC_FRGD_INV
#define DEFAULT_BTN_BKGD_INV DEFAULT_TC_BKGD
#define DEFAULT_BTN_FRGD_INV DEFAULT_TC_FRGD

// for Window
#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600

// WorldCamera
#define DEFAULT_CAMERA_LOOKFROM {50, 0, 0}
#define DEFAULT_CAMERA_LOOKTO {0, 0, 0}
#define DEFAULT_CAMERA_MOVE_SPEED 10
#define DEFAULT_CAMERA_SPIN_SPEED 2

// for parameterdict
#define DEFAULT_PARM_PER_COL 14