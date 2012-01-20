#include "../../src/ting/debug.hpp"
#include "../../src/ting/Socket.hpp"

#include "dns.hpp"
#include "socket.hpp"


int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<< "Socket test " << std::endl)

	ting::net::Lib netLib;

	BasicIPAddressTest::Run();
	BasicClientServerTest::Run();
	BasicUDPSocketsTest::Run();
	SendDataContinuouslyWithWaitSet::Run();
	SendDataContinuously::Run();

	TRACE_ALWAYS(<< "[PASSED]: Socket test" << std::endl)

	return 0;
}
