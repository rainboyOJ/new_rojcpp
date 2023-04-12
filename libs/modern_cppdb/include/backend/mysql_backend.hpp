#pragma once

#include <vector>

#include <mysql/mysql.h>

#include "connection_info.hpp"
#include "utils.hpp"

namespace cppdb {
namespace backend {

class result;

class connection {
public:
    friend class result;
    connection(std::string const & str)
        :connection(connection_info(str))
    {}

    connection(connection_info const &ci) : 
        conn_(0),
        last_used{std::chrono::system_clock::now()}
    {
        conn_ = mysql_init(0);
        if(!conn_) {
            throw cppdb_error("cppdb::mysql failed to create connection");
        }
        std::string host         = ci.get("host","");
        char const *phost        = host.empty() ? 0 : host.c_str();
        std::string user         = ci.get("user","");
        char const *puser        = user.empty() ? 0 : user.c_str();
        std::string password     = ci.get("password","");
        char const *ppassword    = password.empty() ? 0 : password.c_str();
        std::string database     = ci.get("database","");
        char const *pdatabase    = database.empty() ? 0 : database.c_str();
        int port                 = ci.get("port",3306);
        std::string unix_socket  = ci.get("unix_socket","");
        char const *punix_socket = unix_socket.empty() ? 0 : unix_socket.c_str();

#if MYSQL_VERSION_ID >= 50507
        std::string default_auth = ci.get("default_auth","");
        if (!default_auth.empty()) {
            mysql_set_option(MYSQL_DEFAULT_AUTH, default_auth.c_str());
        }
#endif
        std::string init_command = ci.get("init_command","");
        if(!init_command.empty()) {
            mysql_set_option(MYSQL_INIT_COMMAND, init_command.c_str());
        }
        if(ci.has("opt_compress")) {
            if(ci.get("opt_compress", 1)) {
                mysql_set_option(MYSQL_OPT_COMPRESS, NULL);
            }
        }
        if(ci.has("opt_connect_timeout")) {
            if(unsigned connect_timeout = ci.get("opt_connect_timeout", 0)) {
                mysql_set_option(MYSQL_OPT_CONNECT_TIMEOUT, &connect_timeout);
            }
        }
        if(ci.has("opt_guess_connection")) {
            if(ci.get("opt_guess_connection", 1)) {
                mysql_set_option(MYSQL_OPT_GUESS_CONNECTION, NULL);
            }
        }
        if(ci.has("opt_local_infile")) {
            if(unsigned local_infile = ci.get("opt_local_infile", 0)) {
                mysql_set_option(MYSQL_OPT_CONNECT_TIMEOUT, &local_infile);
            }
        }
        if(ci.has("opt_named_pipe")) {
            if(ci.get("opt_named_pipe", 1)) {
                mysql_set_option(MYSQL_OPT_NAMED_PIPE, NULL);
            }
        }
        if(ci.has("opt_protocol")) {
            if(unsigned protocol = ci.get("opt_protocol", 0)) {
                mysql_set_option(MYSQL_OPT_PROTOCOL, &protocol);
            }
        }
        if(ci.has("opt_read_timeout")) {
            if(unsigned read_timeout = ci.get("opt_read_timeout", 0)) {
                mysql_set_option(MYSQL_OPT_READ_TIMEOUT, &read_timeout);
            }
        }
        if(ci.has("opt_reconnect")) {
            if(unsigned reconnect = ci.get("opt_reconnect", 1)) {
                my_bool value = reconnect;
                mysql_set_option(MYSQL_OPT_RECONNECT, &value);
            }
        }
#if MYSQL_VERSION_ID >= 50507
        std::string plugin_dir = ci.get("plugin_dir", "");
        if(!plugin_dir.empty()) {
            mysql_set_option(MYSQL_PLUGIN_DIR, plugin_dir.c_str());
        }
#endif
        std::string set_client_ip = ci.get("set_client_ip", "");
        if(!set_client_ip.empty()) {
            mysql_set_option(MYSQL_SET_CLIENT_IP, set_client_ip.c_str());
        }
        if(ci.has("opt_ssl_verify_server_cert")) {
            if(unsigned verify = ci.get("opt_ssl_verify_server_cert", 1)) {
                my_bool value = verify;
                mysql_set_option(MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &value);
            }
        }
        if(ci.has("opt_use_embedded_connection")) {
            if(ci.get("opt_use_embedded_connection", 1)) {
                mysql_set_option(MYSQL_OPT_USE_EMBEDDED_CONNECTION, NULL);
            }
        }
        if(ci.has("opt_use_remote_connection")) {
            if(ci.get("opt_use_remote_connection", 1)) {
                mysql_set_option(MYSQL_OPT_USE_REMOTE_CONNECTION, NULL);
            }
        }
        if(ci.has("opt_write_timeout")) {
            if(unsigned write_timeout = ci.get("opt_write_timeout", 0)) {
                mysql_set_option(MYSQL_OPT_WRITE_TIMEOUT, &write_timeout);
            }
        }
        std::string read_default_file = ci.get("read_default_file", "");
        if(!read_default_file.empty()) {
            mysql_set_option(MYSQL_READ_DEFAULT_FILE, read_default_file.c_str());
        }
        std::string read_default_group = ci.get("read_default_group", "");
        if(!read_default_group.empty()) {
            mysql_set_option(MYSQL_READ_DEFAULT_GROUP, read_default_group.c_str());
        }
        if(ci.has("report_data_truncation")) {
            if(unsigned report = ci.get("report_data_truncation", 1)) {
                my_bool value = report;
                mysql_set_option(MYSQL_REPORT_DATA_TRUNCATION, &value);
            }
        }
#if MYSQL_VERSION_ID >= 40101
        if(ci.has("secure_auth")) {
            if(unsigned secure = ci.get("secure_auth", 1)) {
                my_bool value = secure;
                mysql_set_option(MYSQL_SECURE_AUTH, &value);
            }
        }
#endif
        std::string set_charset_dir = ci.get("set_charset_dir", "");
        if(!set_charset_dir.empty()) {
            mysql_set_option(MYSQL_SET_CHARSET_DIR, set_charset_dir.c_str());
        }
        std::string set_charset_name = ci.get("set_charset_name", "");
        if(!set_charset_name.empty()) {
            mysql_set_option(MYSQL_SET_CHARSET_NAME, set_charset_name.c_str());
        }
        std::string shared_memory_base_name = ci.get("shared_memory_base_name", "");
        if(!shared_memory_base_name.empty()) {
            mysql_set_option(MYSQL_SHARED_MEMORY_BASE_NAME, shared_memory_base_name.c_str());
        }
        
        if(!mysql_real_connect(conn_,phost,puser,ppassword,pdatabase,port,punix_socket,0)) {
            std::string err="unknown";
            try { err = mysql_error(conn_); }catch(...){}
            mysql_close(conn_);
            throw cppdb_error(err);
        }

    }

    ~connection()
    {
        if(conn_)
            mysql_close(conn_);
    }

    inline void update_last_used()  { last_used = std::chrono::system_clock::now(); }
    inline auto get_last_used() const{ return  last_used; }

    void exec(std::string_view s) 
    {
        if(mysql_real_query(conn_,s.data(),s.length())) {
            throw cppdb_error(mysql_error(conn_));
        }
    }

    unsigned long long server_version(){
        return mysql_get_server_version(conn_);
    }

    std::string escape(std::string_view s)
    {
        std::vector<char> buf(2*s.length()+1);
        size_t len = mysql_real_escape_string(conn_,&buf.front(),s.data(),s.length());
        std::string result;
        result.assign(&buf.front(),len);
        return result;
    }

    std::string_view driver() const 
    {
        using namespace std::literals;
        return "mysql"sv;
    }

    std::string_view engine() const 
    {
        using namespace std::literals;
        return "mysql"sv;
    }

    MYSQL * get_raw_conn() const{
        return conn_;
    }

private:


    /// Set a custom MYSQL option on the connection.
    void mysql_set_option(mysql_option option, const void* arg)
    {
        // char can be casted to void but not the other way, support older API
        if(mysql_options(conn_, option, static_cast<char const *>(arg))) {
            throw cppdb_error("cppdb::mysql failed to set option");
        }
    }

    connection_info ci_;
    MYSQL *conn_;
    std::chrono::time_point<std::chrono::system_clock> last_used;
};

} // end namespace backend
} // end namespace cppdb

