#pragma once
#include "spdlog/spdlog.h"

static struct SpdlogInit {
    SpdlogInit() { spdlog::set_level(spdlog::level::debug); }
} spdlog_init;