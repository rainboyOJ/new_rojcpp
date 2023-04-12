//用户的相关操作
#pragma once

#include "curd.h"
#include "logger/logger.h"

namespace CURD {

/**
 * table desc
 *
 * username| nickname| password| school| email| role{"student"}|
 */
using namespace std::literals::string_view_literals;
struct UserTable {

    using simpleUserRow = cppdb::row_type<
        cppdb::column<"UserId", unsigned long long>,
        cppdb::column<"NickName", std::string>,
        cppdb::column<"Email", std::string>
        >;

    /**
     * @desc 创建一个用户
     */
    static void add(
            std::string_view username,
            std::string_view nickname,
            std::string_view password,
            std::string_view email,
            std::string_view school = ""sv,
            std::string_view role = "student"sv
            ) 
    {

        //INSERT INTO rojcpp.users (created_at,updated_at,deleted_at,username,nickname,password,school,email,`role`,email_verified_at,status,remember_token) VALUES
        cppdb::query<"INSERT INTO users (username,nickname,password,school,email,role) VALUES ('?','?','?','?','?','?');", void> q;
        q << username << nickname << password << email << school << role << cppdb::exec;
    }

    //根据用记名和密码查找用户
    static simpleUserRow findByNameAndPasswd(
            bool &ok,
            std::string_view username,
            std::string_view password
            ){


        ok = 0;
        cppdb::query<"select id,nickname,email from users where username = '?' and password = '?';",
        simpleUserRow> q;

        try {
            auto row = q << username << password << cppdb::exec;
            ok = 1;
            //return std::make_tuple<bool,simpleUserRow>(true,row);
            return row;
        }
        catch(cppdb::cppdb_zero_row & e) {
            //return std::make_tuple<bool,simpleUserRow>(false,{});
            return {};
        }
        catch(cppdb::cppdb_zero_filed & e) {
            //return std::make_tuple<bool,simpleUserRow>(false,{});
            return {};
        }
    }

    static void del() {
    }

    static void update() {
    }

};

} // namespace CURD
