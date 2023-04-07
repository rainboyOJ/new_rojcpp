/**
 * @file
 * @brief 有关用户的相关路由
 */

#include "http_route.h"
#include "http_request.h"
#include "http_response.h"


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

        res.set_status_and_content<status_type::ok, content_type::json>("{msg:\"ok\",code:1}");
    }


    //注册路由
    template<typename Server>
    static void regist_route() {
        using namespace std::literals::string_view_literals;
        Server::m_http_route.template register_handler<POST>("/usr/login"sv,user_login);
    }

};


} // end namespace rojcpp
