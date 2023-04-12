#pragma once

namespace cppjson {
    
struct NoSuchClassException:public std::exception
{ 
public:
    explicit NoSuchClassException(const std::string&type_name) {
        std::ostringstream oss;
        oss<<"Class '"<<type_name<<"' dose not exist or not reflectable.";
        this->message=oss.str();
    }
    virtual ~NoSuchClassException()throw() {}
    virtual const char*what()const throw() { return message.c_str(); }
protected: 
    std::string message;
};


struct NoSuchFieldException:public std::exception
{
public:
    explicit NoSuchFieldException(std::string_view type_name,std::string_view field_name){
        std::ostringstream oss;
        oss<<"Object of type <"
            << type_name 
            << "> dose not have field named '" 
            << field_name << "' .";
        this->message = oss.str();
    }
    virtual ~NoSuchFieldException()throw() {}
    virtual const char*what()const throw() { return message.c_str();}
protected: 
    std::string message;
};

class JsonDecodeException:public std::exception
{
public:
    explicit JsonDecodeException():message("JsonDecodeException: ") {};
    virtual ~JsonDecodeException() throw() {};
    virtual const char*what()const throw()=0;
protected:
    std::string message;
};

class JsonDecodeDelimiterException:public JsonDecodeException
{
public:
    explicit JsonDecodeDelimiterException(const char&ch) : JsonDecodeException()
    {
        std::ostringstream oss;
        oss<<"Expecting '"<<ch<<"' delimiter in decoding json data.";
        message += oss.str();
    }

    virtual ~JsonDecodeDelimiterException(){};
    virtual const char*what()const throw() { return this->message.c_str(); }
};

} // end namespace cppjson

