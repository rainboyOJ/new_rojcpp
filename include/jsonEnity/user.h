
/**
 * 定义给user使用
 */


#pragma once
#include <exception>
#include <string>

#include "serializable.hpp"
#include "appException.hpp"

using namespace cppjson;
/**
 * 注册时使用
 */
struct userRegistJson {
    std::string username;
    std::string nickname;
    std::string password;
    std::string school;
    std::string email;

    config get_config()const 
    {
        config config=Serializable::get_config(this);
        config.update({
            {"username", username},
            {"nickname", nickname},
            {"password", password},
            {"school",   school},
            {"email",    email}
        });
        return config;
    }
};

/**
 * 登录时使用
 */
struct userLoginJson {
    std::string username;
    std::string password;

    config get_config() const
    {
        config config=Serializable::get_config(this);
        config.update({
            {"username", username},
            {"password", password},
        });
        return config;
    }
};


/**
 * 创建一个用户
 * username
 * nickname
 * password
 * school
 * email
 * role
 * email_verified_at
 * status
 * remember_token
 */
/*
struct sqlUserTable {
    std::string username;
    std::string nickname;
    std::string password;
    std::string school;
    std::string email;
    std::string role{"student"};
    //std::string email_verified_at;
    int status{0};
    //std::string remember_token;

    sqlUserTable(const userRegistJson & urj )
        :username{urj.username},
        nickname{urj.nickname},
        password{urj.password},
        email{urj.email}
    {}

    //创建
    void Insert(SQLConnection * conn){
        isValid(conn);
        std::string error;
        std::ostringstream oss;
        oss << "insert into users (`username`,`nickname`,`password`,`school`,`email`,`role`,`status`) values (";
        oss << '\"' << username <<  "\",";
        oss << '\"' << nickname <<  "\",";
        oss << '\"' << password <<  "\",";
        oss << '\"' << school <<  "\",";
        oss << '\"' << email <<  "\",";
        oss << '\"' << role <<  "\",";
        oss  << status ; 
        oss << ")";
        std::cout << oss.str() << std::endl;
        auto ret = conn->infoQuery(oss.str(), error);
        //std::cout << "checkQuery ret " << ret << std::endl;
        if(error.length() ){
            std::cout << error << std::endl;
            throw sqlError(error);
        }
    }
    void Delete(); //删除
    void Update();

    //查询
    //@retval 返回id
    uint64_t Select(SQLConnection * conn){
        std::ostringstream oss;
        oss << "select id from  users where password = '";
        oss << password;
        oss << "' and username  = '";
        oss << username << "';";
        std::string error;
        auto res = conn->infoQuery(oss.str(), error);
        if(res.size() > 0 && res[0].length() > 0){
            return std::stoull(res[0]);
        }
        return 0;
    } 

    //验证
    void isValid(SQLConnection * conn){
        if (username.length() == 0)
            throw sqlError("username is empty!");
        if (email.length() == 0)
            throw sqlError("email is empty!");

        std::ostringstream oss;
        oss << "select count(*) from  users where email = '";
        oss << email;
        oss << "' or username  = '";
        oss << username << "';";
        std::string error;
        auto res = conn->infoQuery(oss.str(), error);
        std::cout << "===isValid" << std::endl;
        if (res[0] != "0")
            throw sqlError("email or username had exited");
    }
};
*/

