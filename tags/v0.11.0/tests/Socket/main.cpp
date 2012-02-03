#include "../../src/ting/debug.hpp"
#include "../../src/ting/Socket.hpp"

#include "dns.hpp"
#include "socket.hpp"


int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<< "Socket test " << std::endl)

	ting::net::Lib netLib;
	
	BasicIPAddressTest::Run();
	TestIPAddress::Run();

	TestSimpleDNSLookup::Run();
	TestRequestFromCallback::Run();
		
	BasicClientServerTest::Run();
	BasicUDPSocketsTest::Run();
	TestUDPSocketWaitForWriting::Run();
	SendDataContinuouslyWithWaitSet::Run();
	SendDataContinuously::Run();

	TRACE_ALWAYS(<< "[PASSED]: Socket test" << std::endl)

	return 0;
}
