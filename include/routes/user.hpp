/**
 * @file
 * @brief 有关用户的相关路由
 */

#include "http_route.h"
#include "http_request.h"
#include "http_response.h"

#include "curd/user.h"


namespace rojcpp {
    

//TODO check is logined
struct is_login {

    bool before(request& req,response & res) {

        return true;
    }

};

struct USR_API {
    /**
     * method: POST
     * url: /usr/login
     * 接收的json {user}
     * 登录
     */
    static void user_login(request& req,response & res) {

        bool ok = false;
        auto user = CURD::UserTable::findByNameAndPasswd(ok, "root9","root9");

        //得到名字
        try {
            auto name = cppdb::get<"NickName">(user);
            LOG_INFO << "name " << name;
        }
        catch(...) {
            LOG_ERROR << "error";

        }

        res.set_status_and_content<status_type::ok, content_type::json>("{msg:\"ok\",code:1}");
    }

    /**
     * method: POST
     * url: /usr/register
     * 接收的json {user}
     * 注册
     */
    static void user_register(request& req, response & res) {

        // 得到user data json
    }


    //注册路由
    template<typename Server>
    static void regist_route() {
        using namespace std::literals::string_view_literals;
        Server::m_http_route.template register_handler<POST>("/usr/login"sv,user_login);
    }

};


} // end namespace rojcpp
