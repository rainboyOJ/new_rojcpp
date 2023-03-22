#ifndef  __ASYNC_EXCEPTION__
#define __ASYNC_EXCEPTION__
// #include <exception>


namespace tinyasync {

    class AsyncRecvTimeOutError : public std::exception {

        virtual const char * what()  const noexcept override {
            return "AsyncRecvTimeOutError";
        }

    };
}

#endif
