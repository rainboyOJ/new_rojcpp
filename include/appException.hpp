/**
 * @desc 异常
 */
#pragma once

#include <exception>

/**
 * @brief: sql执行时发生错误
 */
class sqlError:public std::exception {
public:
    explicit sqlError(std::string_view msg) :msg{msg} {}
    virtual ~sqlError() throw() {}
    virtual const char * what() const throw() { return msg.c_str(); }
private:
    std::string msg;
};
