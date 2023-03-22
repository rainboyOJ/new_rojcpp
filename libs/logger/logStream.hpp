#pragma once

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <string>
#include <concepts>

namespace LOGGER {
    
constexpr const int kSmallBuffer = 4096; // 4 kb
constexpr const int kLargeBuffer = 4096*1024;// 4mb

template<std::size_t SIZE>
class logBuffer {
    
    public:
        logBuffer() : m_cur(m_data)
        {}
        // ~logBuffe() = default;

        char * current() { return m_cur;}

        std::size_t avail() const { return static_cast<std::size_t>(end()-m_cur);}

        std::size_t length() const {
            return static_cast<std::size_t>(m_cur-m_data);
        }

        void add(std::size_t len) { m_cur += len;}

        void bzero() { ::bzero(m_data, sizeof(m_data));}

        void reset() { m_cur = m_data;}

        const char * data() const { return m_data;}


        //core function
        void append(const char * src,std::size_t len) {
            if( avail() > len ) {
                mempcpy(m_cur, src, len);
                m_cur+=len;
            }
        }


    private:
        const char * end() const {
            return m_data + sizeof(m_data);
        }
        char m_data[SIZE]; // buff memory
        char * m_cur;      // current pointer to no used memory
};


//作用: 将对应的类型的数据存到buffer里,通过operator<<
class logStream {

    public:
        logStream() = default;
        logStream(const logStream &) = delete; // nocopyable

        //interger
        template<std::integral T>
        logStream& operator<<(T num) {
            formatInter(num);
            return *this;
        }

        logStream& operator<< (float v) {
            *this << static_cast<double>(v);
            return *this;
        }
        logStream& operator<< (double v) {
            if( m_buf.avail() >= kMaxNumericSize ) {
                int len = snprintf(m_buf.current(), kMaxNumericSize, "%.12g",v);
                m_buf.add(len);
            }
            return *this;
        }

        logStream& operator<<(char c) {
            m_buf.append(&c, 1);
            return  *this;
        }

        logStream& operator<<(const char * str) {
            if( str ) {
                m_buf.append(str, strlen(str));
            }
            else {
                m_buf.append("(null)",6);
            }
            return *this;
        }

        logStream& operator<<(const void * data) {
            *this << static_cast<const char *>(data);
            return *this;
        }

        logStream& operator<<(const std::string & str) {
            m_buf.append(str.c_str(),str.length() );
            return *this;
        }

        logStream& operator<<(const std::string_view str_view) {
            m_buf.append(str_view.data(),str_view.length() );
            return *this;
        }

        logBuffer<kSmallBuffer> & get_buff() {
            return  m_buf;
        }

    private:

        static constexpr int kMaxNumericSize = 48;

        static constexpr const char digits[] = 
            {'9', '8', '7', '6', '5', '4', '3', '2', '1', '0',
                '1', '2', '3', '4', '5', '6', '7', '8', '9'};

        template<std::integral T>
        void formatInter(T num) {
            if(m_buf.avail() >= kMaxNumericSize ) {
                char * start = m_buf.current();
                char * cur = start;
                bool negative = (num < 0);

                const char * zero = digits+9;
                //末尾取值加入,最后反转
                do {
                    int idx = static_cast<int>(num % 10);
                    num /= 10;
                    *(cur++) = zero[idx];
                }while(num != 0);

                if( negative )
                    *(cur++) = '-';
                *cur='\0';

                std::reverse(start,cur);
                m_buf.add(static_cast<int>(cur-start));
            }

        }

        logBuffer<kSmallBuffer> m_buf;

};

} // end namespace LOGGER
