#pragma once
#include <iostream>
#include <iomanip>
#include "sql/query.hpp"

template<typename Clock, typename Duration>
std::ostream &operator<<(std::ostream &stream,
  const std::chrono::time_point<Clock, Duration> &time_point) {
  const time_t time = Clock::to_time_t(time_point);
#if __GNUC__ > 4 || \
    ((__GNUC__ == 4) && __GNUC_MINOR__ > 8 && __GNUC_REVISION__ > 1)
  // Maybe the put_time will be implemented later?
  struct tm tm;
  localtime_r(&time, &tm);
  return stream << std::put_time(&tm, "%c"); // Print standard date&time
#else
  char buffer[26];
  ctime_r(&time, buffer);
  buffer[24] = '\0';  // Removes the newline that is added
  return stream << buffer;
#endif
}

template<typename Row,std::size_t... Is>
constexpr void __print_row(Row &r,std::index_sequence<Is...>){
    (( std::cout << '"' << cppdb::get<Is>(r) << "\" " ),...);
    std::cout <<'\n';
}

template<typename Row>
void print_row(Row &r){
    __print_row(r, std::make_index_sequence<Row::depth> {});
}
