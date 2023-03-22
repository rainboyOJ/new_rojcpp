//#define TINYASYNC_TRACE

// 带有async_read_timeout,超时检测
#include <iostream>
#include <tinyasync/tinyasync.h>
using namespace tinyasync;

Task<> echo(IoContext &ctx, Connection c)
{
	for(;;) {

        std::size_t nread = 0;
        char b[100];
        try {

            // read some
            printf("wait read...\n");
            nread = co_await c.async_read_timeout(b, 100);
        }
        catch(std::exception & e){
            std::cout << "error" << "\n";
            std::cout << e.what() << "\n";
        }


		if(!nread) {
			break;
		}
		// repeat to send all read
		auto remain = nread;
		char *buf = b;
		for(;;) {
			printf("wait send...");
			auto sent = co_await c.async_send(buf, remain);
			if(!sent)
				break;
			buf += sent;
			remain -= sent;
			if(!remain)
				break;
		}
	}
	
}

Task<> listen(IoContext &ctx)
{

	Acceptor acceptor(ctx, Protocol::ip_v4(), Endpoint(Address::Any(), 8899));
	printf("echo server listening %s:8899\n", acceptor.endpoint().address().to_string().c_str());
	for (;;) {
		Connection conn = co_await acceptor.async_accept();
		co_spawn(echo(ctx, std::move(conn)));
	}

}

void server() {	
	TINYASYNC_GUARD("server():");

	IoContext ctx(std::false_type{}); //改成了非多线程

	co_spawn(listen(ctx));

	TINYASYNC_LOG("run");
	ctx.run();
}

int main()
{
	server();
	return 0;
}


