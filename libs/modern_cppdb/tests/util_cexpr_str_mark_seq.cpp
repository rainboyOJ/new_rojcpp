#include <iostream>
#include <utility>

#include "cexpr/string.hpp"
#include "utils.hpp"

template<std::size_t idx>
constexpr auto gen_seq() -> std::index_sequence<>;

template<std::size_t idx,std::size_t... Is>
constexpr auto index_seq_push_front( std::index_sequence<Is...> )  -> std::index_sequence<idx,Is...>;

template<std::size_t... As,std::size_t... Bs>
constexpr auto operator+(std::index_sequence<As...>,std::index_sequence<Bs...>) 
    -> std::index_sequence<As...,Bs...>
{ return {}; }

template<cexpr::string str>
struct node {
    static constexpr auto S {str};
    static constexpr std::size_t size__ = S.size();

    static constexpr auto get()
    {
        return  filter(std::make_index_sequence<size__>{});
    }

    template<std::size_t... Is>
    static constexpr auto filter(std::index_sequence<Is...>){
        return (filter_single<Is>()  + ...);
    }

    template <std::size_t Val>
    static constexpr auto filter_single() {
        if constexpr ( S.get(Val) == '?')
            return std::index_sequence<Val> {};
        else
            return std::index_sequence<> {};
    }
    using index_seq = decltype(get());

};

template<std::size_t ...Is>
void print_seq(std::index_sequence<Is...>){
    ( (std::cout << Is << ","),...);
}

int main(){

    //using T = cexpr::string<typename Char, std::size_t N>
    constexpr
    cexpr::string X("select ? from ? (?)");
    
    std::cout << GET_TYPE_NAME(decltype(X)) << std::endl;

    using T = node<"select ?">::index_seq;
    std::cout << GET_TYPE_NAME(T) << std::endl;
    print_seq(T{});
    std::cout  << std::endl;

    print_seq(node<X>::index_seq {} );
    
    return 0;
}
