#include "judge/judgeServerMediator.hpp"

judgeServerMediator<rojcpp::server> & JSM() {
    static judgeServerMediator<rojcpp::server> myJSM(__config__::server_size);
    return myJSM;
}
