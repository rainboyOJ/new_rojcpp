/**
 * 用户定义
 */
#pragma once
#include <filesystem>
#include <string_view>
#include <string>

namespace fs = std::filesystem;

struct __CONFIG {
    //基础题目路径
    static constexpr std::string_view BASE_PROBLEM_PATH = "/home/rainboy/mycode/RainboyOJ/problems/problems";
    static constexpr std::string_view BASE_WORK_PATH= "/tmp";

    //judger 的位置
    static constexpr std::string_view judger_bin = "/usr/bin/judger_core";
    static constexpr std::size_t memory_base = 16*1024*1024; // 16mb
};
