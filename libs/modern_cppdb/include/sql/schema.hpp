#pragma once

#include <fstream>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "cexpr/string.hpp"

#include "sql/column.hpp"
#include "sql/row.hpp"

namespace cppdb
{

    template <cexpr::string Name, typename... Cols>
    class schema
    {
    public:
        static constexpr auto name{ Name };

        using row_type       = typename cppdb::variadic_row<Cols...>::row_type;
        using container      = std::vector<row_type>;
        using const_iterator = typename container::const_iterator;
        
        schema() = default;
        schema(schema&& sch): table_{std::move(sch.table_)}
        {}

        void operator=(schema && sch ) {
            table_ = std::move(sch.table_);
        }

        template <typename Type, typename... Types>
        schema(std::vector<Type> const& col, Types const&... cols) : schema{}
        {
            insert(col, cols...);
        }

        template <typename Type, typename... Types>
        schema(std::vector<Type>&& col, Types&&... cols) : schema{}
        {
            insert(std::forward<Type>(col), std::forward<Types>(cols)...);
        }

        template <typename... Types>
        inline void emplace(Types const&... vals)
        {
            table_.emplace_back(vals...);
        }

        template <typename... Types>
        inline void emplace(Types&&... vals)
        {
            table_.emplace_back( std::forward<Types>(vals)...);
        }

        template <typename Type, typename... Types>
        void insert(std::vector<Type> const& col, Types const&... cols)
        {
            for (std::size_t i{}; i < col.size(); ++i)
            {
                emplace(col[i], cols[i]...);
            }
        }

        template <typename Type, typename... Types>
        void insert(std::vector<Type>&& col, Types&&... cols)
        {
            for (std::size_t i{}; i < col.size(); ++i)
            {
                emplace(std::forward<Type>(col[i]), std::forward<Types>(cols[i])...);
            }
        }

        void insert(row_type const& row)
        {
            table_.push_back(row);
        }

        void insert(row_type&& row)
        {
            table_.push_back(std::forward<row_type>(row));
        }

        inline auto size()  const noexcept {
            return table_.size();
        }

        inline const_iterator begin() const noexcept
        {
            return table_.begin();
        }

        inline const_iterator end() const noexcept
        {
            return table_.end();
        }

    private:
        container table_;
    };

    namespace
    {

        template <typename Row>
        void fill(std::fstream& fstr, Row& row, [[maybe_unused]] char delim)
        {
            if constexpr (!std::is_same_v<Row, cppdb::void_row>)
            {
                if constexpr (std::is_same_v<typename Row::column::type, std::string>)
                {
                    if constexpr (std::is_same_v<typename Row::next, cppdb::void_row>)
                    {
                        std::getline(fstr, row.head());
                    }
                    else
                    {
                        std::getline(fstr, row.head(), delim);
                    }
                }
                else
                {
                    fstr >> row.head();
                }

                fill<typename Row::next>(fstr, row.tail(), delim);
            }
        }

        template <typename Row>
        void fill(std::fstream& fstr, Row const& row, char delim)
        {
            if constexpr (!std::is_same_v<Row, cppdb::void_row>)
            {
                fstr << row.head();

                if constexpr (std::is_same_v<typename Row::next, cppdb::void_row>)
                {
                    fstr << '\n';
                }
                else
                {
                    fstr << delim;
                }
                
                fill<typename Row::next>(fstr, row.tail(), delim);
            }
        }

    } // namespace


    // for devs who want to use the previous format
    template <typename Type, char Delim>
    inline void store(Type const& data, std::string const& file)
    {
        store<Type>(data, file, Delim);
    }


    //alias
    template <cexpr::string Name, typename... Cols>
    using Result = schema<Name,Cols...>;

    template<typename T>
    struct is_schema_type : public std::false_type {};

    template <cexpr::string Name, typename... Cols>
    struct is_schema_type<cppdb::schema<Name, Cols...>> : public std::true_type {};

} // namespace cppdb
