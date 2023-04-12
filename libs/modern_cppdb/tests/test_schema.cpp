/**
 * 测试schema的使用
 */
#include <iostream>
#include "sql/schema.hpp"
#include "utils.hpp"

namespace  sql = cppdb;

template<typename T>
void print(const T & a){
    const char *b = a.cbegin();
    const char *e = a.cend();
    while ( b != e ) {
        std::cout << *b;
        b++;
    }
    //for (const auto e : a) {
        //std::cout << e ;
    //}
}

template<typename Row,cexpr::string Name,typename Type>
void do_work(std::size_t idx,Row & r,sql::column<Name, Type> ){
    using Col = sql::column<Name, Type>;
    std::cout << "idx: " << idx << std::endl;
    print(Col::name);
    std::cout << "\n+++++++++" << std::endl;
}

int main(){
    // column
    sql::column<"integer", int > col1;
    sql::column<"integer", char > col2;
    std::cout << (col1.name == col2.name) << std::endl;
    std::cout << (col1.name == col1.name) << std::endl;
    print(col1.name);
    std::cout << std::endl;
    decltype(col1)::type x = 100;
    std::cout << "x : " << x<< std::endl;


    // row
    // 1.row type
    
    using RowType = sql::variadic_row<
        sql::column<"internal",int>,
        sql::column<"char",char>,
        sql::column<"std::string",std::string>,
        sql::column<"hello",double>
    >::row_type ;

    RowType row1;
    std::cout << GET_TYPE_NAME(row1) << std::endl;
    // 2.set value
    sql::set<"internal">(row1, 100);
    // 3.get value
    int v = sql::get<"internal">(row1);
    // 4. print allow value
    //
    std::cout << "get value from row1 : " << v << std::endl;



    RowType row2(1,'c',"hello",1.23);
    //sql::variadic_row<
        //sql::column<"internal",int> >::row_type row2(1);
    std::cout << sql::get<"internal">(row2) << std::endl;
    std::cout << sql::get<"char">(row2) << std::endl;
    std::cout << sql::get<"std::string">(row2) << std::endl;
    std::cout << sql::get<"hello">(row2) << std::endl;

    std::cout << "=type" << std::endl;
    using TT =  std::tuple_element<1,RowType>::type ;
    std::cout << GET_TYPE_NAME(TT) << std::endl;

    using ColType = sql::row_element<1, RowType>::type;
    std::cout << GET_TYPE_NAME(ColType) << std::endl;
    print(ColType::name);
    ColType::type  t1 = 100;
    std::cout  << std::endl;

    std::cout << "\n\n\n\n";



    using RowType2 = sql::variadic_row<
        sql::column<"internal",int> ,
        sql::column<"string",std::string> 
        >::row_type;
    std::vector<RowType2> row_vec;
    row_vec.emplace_back(1,"hello");
    std::cout << "row_vec : " << std::endl;
    std::cout << sql::get<"internal">(row_vec[0]) << std::endl;
    std::cout << sql::get<"string">(row_vec[0]) << std::endl;


    // schema
    // 1. 创建schema
    using books = sql::Result<"books", 
          sql::column<"bookName", std::string>,
          sql::column<"bookId", uint64_t>
        >;
    books mybook;
    // 2. 赋值
    mybook.emplace("bookname",100u);

    books::row_type book_row;

    //each_row_column(book_row, [](auto idx,auto &r,auto c){
    //    if constexpr ( std::is_same_v<typename decltype(c)::type, uint64_t> ){
    //        sql::set<decltype(c)::name>(r,123);
    //        return;
    //    }
    //    if constexpr ( std::is_same_v<typename decltype(c)::type, std::string> ){
    //        sql::set<decltype(c)::name>(r,"newbook123");
    //        return;
    //    }
    //});
    std::cout << "book_row" << std::endl;
    std::cout << sql::get<0>(book_row) << std::endl;
    std::cout << sql::get<1>(book_row) << std::endl;

    mybook.insert(std::move(book_row));
    std::cout << "mybook size : " << mybook.size() << std::endl;

    // 3. 得到值
    return 0;
}
