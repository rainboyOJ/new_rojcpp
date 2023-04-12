#pragma once
#include <map>
#include "utils.hpp"

namespace cppdb {

    ///
    /// \brief Class that represents parsed connection string
    ///
    class connection_info {
    public:
        ///
        /// The original connection string
        ///
        std::string connection_string;
        ///
        /// The driver name
        ///
        std::string driver;
        ///
        /// Type that represent key, values set
        ///
        typedef std::map<std::string,std::string> properties_type;
        ///
        /// The std::map of key value properties.
        ///
        properties_type properties;
        
        ///
        /// Cheks if property \a prop, has been given in connection string.
        ///
        bool has(std::string const &prop) const;
        ///
        /// Get property \a prop, returning \a default_value if not defined.
        ///
        std::string get(std::string const &prop,std::string const &default_value=std::string()) const;
        ///
        /// Get numeric value for property \a prop, returning \a default_value if not defined. 
        /// If the value is not a number, throws cppdb_error.
        ///
        int get(std::string const &prop,int default_value) const;
    
        ///
        /// Default constructor - empty info
        ///    
        connection_info()
        {
        }
        ///
        /// Create connection_info from the connection string parsing it.
        ///
        explicit connection_info(std::string const &cs) :
            connection_string(cs)
        {
            parse_connection_string(cs,driver,properties);
        }

    };

    std::string connection_info::get(std::string const &prop,std::string const &default_value) const
    {
        properties_type::const_iterator p=properties.find(prop);
        if(p==properties.end())
            return default_value;
        else
            return p->second;
    }

    bool connection_info::has(std::string const &prop) const
    {
        return properties.find(prop) != properties.end();
    }

    int connection_info::get(std::string const &prop,int default_value) const
    {
        properties_type::const_iterator p=properties.find(prop);
        if(p==properties.end())
            return default_value;
        std::istringstream ss;
        ss.imbue(std::locale::classic());
        ss.str(p->second);
        int val;
        ss >> val;
        if(!ss || !ss.eof()) {
            throw cppdb_error("cppdb::connection_info property " + prop + " expected to be integer value");
        }
        return val;
    }

} // end namespace cppdb
