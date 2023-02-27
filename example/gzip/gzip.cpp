#include <iostream>
#include "tools/gzip.hpp"

int main() {
    std::string orgin_string = "hello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello worldhello world";
    std::string out_string;
    bool ret = rojcpp::gzip_codec::compress(out_string, out_string);
    if( ret ) {
        std::cout << orgin_string<< "\n";
        std::cout << "out_string length : " << out_string.length()<< "\n";

        std::string uncompress_str;
        bool ret2 = rojcpp::gzip_codec::uncompress(out_string, uncompress_str);
        if( ret2 ) {
            std::cout << "uncompress_str: length " << uncompress_str.length() << "\n";
            std::cout << uncompress_str << "\n";
        }
        //std::cout << out_string << "\n";
    }
    else {
        std::cout << "compress failed" << "\n";
    }
    return 0;
}
