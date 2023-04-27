//读写锁

#pragma once
#include <mutex>
#include <condition_variable>

class RWLock {

    public:

        RWLock(): read_cnt{0},write_cnt{0}
        {}

        void lock_read(){
            std::unique_lock<std::mutex> lock(mtx);
            while( write_cnt > 0) cv.wait(lock);
            ++read_cnt;
        }
        void unlock_read(){
            std::unique_lock<std::mutex> lock(mtx);
            --read_cnt;
            if( read_cnt == 0) cv.notify_all();
        }
        void lock_write(){
            std::unique_lock<std::mutex> lock(mtx);
            while( write_cnt > 0 || read_cnt > 0) cv.wait(lock);
            ++write_cnt;

        }
        void unlock_write(){
            std::unique_lock<std::mutex> lock(mtx);
            --write_cnt;
            cv.notify_all();
        }

    private:
        int read_cnt;
        int write_cnt;
        std::mutex mtx;
        std::condition_variable cv;
};
