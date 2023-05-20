/**
 * @desc 和 评测相关这个表相关的CURD
 *
 * table
 * - solutions
 * - solution_codes
 * - solution_full_results
 *
 */

#include <string_view>
#include "curd.h"

namespace CURD {

struct judgeTable {
    
    // 添加一条提交记录
    static
    unsigned long long add_solutions(
            unsigned long long owner_id,
            std::string_view problem_id,
            std::string_view lang)
    {
        cppdb::query<"insert into solutions (owner_id,problem_id,lang)"
        "values(?,?,'?') RETURNING id;",unsigned long long> q;
        LOG_DEBUG << "before exec query api";
        unsigned solution_id = 
            q << owner_id << problem_id << lang << cppdb::exec;
        LOG_DEBUG << "after exec query api";
        return solution_id;
    }

    /**
     * 将评测的结果存入数据库
     */
    static 
        void update_solution(
                std::string_view solution_id,
                int result, // -3:RJ WAIT -2:WAIT -1:RUN 0:AC 1:WA 2:PE 3:TLE 4:MLE 5:OLE 6:RE 7:SE
                unsigned long long time_used,
                unsigned long long memory_used,
                int score,
                std::string_view mini_result,
                std::string_view full_result
                )
    {
        cppdb::query<"UPDATE solutions SET result = ?, time_used = ?,"
        "memory_used = ?,"
        "mini_result = '?',"
        "score = ?"
        " WHERE id = ?;",
        void > q;

        q << result << time_used << memory_used << mini_result << score;
        q << solution_id;
        q << cppdb::exec;

        cppdb::query<"UPDATE solution_full_results SET "
            "data = '?'"
            "where solution_id = ?;"
            , void> q2;
        q2 << full_result;
        q2 << solution_id << cppdb::exec;
    }

    // 评测的代码,另一个表里
    static
    void add_solution_codes(
            unsigned long long solution_id,
            std::string_view code)
    {
        cppdb::query< "insert into solution_codes (solution_id,code) values (?,'?');" ,void> q;
              q << solution_id << code << cppdb::exec;
    }

    // 完成的结果,记录在另一个表里
    static
    void add_solution_full_result(unsigned long long solution_id)
    {
        cppdb::query<"insert into solution_full_results(solution_id) "
            "values (?)" ,void> q;
        q << solution_id << cppdb::exec;
    }

};

} // end namespace CURD

