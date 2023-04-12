#pragma once

#include "cexpr/string.hpp"

namespace cppdb
{

    template <cexpr::string Name, typename Type>
    struct column
    {
        static constexpr auto name{ Name };
        
        using type = Type;
    };


template<typename T>
struct is_column_type : public std::false_type {};

template<cexpr::string Name,typename Type>
struct is_column_type<cppdb::column<Name, Type>> : public std::true_type {};


} // namespace sql
