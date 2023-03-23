#include <cstdarg>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <iostream>
#include <vector>
#include <string>

#define TINYASYNC_TRACE

#ifdef TINYASYNC_TRACE

#define TINYASYNC_CAT_(a, b) a##b
#define TINYASYNC_CAT(a, b) TINYASYNC_CAT_(a, b)
#define TINYASYNC_GUARD(...) log_prefix_guard TINYASYNC_CAT(log_prefix_guard_, __LINE__)(__VA_ARGS__)
#define TINYASYNC_LOG(...) \
    do                     \
    {                      \
        auto out = get_log_str(__VA_ARGS__);\
        printf("%s\n", out.c_str());\
    } while (0)

#define TINYASYNC_LOG_NNL(...) \
    do                     \
    {                      \
        auto out = get_log_str(__VA_ARGS__);\
        printf("%s", out.c_str());\
    } while (0)




    inline thread_local std::vector<std::string> log_prefix;

    inline std::string get_log_str(char const* fmt, ...)
    {
        std::string out;
        for (auto& p : log_prefix) {
            out.append(p);
        }

        int n = 1;
        {
            va_list args;
            va_start(args, fmt);
            n = vsnprintf(NULL, 0, fmt, args);
            va_end(args);
        }

        std::string buf;
        buf.resize(n); // actual size: n + 1

        va_list args;
        va_start(args, fmt);
        vsnprintf(buf.data(), n+1, fmt, args);
        va_end(args);

        out += buf;

        return out;
    }

    struct log_prefix_guard
    {
        std::size_t l;

        log_prefix_guard(char const* fmt, ...) : l(0)
        {

            char buf[1000];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buf, 1000, fmt, args);
            va_end(args);

            buf[1000 - 1] = 0;


            log_prefix.emplace_back(buf);
            l = log_prefix.size();
        }

        ~log_prefix_guard()
        {
            assert(l == log_prefix.size());
            log_prefix.pop_back();
        }
    };

#else

#define TINYASYNC_GUARD(...) \
    do                       \
    {                        \
    } while (0)
#define TINYASYNC_LOG(...) \
    do                     \
    {                      \
    } while (0)
#define TINYASYNC_LOG_NNL(...) \
    do                         \
    {                          \
    } while (0)

#endif // TINYASYNC_TRACE

void foo1() {
    TINYASYNC_LOG("in function %s",__func__);
}

void bar1() {
    // TINYASYNC_LOG("in function %s",__func__);
    TINYASYNC_GUARD("->GUARD, in function %s",__func__);
    TINYASYNC_LOG("in function %s",__func__);
    
}
void foo2() {
    TINYASYNC_GUARD("GUARD, in function %s",__func__);
    // TINYASYNC_LOG("in function %s",__func__);
    bar1();
}

//一个具有分支的dfs
void dfs(int dep) {
    if( dep == 5 || dep == 8) {
        TINYASYNC_LOG("approach the dfs boundary,and dep = %d",dep);
        return;
    }
    TINYASYNC_GUARD("in dfs, at dep = %d\n",dep);
    if( dep == 3) {
        dfs(6);
    }
    dfs(dep+1);
}

int main(){
    foo1();
    printf("\n\n\n");

    foo2();
    printf("\n\n\n");

    dfs(1);

    return 0;
}
