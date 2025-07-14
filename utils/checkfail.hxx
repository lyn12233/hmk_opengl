#pragma once

#include <iostream>

#include <spdlog/spdlog.h>

#ifndef __gl_h_
    #include <glad/glad.h>
#endif

#define _CHECKFAIL_CASING(x)                                                                       \
    case x:                                                                                        \
        spdlog::error("{}", #x);                                                                   \
        break;

inline void CheckFail(const char *file, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        spdlog::error("common error occurred: {} at {}:{}", err, file, line);
        switch (err) {
            _CHECKFAIL_CASING(GL_INVALID_ENUM)
            _CHECKFAIL_CASING(GL_INVALID_FRAMEBUFFER_OPERATION)
            _CHECKFAIL_CASING(GL_INVALID_OPERATION)
            _CHECKFAIL_CASING(GL_INVALID_INDEX)
            _CHECKFAIL_CASING(GL_INVALID_VALUE)
        }
        exit(-1);
    }
}
#ifdef _DEBUG
    #define MY_CHECK_FAIL CheckFail(__FILE__, __LINE__);
#else
    #define MY_CHECK_FAIL
#endif
#undef _CHECKFAIL_CASING