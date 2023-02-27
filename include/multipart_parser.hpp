#fndef _MULTIPART_PARSER_H_
#define _MULTIPART_PARSER_H_

#include <string>
#include <stdexcept>
#include <cstring>

namespace netcore {
    class multipart_parser {
    public:
        typedef void(*Callback)(const char *buffer, size_t start, size_t end, void *userData);

    public:
        //钩子函数
        Callback onHeaderField;     ///头的名
        Callback onHeaderValue;     ///头的值
        Callback onHeaderEnd;       ///头的结

        Callback onHeadersEnd;      ///所有头的结束

        Callback onPartBegin;       ///部分中的开始
        Callback onPartData;        ///部分中的数据
        Callback onPartEnd;         ///部分结束

        Callback onEnd;             ///全部结束
        void *userData; //这是一个指针

        multipart_parser() { //构造函数
            lookbehind = nullptr;
            resetCallbacks();
            reset();
        }

        multipart_parser(std::string&& boundary) {
            lookbehind = nullptr;
            resetCallbacks();
            set_boundary(std::move(boundary));
        }

        ~multipart_parser() {
            delete[] lookbehind;
        }

        void reset() {
            delete[] lookbehind;
            state           = PARSE_ERROR;
            boundary.clear();
            boundaryData    = boundary.c_str();
            boundarySize    = 0;
            lookbehind      = nullptr;
            lookbehindSize  = 0;
            flags           = 0;
            index           = 0;
            headerFieldMark = UNMARKED;
            headerValueMark = UNMARKED;
            partDataMark    = UNMARKED;
            errorReason     = "Parser uninitialized.";
        }

        /**
         * @brief 设置 boundary
         * \r\n----WebKitFormBoundaryX4zzs3sYStDhVBUX
         */
        void set_boundary(std::string&& boundary) {
            reset();
            this->boundary = std::move(boundary);
            boundaryData = this->boundary.c_str();
            boundarySize = this->boundary.size();
            indexBoundary();
            lookbehind = new char[boundarySize + 8];
            lookbehindSize = boundarySize + 8;
            state = START;
            errorReason = "No error.";
        }

        /**
         * 喂,食物是mulpart的数据
         * @brief 核心函数
         *
         * @agrument buffer 数据起始指针
         * @agrument len 处理的长度
         *
         * @return 处理的数据的buffer的哪个位置停止的
         */
        size_t feed(const char *buffer, size_t len) {
            if (state == PARSE_ERROR || len == 0) {
                return 0;
            }

            State state        = this->state;
            int flags          = this->flags;
            size_t prevIndex   = this->index;
            size_t index       = this->index;
            size_t boundaryEnd = this->boundarySize - 1;
            //拷贝 对应的状态值

            size_t i;
            char c, cl;

            //核心状态机
            for (i = 0; i < len; i++) {
                c = buffer[i];

                switch (state) {
                case PARSE_ERROR:
                    return i;
                case START:
                    index = 0;      //起始时 index 为 0
                    state = START_BOUNDARY;
                case START_BOUNDARY: //这里的index表示处理的字符的位置
                    if (index == boundarySize - 2) { //去除2 表示读取时 其实不是从\r\n开始读取的
                        if (c != CR) {
                            setError("Malformed. Expected CR after boundary.");
                            return i;
                        }
                        index++;
                        break;
                    }
                    else if (index - 1 == boundarySize - 2) { //应该写成 index == boundarySize-1 更容易理解
                        if (c != LF) {
                            setError("Malformed. Expected LF after boundary CR.");
                            return i;
                        }
                        index = 0;
                        callback(onPartBegin);
                        state = HEADER_FIELD_START;
                        break;
                    }
                    if (c != boundary[index + 2]) {  //不是对应的boundary的值
                        setError("Malformed. Found different boundary data than the given one.");
                        return i;
                    }
                    index++; // 增加1
                    break;
                case HEADER_FIELD_START: // 信息头开始
                    state = HEADER_FIELD;
                    headerFieldMark = i; // 标明,header的开始位置
                    index = 0;      // 指示器置0
                case HEADER_FIELD:  // 进入信息头内部
                    if (c == CR) {
                        headerFieldMark = UNMARKED;
                        state = HEADERS_ALMOST_DONE;
                        break;
                    }

                    index++;
                    if (c == HYPHEN) { // - 不做处理
                        break; // 跳转出switch 处理下一个字符
                    }

                    if (c == COLON) { // : 
                        if (index == 1) { //只有 : 没有header_name
                            // empty header field
                            setError("Malformed first header name character.");
                            return i; // 结束
                        }
                        // 处理读取到的 header,例如 content-Type: text/plain
                        // buffer 整个buffer的指针,i :号所在位置, len buffer长度
                        // clear 清空,
                        dataCallback(onHeaderField, headerFieldMark, buffer, i, len, true);
                        state = HEADER_VALUE_START;
                        break;
                    }

                    cl = lower(c);
                    if (cl < 'a' || cl > 'z') {
                        setError("Malformed header name."); // 名字必须是字母
                        return i; // 出错
                    }
                    break;
                case HEADER_VALUE_START:
                    if (c == SPACE) { //过滤空格
                        break;
                    }

                    headerValueMark = i;
                    state = HEADER_VALUE;
                case HEADER_VALUE:
                    if (c == CR) {
                        dataCallback(onHeaderValue, headerValueMark, buffer, i, len, true, true);
                        callback(onHeaderEnd);
                        state = HEADER_VALUE_ALMOST_DONE;
                    }
                    break;
                case HEADER_VALUE_ALMOST_DONE:
                    if (c != LF) {
                        setError("Malformed header value: LF expected after CR");
                        return i;
                    }

                    state = HEADER_FIELD_START;
                    break;
                case HEADERS_ALMOST_DONE:
                    if (c != LF) {
                        setError("Malformed header ending: LF expected after CR");
                        return i;
                    }

                    callback(onHeadersEnd);
                    state = PART_DATA_START;
                    break;
                case PART_DATA_START:
                    state = PART_DATA;
                    partDataMark = i;
                case PART_DATA:
                    processPartData(prevIndex, index, buffer, len, boundaryEnd, i, c, state, flags);
                    break;
                case END:
                    break;
                default:
                    return i;
                }
            }

            //整个for结束后,再来一遍处理
            dataCallback(onHeaderField, headerFieldMark, buffer, i, len, false);
            dataCallback(onHeaderValue, headerValueMark, buffer, i, len, false);
            dataCallback(onPartData, partDataMark, buffer, i, len, false);

            //保存状态
            this->index = index;
            this->state = state;
            this->flags = flags;

            return len;
        }

        bool succeeded() const {
            return state == END;
        }

        bool has_error() const {
            return state == PARSE_ERROR;
        }

        bool stopped() const {
            return state == PARSE_ERROR || state == END;
        }

        const char *get_error_message() const {
            return errorReason;
        }

    private:
        static const char CR = 13;
        static const char LF = 10;
        static const char SPACE = 32;
        static const char HYPHEN = 45; // -
        static const char COLON = 58;  // :
        static const size_t UNMARKED = (size_t)-1;

        enum State {
            PARSE_ERROR,
            START,
            START_BOUNDARY,
            HEADER_FIELD_START,
            HEADER_FIELD,
            HEADER_VALUE_START,
            HEADER_VALUE,
            HEADER_VALUE_ALMOST_DONE,
            HEADERS_ALMOST_DONE,
            PART_DATA_START,
            PART_DATA,
            PART_END,
            END
        };

        enum Flags {
            PART_BOUNDARY = 1,
            LAST_BOUNDARY = 2
        };

        void resetCallbacks() {
            onPartBegin   = nullptr;
            onHeaderField = nullptr;
            onHeaderValue = nullptr;
            onHeaderEnd   = nullptr;
            onHeadersEnd  = nullptr;
            onPartData    = nullptr;
            onPartEnd     = nullptr;
            onEnd         = nullptr;
            userData      = nullptr;
        }

        /**
         * @brief 根据BoudaryData(boundary的值),把boundaryIndex数组的对应位置设置为true
         * 建立索引
         */
        void indexBoundary() { 
            const char *current;
            const char *end = boundaryData + boundarySize;

            memset(boundaryIndex, 0, sizeof(boundaryIndex));

            for (current = boundaryData; current < end; current++) {
                boundaryIndex[(unsigned char)*current] = true;
            }
        }

        /**
         * callback 调用cb函数
         * buffer 处理的数据指针
         * start 处理部分的开始位置
         * end 处理部分的结束位置
         * allowEmpty 是否允许处理部分为空
         */
        void callback(Callback cb, const char *buffer = nullptr, size_t start = UNMARKED,
            size_t end = UNMARKED, bool allowEmpty = false)
        {
            if (start != UNMARKED && start == end && !allowEmpty) {
                return;
            }
            if (cb != nullptr) {
                cb(buffer, start, end, userData);
            }
        }

        /**
         * dataCallback 调用数据处理的函数
         *  cb 数据处理的函数
         *  mark
         *  clear ?
         *  buffer
         *  i
         *  bufferLen
         *  clear 
         *      影响传递的参数的位置
         *      false -> start = mark,end = bufferLen
         *      true  -> start = mark, end = i
         *      
         */
        void dataCallback(Callback cb, size_t &mark, const char *buffer, size_t i, size_t bufferLen,
            bool clear, bool allowEmpty = false)
        {
            if (mark == UNMARKED) {
                return;
            }

            if (!clear) {
                callback(cb, buffer, mark, bufferLen, allowEmpty);
                mark = 0;
            }
            else {
                callback(cb, buffer, mark, i, allowEmpty);
                mark = UNMARKED; // clear == true 时, 改变原mark为UNMARKED
            }
        }

        char lower(char c) const {
            return c | 0x20;
        }

        inline bool isBoundaryChar(char c) const {
            return boundaryIndex[(unsigned char)c];
        }

        bool isHeaderFieldCharacter(char c) const {
            return (c >= 'a' && c <= 'z')
                || (c >= 'A' && c <= 'Z')
                || c == HYPHEN;
        }

        void setError(const char *message) {
            state = PARSE_ERROR;
            errorReason = message;
        }

        /**
         * @brief 解析 body 部分
         * @agrument prevIndex  
         * @agrument index      只有在header name里 和boundary 才会增加
         * @agrument buffer     处理的数据的指针
         * @agrument len        长度
         * @agrument boundaryEnd    boundary的长度???
         * @agrument i              处理到的i的位置
         * @agrument c              处理到的字符
         * @agrument state          状态
         * @agrument flag           ??
         */
        void processPartData(size_t &prevIndex, size_t &index, const char *buffer,
            size_t len, size_t boundaryEnd, size_t &i, char c, State &state, int &flags)
        {
            prevIndex = index;

            if (index == 0) {
                // boyer-moore derived algorithm to safely skip non-boundary data
                while (i + boundarySize <= len) {
                    if (isBoundaryChar(buffer[i + boundaryEnd])) {
                        break;
                    }

                    i += boundarySize;
                }
                if (i == len) {
                    return;
                }
                c = buffer[i];
            }

            if (index < boundarySize) {
                if (boundary[index] == c) {
                    if (index == 0) {
                        dataCallback(onPartData, partDataMark, buffer, i, len, true);
                    }
                    index++;
                }
                else {
                    index = 0;
                }
            }
            else if (index == boundarySize) {
                index++;
                if (c == CR) {
                    // CR = part boundary
                    flags |= PART_BOUNDARY;
                }
                else if (c == HYPHEN) {
                    // HYPHEN = end boundary
                    flags |= LAST_BOUNDARY;
                }
                else {
                    index = 0;
                }
            }
            else if (index - 1 == boundarySize) {
                if (flags & PART_BOUNDARY) {
                    index = 0;
                    if (c == LF) {
                        // unset the PART_BOUNDARY flag
                        flags &= ~PART_BOUNDARY;
                        callback(onPartEnd);
                        callback(onPartBegin);
                        state = HEADER_FIELD_START;
                        return;
                    }
                }
                else if (flags & LAST_BOUNDARY) {
                    if (c == HYPHEN) {
                        callback(onPartEnd);
                        callback(onEnd);
                        state = END;
                    }
                    else {
                        index = 0;
                    }
                }
                else {
                    index = 0;
                }
            }
            else if (index - 2 == boundarySize) {
                if (c == CR) {
                    index++;
                }
                else {
                    index = 0;
                }
            }
            else if (index - boundarySize == 3) {
                index = 0;
                if (c == LF) {
                    callback(onPartEnd);
                    callback(onEnd);
                    state = END;
                    return;
                }
            }

            if (index > 0) {
                // when matching a possible boundary, keep a lookbehind reference
                // in case it turns out to be a false lead
                if (index - 1 >= lookbehindSize) {
                    setError("Parser bug: index overflows lookbehind buffer. "
                        "Please send bug report with input file attached.");
                    throw std::out_of_range("index overflows lookbehind buffer");
                }
                else if (index - 1 < 0) {
                    setError("Parser bug: index underflows lookbehind buffer. "
                        "Please send bug report with input file attached.");
                    throw std::out_of_range("index underflows lookbehind buffer");
                }
                lookbehind[index - 1] = c;
            }
            else if (prevIndex > 0) {
                // if our boundary turned out to be rubbish, the captured lookbehind
                // belongs to partData
                callback(onPartData, lookbehind, 0, prevIndex);
                prevIndex = 0;
                partDataMark = i;

                // reconsider the current character even so it interrupted the sequence
                // it could be the beginning of a new sequence
                i--;
            }
        }

        std::string boundary;       //boundary本身
        const char* boundaryData;   //boundary数据指针
        size_t      boundarySize;   //boundary大小
        bool        boundaryIndex[256]; //boundary的标明器
        char*       lookbehind; // ?
        size_t      lookbehindSize;
        State       state;          //解析的状态
        int         flags;
        size_t      index;
        size_t      headerFieldMark; //在buffer_ heaer起始的位置
        size_t      headerValueMark; //headerValue 起始的位置
        size_t      partDataMark;    //part起始位置
        const char* errorReason;    //解析出现错误的原因
    };
}
#endif /* _MULTIPART_PARSER_H_ */
