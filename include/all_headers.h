//所有的头文件

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <memory>
#include <memory_resource>

namespace rojcpp {

#ifdef USE_POLYMORPIC_RESOURCE

    template<typename T>
    using __vector = std::pmr::vector<T>;

    template<typename Key,typename T>
    using __map    = std::pmr::map<Key,T>;

    using __string = std::pmr::string;

#else

    template<typename Tp>
    using __vector = std::vector<Tp>;

    template<typename Tp,typename Tp2>
    using __map    = std::map<Tp,Tp2>;

    using __string = std::string;

#endif


} // end namespace
