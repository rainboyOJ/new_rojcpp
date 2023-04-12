#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <type_traits>

namespace cexpr
{

    //compile time string
    template <typename Char, std::size_t N>
    class string
    {
    public:
        using char_type = Char;

        constexpr string() noexcept : size_{ 0 }, string_{ 0 }
        {}

        constexpr string(const Char(&s)[N]) noexcept : string{}
        {
            for(; s[size_]; ++size_)
            {
                string_[size_] = s[size_];
            }
        }

        constexpr string(cexpr::string<Char, N> const& s) noexcept : string{}
        {
            for (; s[size_]; ++size_)
            {
                string_[size_] = s[size_];
            }
        }

        //template<std::size_t pos>
        constexpr Char get(std::size_t pos) const{
            return string_[pos];
        }

        constexpr string(std::basic_string_view<Char> const& s) noexcept : string{}
        {
            if (s.length() < N)
            {
                for (; size_ < s.length(); ++size_)
                {
                    string_[size_] = s[size_];
                }
            }
        }

        constexpr void fill(const Char* begin, const Char* end) noexcept
        {
            fill_from(begin, end, begin());
        }

        constexpr void fill_from(const Char* begin, const Char* end, Char* start) noexcept
        {
            if (end - begin < N)
            {
                for (auto curr{ start }; begin != end; ++begin, ++curr)
                {
                    *curr = *begin;
                }
            }
        }

        inline constexpr std::size_t capacity() const noexcept
        { 
            return N - 1;
        }

        inline constexpr std::size_t size() const noexcept
        {
            return size_;
        }

        inline constexpr Char* begin() noexcept
        {
            return string_;
        }
        inline constexpr const Char* cbegin() const noexcept
        {
            return string_;
        }

        inline constexpr Char* end() noexcept
        {
            return &string_[size_];
        }
        inline constexpr const Char* cend() const noexcept
        {
            return &string_[size_];
        }

        inline constexpr Char& operator[](std::size_t i)
        {
            return string_[i];
        }
        inline constexpr Char const& operator[](std::size_t i) const
        {
            return string_[i];
        }

        template <typename OtherChar, std::size_t OtherN>
        constexpr bool operator==(string<OtherChar, OtherN> const& other) const noexcept
        {
            if constexpr (N != OtherN)
            {
                return false;
            }

            std::size_t i{};
            for (; i < N && string_[i] == other[i]; ++i);

            return i == N;
        }

        template <typename OtherChar, std::size_t OtherN>
        constexpr bool operator==(const OtherChar(&other)[OtherN]) const noexcept
        {
            if constexpr (N != OtherN)
            {
                return false;
            }

            std::size_t i{};
            for (; i < N && string_[i] == other[i]; ++i);

            return i == N;
        }

        template <typename OtherChar>
        inline bool operator==(std::basic_string<OtherChar> const& other) const noexcept
        {
            return other == string_;
        }

        template <typename OtherChar>
        inline bool operator!=(std::basic_string<OtherChar> const& other) const noexcept
        {
            return !(other == string_);
        }

        std::size_t size_;
        Char string_[N];
    };

    template <typename Char, std::size_t N>
    string(const Char[N]) -> string<Char, N>;

    template <typename Char, std::size_t N>
    inline bool operator==(std::basic_string<Char> const& str, string<Char, N> const& cstr) noexcept
    {
        return cstr == str;
    }

    template <typename Char, std::size_t N>
    inline bool operator!=(std::basic_string<Char> const& str, string<Char, N> const& cstr) noexcept
    {
        return cstr != str;
    }


    template<typename T>
    struct is_cexpr_string : std::false_type {};
    
    template <typename Char, std::size_t N>
    struct is_cexpr_string <cexpr::string<Char, N>> : std::true_type {};

} // namespace cexpr
  //

//增加功能
namespace std {
    // std::string += cexpr::string
    template <typename Char, std::size_t N>
    std::string& operator+=(std::string & str,cexpr::string<Char,N> const & cstr) {
        for(int i =0;i<cstr.size_;i++) {
            str += cstr.string_[i];
        }
        return str;
    }
}
