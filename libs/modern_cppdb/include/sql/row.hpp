#pragma once

#include <type_traits>
#include <utility>
#include <vector>

#include "cexpr/string.hpp"
#include "column.hpp"

namespace cppdb
{

    struct void_row
    {
        static constexpr std::size_t depth{ 0 };
    };

    template <typename Col, typename Next>
    class row
    {
    public:
        using column = Col;
        using next = Next;

        static constexpr std::size_t depth{ 1 + next::depth };

        row() = default;

        template <typename... ColTs>
        row(typename column::type const& val, ColTs const&... vals) : value_{ val }, next_{ vals... }
        {}

        template <typename... ColTs>
        row(typename column::type&& val, ColTs&&... vals) : value_{ std::forward<typename column::type>(val) }, next_{ std::forward<ColTs>(vals)... }
        {}

        inline constexpr next const& tail() const noexcept
        {
            return next_;
        }

        inline constexpr next& tail() noexcept
        {
            return next_;
        }

        inline constexpr typename column::type const& head() const noexcept
        {
            return value_;
        }

        inline constexpr typename column::type& head() noexcept
        {
            return value_;
        }
    
    private:
        typename column::type value_;
        next next_;
    };

    //工具类, 得到row的类型
    template <typename Col, typename... Cols>
    struct variadic_row
    {
    private:
        static inline constexpr auto resolve() noexcept
        {
            if constexpr (sizeof...(Cols) != 0)
            {
                return typename variadic_row<Cols...>::row_type{};
            }
            else
            {
                return void_row{};
            }
        }

    public:
        using row_type = row<Col, decltype(resolve())>;
    };

    template<typename... Cols>
    using row_type = typename cppdb::variadic_row<Cols...>::row_type;



    // user function to query row elements by column name
    template <cexpr::string Name, typename Row>
    constexpr auto const& get(Row const& r) noexcept
    {
        static_assert(!std::is_same_v<Row, cppdb::void_row>, "Name does not match a column name.");

        if constexpr (Row::column::name == Name)
        {
            return r.head();
        }
        else
        {
            return get<Name>(r.tail());
        }
    }

    // compiler function used by structured binding declaration
    template <std::size_t Pos, typename Row>
    constexpr auto const& get(Row const& r) noexcept
    {
        static_assert(Pos < Row::depth, "Position is larger than number of row columns.");

        if constexpr (Pos == 0)
        {
            return r.head();
        }
        else
        {
            return get<Pos - 1>(r.tail());
        }
    }

    // function to assign a value to a column's value in a row
    template <cexpr::string Name, typename Row, typename Type>
    constexpr void set(Row& r, Type const& value)
    {
        static_assert(!std::is_same_v<Row, cppdb::void_row>, "Name does not match a column name.");

        if constexpr (Row::column::name == Name)
        {
            r.head() = value;
        }
        else
        {
            set<Name>(r.tail(), value);
        }
    }

    template <std::size_t Pos, typename Row, typename Type>
    constexpr void set(Row& r, Type const& value)
    {
        static_assert(!std::is_same_v<Row, cppdb::void_row>, "Name does not match a column name.");

        if constexpr (Pos == 0)
        {
            r.head() = value;
        }
        else
        {
            set<Pos-1>(r.tail(), value);
        }
    }

    template <std::size_t Index, typename>
    struct row_element;

    template <std::size_t Index, typename Col, typename Next>
    struct row_element<Index,cppdb::row<Col,Next>> 
    {
        template<std::size_t Pos,typename Col_, typename Next_>
        static constexpr auto __type(cppdb::row<Col_,Next_>) {
            if constexpr (Pos == 0)
                return Col_{};
            else
                return __type<Pos-1>(Next_{});
        }
        using type = decltype(__type<Index>(cppdb::row<Col, Next>{}));
    };

} // namespace cppdb

// STL injections to allow row to be used in structured binding declarations
namespace std
{

    template <typename Col, typename Next>
    class tuple_size<cppdb::row<Col, Next>> : public integral_constant<std::size_t, cppdb::row<Col, Next>::depth>
    {};

    template <std::size_t Index, typename Col, typename Next>
    struct tuple_element<Index, cppdb::row<Col, Next>>
    {
        //using type = decltype(cppdb::get<Index>(cppdb::row<Col, Next>{}));
        using TTT  =  cppdb::row_element<Index, cppdb::row<Col,Next>>;
        using type = typename TTT::type::type;
    };


} // namespace std

namespace cppdb {

template<typename T>
struct is_row_type : public std::false_type {};

// 是否是row 类型
template<>
struct is_row_type<cppdb::void_row> : public std::true_type{};

template<typename Col,typename Next>
struct is_row_type<cppdb::row<Col, Next>> {
    static constexpr bool value = cppdb::is_row_type<Next>::value && cppdb::is_column_type<Col>::value;
};


//struct is_row_type {

    //template<typename Col,typename Next>
    //static constexpr auto check(row<Col, Next> _row) {
        //if constexpr (std::is_same_v<Next, void_row>)
            //return std::true_type{};
        //return check(std::declval<Next>());
    //} 

    //template<typename Col,typename Next>
    //static constexpr auto check(...) -> std::false_type ;

    //static constexpr bool value =  decltype(check(std::declval<T>()))::value;
//};

//工具函数 目的,依次设置值
template<typename Row,typename Func,std::size_t... idx>
constexpr void __each_row_column(Row& r,Func&& f,
        std::index_sequence<idx...>) 
{
    //(std::forward<Func>(f)(idx,r,typename std::tuple_element<idx,Row>::type {}),... );
    //调用set 依次设置值
    (cppdb::set<idx>(r, f(idx,typename std::tuple_element<idx,Row>::type {})),...);
}

//template<typename Row,typename Func>
//constexpr void each_row_column(Row& r,Func&& f)
//{
    //__each_row_column(r, std::forward<Func>(f),
            //std::make_index_sequence<Row::depth> {}
            //);
//}


}// namespace sql 
