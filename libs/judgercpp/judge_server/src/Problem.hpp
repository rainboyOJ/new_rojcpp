//对给定位置的题目进行侦测
#pragma once
#include <regex>

#include "define.hpp"

/**
 * @desc 路径,子id
 */
struct Problem {
    Problem() = default;
    Problem(Problem && p)
        : input_data{std::move(p.input_data)},
        output_data{std::move(p.output_data)}
    {}

    Problem& operator= (Problem && p) 
    {

        input_data = std::move(p.input_data);
        output_data = std::move(p.output_data);
        return *this;
    }

    explicit Problem(const fs::path& p_path){
        //auto p_path = fs::path(path);
        if(not fs::exists(p_path) )
            throw std::runtime_error(p_path.string() + " 不存在!");

        auto data_path = p_path / "data"; //检查目录下的data文件夹
        if(not fs::exists(data_path) )
            throw std::runtime_error(data_path.string() + " 不存在!");
        //对数据进行检查

        for ( const auto& e : fs::directory_iterator(data_path) ) {
            auto filename = fs::path(e).filename();
            std::cmatch cm;
            if( std::regex_match(filename.c_str(),cm,input_regex ) ){
                input_data.emplace_back(stoi(cm[1].str()),e.path());
            }
            else if(  std::regex_match(filename.c_str(),cm,output_regex ) ){
                output_data.emplace_back(stoi(cm[1].str()),e.path());
            }
        }
        if( input_data.size() != output_data.size() )
            throw std::runtime_error(p_path.string() + " 输入输出数据个数不匹配");

        sort(input_data.begin(),input_data.end());
        sort(output_data.begin(),output_data.end());
    }
    explicit Problem(const std::string_view path,const std::string_view pid)
        : Problem(fs::path(path) / pid)
    {};

    std::vector<std::pair<int,std::string>> input_data; //数据
    std::vector<std::pair<int,std::string>> output_data; //数据
    const std::regex input_regex{"[A-Za-z_]+(\\d+)\\.in"};
    const std::regex output_regex{"[A-Za-z_]+(\\d+)\\.(out|ans)"};
};
