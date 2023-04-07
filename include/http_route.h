/**
 * @file
 * @brief 实现路由功能
 *
 * 路由带有ap点,可以在路由函数执行前执行,也可以在之后执行.
 *
 * target(目标)
 *
 * 看起来只需要只一个 std::map<string,Function> 就可以实现了.但实际的情况更复杂一些.
 * 
 * url对应的不仅是一个函数(function),还有Method(GET,POST,DELETE...).显然一个url可以
 * 既有POST,也有DELETE.
 * 同时function也不能仅是一个函数类型(函数指针,仿函数),它还要有AP的功能.AP(Access Point)
 * 主要作用是在真正执行路由函数function之前或之后所执行的一些预处理或后序处理函数.
 * 如果,检查用户是否登录等.
 *
 * AP 是一个这样的结构体
 *
 * struct {
 *      bool before(request&,response&);
 *      bool after(request&,response&);
 * };
 *
 * 具有before 和 after这两个成员函数.
 *
 * complement(实现)
 *
 */


#pragma once

#include <map>
#include <unordered_map>
#include <functional>
#include <string_view>
#include <iostream>
#include "tools/mime_types.hpp"
#include "http_request.h"
#include "http_response.h"


namespace rojcpp {

    namespace {
        constexpr char DOT = '.';
        constexpr char SLASH = '/';
        constexpr std::string_view INDEX = "index";
    }

    /**
     * @class http_router
     * @brief helloworld
     *
     */
    class http_router {

    public:
            
        template<http_method... Is,typename Function,typename... AP>
        void register_handler(
                std::string_view raw_name,
                Function && f,
                AP &&... ap
        ){
            if constexpr ( sizeof...(Is) > 0 ){
                // 将enum整数template vair arguments 转成 std::array<char,26>类型
                auto arr = get_method_arr<Is...>();
                register_nonmember_func(raw_name, arr, std::forward<Function>(f), std::forward<AP>(ap)...);
            }
            else {
                register_nonmember_func(raw_name, {0}, std::forward<Function>(f), std::forward<AP>(ap)...);
            }
        }

        //TODO
        void remove_handler();

        //执行路由
        /**
         * @brief 执行路由
         *
         * @param method GET,POST,DELETE 等Method
         * @param url url_string
         * @param req 
         * @param res 
         * @return true 表示找到并执行了路由,false 表示没有找到路由
         */
        bool route(std::string_view method,std::string_view url, request& req,response & res){

            //查询完全匹配
            auto it = map_invokers_.find(url);
            if( it != map_invokers_.end() ) {
                auto & pair = it->second;
                if( method[0] < 'A' || method[0] > 'Z') {
                    return false;
                }

                //Corresponding Method exists
                if( pair.first[method[0] - 'A'] != 0) {
                    pair.second(req,res);
                    return true;
                }
                // not match
                return false;
            }
            return false;
        }

        //TODO
        bool has_route(std::string_view method,std::string_view url, request& req,response & res);

    private:
        

        /**
         * @brief 注册
         *
         * @tparam Function 
         * @tparam AP 
         * @param raw_name 
         * @param arr 
         * @param f 
         * @param ap 
         * @return 
         */
        template<typename Function,typename... AP>
        void register_nonmember_func(   
                std::string_view raw_name,
                const std::array<char,26>& arr,
                Function f,
                AP&&... ap
        ){
            this->map_invokers_[raw_name] = {
                arr,
                // std::bind(&http_router::invoke<Function,Ap...>,this,std::p)
                [this,f= std::move(f),...ap = std::forward<AP>(ap)](request & a1,response & a2) {
                    this->invoke(a1, a2,std::move(f), std::forward<AP>(ap)...);
                }
            };
        }

        
        /**
         * @brief 执行路由函数
         *
         * @tparam Function 
         * @tparam AP 一种struct结构体
         * @param req 
         * @param res 
         * @param f 
         * @param ap 
         */
        template<typename Function,typename... AP>
        void invoke(request & req,response& res, Function f, AP... ap) {
            // using result_type = std::result_of_t<Function(request&,response&)>;

            // if constexpr ( std::is_void_v<result_type>) {

            std::tuple<AP... > tp(std::move(ap)...);

            //在执行ap_before的过程中,如果有一个ap返回false,就直接结束
            //不向后执行
            if( !do_ap_before(req,res,tp)) 
                return;
            f(req,res);
            // do_void_after(req,res,tp);
            do_after(req,res,tp);

            // }

        }

        template<typename Tuple>
        bool do_ap_before(request & req,response & res,Tuple & tp) {
            bool r = true;
            //依次执行 tuple里的东西
            for_each_l(tp, [&r,&req,&res](auto & item){
                        if( !r) return;

                        constexpr bool has_before_mtd = has_before<decltype(item),request&,response&>::value;
                        if constexpr ( has_before_mtd )
                            r= item.before(req,res);

                    }, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
            return r;
        }

        template<typename Tuple>
        void do_after(request & req, response& res, Tuple & tp) {
            bool r = true;
            //依次执行 tuple里的东西
            for_each_r(tp, [&r,&req,&res](auto & item){
                        if( !r) return;

                        constexpr bool has_after_mtd = has_after<decltype(item),request&,response&>::value;
                        if constexpr ( has_after_mtd )
                            r= item.after(req,res);

                    }, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
        }


        
        using invoker_function_ = std::pair<std::array<char,26>,std::function<void(request&,response&)>>;
        std::map<std::string_view,invoker_function_> map_invokers_;
    };

    




} // end namespace rojcpp
