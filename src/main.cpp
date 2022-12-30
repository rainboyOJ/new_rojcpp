#include "server.h"


int main() {

    Acceptor myacc(Protocol::ip_v4(),Endpoint(Address::Any(),8899));
    server myserver(1,myacc);
    myserver.run();
    myserver.join();
    return 0;
}
