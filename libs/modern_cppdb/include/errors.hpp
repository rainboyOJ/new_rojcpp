#pragma once

//#include <exception>
#include <stdexcept>

namespace cppdb {

    /// \brief This is the base error of all errors thrown by cppdb.
    class cppdb_error : public std::runtime_error {
    public:
        cppdb_error(std::string const & v) : std::runtime_error{v} {}
        cppdb_error(std::string const & file,std::size_t line,std::string const & v) 
            : std::runtime_error{
                "[" + file +"," + std::to_string(line) + "] : " + v
            } {}
    };

    /**
     * 返回的结果0行数据
     */
    class cppdb_zero_row: public cppdb_error {
    public:
        cppdb_zero_row(std::string const & file ,std::size_t line)
            : cppdb_error(file,line,"cppdb_no_result : Seems that the query does not produce any result.")
        {}

    };

    /**
     * 发现 MYSQL_RES * 里没有 列
     */
    class cppdb_zero_filed: public cppdb_error {
    public:
        cppdb_zero_filed(std::string const & file ,std::size_t line)
            : cppdb_error(file,line,"cppdb_no_result : Seems that the query does not produce any result.")
        {}

    };
    
    ///
    /// \brief invalid data conversions
    ///
    /// It may be thrown if the data can't be converted to required format, for example trying to fetch
    /// a negative value with unsigned type or parsing invalid string as datatime.
    ///
    class bad_value_cast : public cppdb_error {
    public:
        bad_value_cast(std::string const& file,std::size_t line) : cppdb_error("cppdb::bad_value_cast can't convert data at file :" + file + " ,line : " + std::to_string(line))
        {}
    };

    ///
    /// \brief attempt to fetch a null value.
    ///
    /// Thrown by cppdb::result::get functions.
    ///
    class null_value_fetch : public cppdb_error {
    public:
        null_value_fetch() : cppdb_error("cppdb::null_value_fetch attempt fetch null column")
        {
        }
    };
    ///
    /// \brief attempt to fetch a value from the row without calling next() first time or when next() returned false.
    ///
    class empty_row_access : public cppdb_error {
    public:
        empty_row_access() : cppdb_error("cppdb::empty_row_access attempt to fetch from empty column")
        {
        }
    };

    ///
    /// \brief trying to fetch a value using invalid column index
    ///
    class invalid_column : public cppdb_error {
    public:
        invalid_column() : cppdb_error("cppdb::invalid_column attempt access to invalid column")
        {
        }
    };
    ///
    /// \brief trying to fetch a value using invalid placeholder
    ///
    class invalid_placeholder : public cppdb_error {
    public:
        invalid_placeholder() : cppdb_error("cppdb::invalid_placeholder attempt bind to invalid placeholder")
        {
        }
    };
    ///
    /// \brief trying to fetch a single row for a query that returned multiple ones.
    ///
    class multiple_rows_query : public cppdb_error {
    public:
        multiple_rows_query() : cppdb_error(    "cppdb::multiple_rows_query "
                            "multiple rows result for a single row request")
        {
        }
    };

    ///
    /// \brief This operation is not supported by the backend
    ///
    class not_supported_by_backend : public cppdb_error {
    public:
        ///
        /// Create a not_supported_by_backend with error message \a e
        ///
        not_supported_by_backend(std::string const &e) :
            cppdb_error(e)
        {
        }
    };

} // end namespace cppdb

