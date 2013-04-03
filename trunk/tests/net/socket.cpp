#include "../../src/ting/timer.hpp"
#include "../../src/ting/mt/Thread.hpp"
#include "../../src/ting/mt/MsgThread.hpp"
#include "../../src/ting/mt/Message.hpp"
#include "../../src/ting/net/TCPSocket.hpp"
#include "../../src/ting/net/TCPServerSocket.hpp"
#include "../../src/ting/net/UDPSocket.hpp"
#include "../../src/ting/Ptr.hpp"
#include "../../src/ting/WaitSet.hpp"
#include "../../src/ting/Buffer.hpp"
#include "../../src/ting/Array.hpp"
#include "../../src/ting/config.hpp"
#include "../../src/ting/util.hpp"

#include "socket.hpp"



namespace BasicClientServerTest{

void SendAll(ting::net::TCPSocket& s, const ting::Buffer<ting::u8>& buf){
	if(!s.IsValid()){
		throw ting::net::Exc("TCPSocket::Send(): socket is not opened");
	}

	DEBUG_CODE(int left = int(buf.Size());)
	ASSERT(left >= 0)

	size_t offset = 0;

	while(true){
		int res = s.Send(buf, offset);
		DEBUG_CODE(left -= res;)
		ASSERT(left >= 0)
		offset += res;
		if(offset == buf.Size()){
				break;
		}
		//give 30ms to allow data from send buffer to be sent
		ting::mt::Thread::Sleep(30);
	}

	ASSERT(left == 0)
}


class ServerThread : public ting::mt::MsgThread{
public:
	//override
	void Run(){
		try{
			ting::net::TCPServerSocket listenSock;

			listenSock.Open(13666);//start listening

			ASSERT_ALWAYS(listenSock.GetLocalPort() == 13666)

			//Accept some connection
			ting::net::TCPSocket sock;
			while(!sock.IsValid() && !this->quitFlag){
				sock = listenSock.Accept();
				ting::mt::Thread::Sleep(100);
				if(ting::Ptr<ting::mt::Message> m = this->queue.PeekMsg()){
					m->Handle();
				}
			}

			ASSERT_ALWAYS(sock.IsValid())

			ASSERT_ALWAYS(sock.GetLocalAddress().host.IPv4Host() == 0x7f000001)
			ASSERT_ALWAYS(sock.GetRemoteAddress().host.IPv4Host() == 0x7f000001)

			ting::StaticBuffer<ting::u8, 4> data;
			data[0] = '0';
			data[1] = '1';
			data[2] = '2';
			data[3] = '4';
			SendAll(sock, data);
		}catch(ting::net::Exc &e){
			ASSERT_INFO_ALWAYS(false, "Network error: " << e.What())
		}
	}
};



void Run(){
	ServerThread serverThread;
	
	serverThread.Start();
	
	ting::mt::Thread::Sleep(1000);
	
	try{
		ting::net::IPAddress ip("127.0.0.1", 13666);

		ting::net::TCPSocket sock;

		sock.Open(ip);

		ASSERT_ALWAYS(sock.IsValid())

		ASSERT_ALWAYS(sock.GetLocalAddress().host.IPv4Host() == 0x7f000001)
		ASSERT_ALWAYS(sock.GetRemoteAddress().host.IPv4Host() == 0x7f000001)

		ting::StaticBuffer<ting::u8, 4> data;
		unsigned bytesReceived = 0;
		for(unsigned i = 0; i < 30; ++i){
			ASSERT_ALWAYS(bytesReceived < 4)
			bytesReceived += sock.Recv(data, bytesReceived);
			ASSERT_ALWAYS(bytesReceived <= 4)
			if(bytesReceived == 4)
				break;

			ting::mt::Thread::Sleep(100);
		}
		ASSERT_ALWAYS(bytesReceived == 4)
		
		ASSERT_ALWAYS(data[0] == '0')
		ASSERT_ALWAYS(data[1] == '1')
		ASSERT_ALWAYS(data[2] == '2')
		ASSERT_ALWAYS(data[3] == '4')
	}catch(ting::net::Exc &e){
		ASSERT_INFO_ALWAYS(false, "Network error: " << e.What())
	}
	
	serverThread.Join();
}

}//~namespace



namespace SendDataContinuouslyWithWaitSet{

void Run(){
	ting::net::TCPServerSocket serverSock;

	serverSock.Open(13666);


	ting::net::TCPSocket sockS;
	{
		ting::net::IPAddress ip("127.0.0.1", 13666);
		sockS.Open(ip);
	}

	//Accept connection
//	TRACE(<< "SendDataContinuously::Run(): accepting connection" << std::endl)
	ting::net::TCPSocket sockR;
	for(unsigned i = 0; i < 20 && sockR.IsNotValid(); ++i){
		ting::mt::Thread::Sleep(100);
		sockR = serverSock.Accept();
	}

	ASSERT_ALWAYS(sockS.IsValid())
	ASSERT_ALWAYS(sockR.IsValid())

	//Here we have 2 sockets sockS and sockR

	{
		ting::net::IPAddress addrS = sockS.GetRemoteAddress();
		ting::net::IPAddress addrR = sockR.GetRemoteAddress();
//		TRACE(<< "SendDataContinuously::Run(): addrS = " << std::hex << addrS.host << ":" << addrS.port << std::dec << std::endl)
//		TRACE(<< "SendDataContinuously::Run(): addrR = " << std::hex << addrR.host << ":" << addrR.port << std::dec << std::endl)
		ASSERT_ALWAYS(addrS.host.IPv4Host() == 0x7f000001) //check that IP is 127.0.0.1
		ASSERT_ALWAYS(addrR.host.IPv4Host() == 0x7f000001) //check that IP is 127.0.0.1
	}

	ting::WaitSet ws(2);
	ws.Add(&sockR, ting::Waitable::READ);
	ws.Add(&sockS, ting::Waitable::WRITE);


	ting::u32 scnt = 0;
	ting::Array<ting::u8> sendBuffer;
	unsigned bytesSent = 0;

	ting::u32 rcnt = 0;
	ting::StaticBuffer<ting::u8, sizeof(ting::u32)> recvBuffer;
	unsigned recvBufBytes = 0;


	ting::u32 startTime = ting::timer::GetTicks();
	
	while(ting::timer::GetTicks() - startTime < 5000){ //5 seconds
		ting::StaticBuffer<ting::Waitable*, 2> triggered;

		unsigned numTriggered = ws.WaitWithTimeout(1000, &triggered);
//		unsigned numTriggered = ws.Wait(&triggered);

		ASSERT_ALWAYS(numTriggered <= 2)

		if(numTriggered == 0){
//			TRACE(<< "SendDataContinuously::Run(): 0 triggered" << std::endl)
			continue;
		}

		//If 2 waitables have triggered they should be 2 different waitables.
		if(numTriggered == 2){
//			TRACE(<< "SendDataContinuously::Run(): 2 triggered" << std::endl)
			ASSERT_ALWAYS(triggered[0] != triggered[1])
		}else{
			ASSERT_ALWAYS(numTriggered == 1)
//			TRACE(<< "SendDataContinuously::Run(): 1 triggered" << std::endl)
		}

		for(unsigned i = 0; i < numTriggered; ++i){
			if(triggered[i] == &sockS){
				ASSERT_ALWAYS(triggered[i] != &sockR)

//				TRACE(<< "SendDataContinuously::Run(): sockS triggered" << std::endl)
				ASSERT_ALWAYS(!sockS.CanRead())
				ASSERT_ALWAYS(!sockS.ErrorCondition())
				ASSERT_ALWAYS(sockS.CanWrite())

				ASSERT_ALWAYS(bytesSent <= sendBuffer.Size())

				if(sendBuffer.Size() == bytesSent){
					sendBuffer.Init(0xffff + 1);
					bytesSent = 0;
					
					STATIC_ASSERT(sizeof(ting::u32) == 4)
					ASSERT_INFO_ALWAYS((sendBuffer.Size() % sizeof(ting::u32)) == 0,
							"sendBuffer.Size() = " << sendBuffer.Size()
							<< " (sendBuffer.Size() % sizeof(ting::u32)) = "
							<< (sendBuffer.Size() % sizeof(ting::u32))
						)

					ting::u8* p = sendBuffer.Begin();
					for(; p != sendBuffer.End(); p += sizeof(ting::u32)){
						ASSERT_INFO_ALWAYS(p < (sendBuffer.End() - (sizeof(ting::u32) - 1)), "p = " << p << " sendBuffer.End() = " << sendBuffer.End())
						ting::util::Serialize32LE(scnt, p);
						++scnt;
					}
					ASSERT_ALWAYS(p == sendBuffer.End())
				}

				ASSERT_ALWAYS(sendBuffer.Size() > 0)

				try{
					unsigned res = sockS.Send(sendBuffer, bytesSent);
					bytesSent += res;
					if(res == 0){
						ASSERT_ALWAYS(res > 0) //since it was CanWrite() we should be able to write at least something
					}else{
//						TRACE(<< "SendDataContinuously::Run(): " << res << " bytes sent" << std::endl)
					}
					ASSERT_ALWAYS(!sockS.CanWrite())
				}catch(ting::net::Exc& e){
					ASSERT_INFO_ALWAYS(false, "sockS.Send() failed: " << e.What())
				}
				ASSERT_ALWAYS(bytesSent <= sendBuffer.Size())
			}else if(triggered[i] == &sockR){
				ASSERT_ALWAYS(triggered[i] != &sockS)

//				TRACE(<< "SendDataContinuously::Run(): sockR triggered" << std::endl)
				ASSERT_ALWAYS(sockR.CanRead())
				ASSERT_ALWAYS(!sockR.ErrorCondition())
				ASSERT_ALWAYS(!sockR.CanWrite())

				while(true){
					ting::StaticBuffer<ting::u8, 0x2000> buf; //8kb buffer
					unsigned numBytesReceived;
					try{
						numBytesReceived = sockR.Recv(buf);
					}catch(ting::net::Exc& e){
						ASSERT_INFO_ALWAYS(false, "sockR.Recv() failed: " << e.What())
					}
					ASSERT_ALWAYS(numBytesReceived <= buf.Size())
//					TRACE(<< "SendDataContinuously::Run(): " << numBytesReceived << " bytes received" << std::endl)

					if(numBytesReceived == 0){
						break;//~while(true)
					}

					const ting::u8* p = buf.Begin();
					for(unsigned i = 0; i < numBytesReceived && p != buf.End(); ++p, ++i){
						recvBuffer[recvBufBytes] = *p;
						++recvBufBytes;

						ASSERT_ALWAYS(recvBufBytes <= recvBuffer.Size())

						if(recvBufBytes == recvBuffer.Size()){
							recvBufBytes = 0;
							ting::u32 num = ting::util::Deserialize32LE(recvBuffer.Begin());
							ASSERT_INFO_ALWAYS(
									rcnt == num,
									"num = " << num << " rcnt = " << rcnt
											<< " rcnt - num = " << (rcnt - num)
											<< " recvBuffer = "
											<< unsigned(recvBuffer[0]) << ", "
											<< unsigned(recvBuffer[1]) << ", "
											<< unsigned(recvBuffer[2]) << ", "
											<< unsigned(recvBuffer[3])
								)
							++rcnt;
						}
					}//~for
				}//~while(true)
			}else{
				ASSERT_ALWAYS(false)
			}
		}//~for(triggered)
	}//~while

	ws.Remove(&sockS);
	ws.Remove(&sockR);
}

}//~namespace



namespace SendDataContinuously{

void Run(){
	ting::net::TCPServerSocket serverSock;

	serverSock.Open(13666);


	ting::net::TCPSocket sockS;
	{
		ting::net::IPAddress ip("127.0.0.1", 13666);
		sockS.Open(ip);
	}

	//Accept connection
//	TRACE(<< "SendDataContinuously::Run(): accepting connection" << std::endl)
	ting::net::TCPSocket sockR;
	for(unsigned i = 0; i < 20 && sockR.IsNotValid(); ++i){
		ting::mt::Thread::Sleep(100);
		sockR = serverSock.Accept();
	}

	ASSERT_ALWAYS(sockS.IsValid())
	ASSERT_ALWAYS(sockR.IsValid())

	//Here we have 2 sockets sockS and sockR

	ting::u8 scnt = 0;

	ting::u8 rcnt = 0;


	ting::u32 startTime = ting::timer::GetTicks();

	while(ting::timer::GetTicks() - startTime < 5000){ //5 seconds

		//SEND

		try{
			ting::Buffer<ting::u8> buf(&scnt, 1);
			unsigned res = sockS.Send(buf);
			ASSERT_ALWAYS(res <= 1)
			if(res == 1){
				++scnt;
			}else{
				ASSERT_ALWAYS(false)
			}
		}catch(ting::net::Exc& e){
			ASSERT_INFO_ALWAYS(false, "sockS.Send() failed: " << e.What())
		}


		//READ

		while(true){
			ting::StaticBuffer<ting::u8, 0x2000> buf; //8kb buffer
			unsigned numBytesReceived;
			try{
				numBytesReceived = sockR.Recv(buf);
			}catch(ting::net::Exc& e){
				ASSERT_INFO_ALWAYS(false, "sockR.Recv() failed: " << e.What())
			}
			ASSERT_ALWAYS(numBytesReceived <= buf.Size())
//			TRACE(<< "SendDataContinuously::Run(): " << numBytesReceived << " bytes received" << std::endl)

			if(numBytesReceived == 0){
				break;//~while(true)
			}

			const ting::u8* p = buf.Begin();
			for(unsigned i = 0; i < numBytesReceived && p != buf.End(); ++p, ++i){
				ASSERT_INFO_ALWAYS(rcnt == *p, "rcnt = " << unsigned(rcnt) << " *p = " << unsigned(*p) << " diff = " << unsigned(rcnt - *p))
				++rcnt;
			}//~for
		}//~while(true)
		
	}//~while
}

}//~namespace



namespace BasicIPAddressTest{

void Run(){
	{
		try{
			ting::net::IPAddress a("123.124.125.126", 5);
			ASSERT_ALWAYS(a.host.IPv4Host() == (123 << 24) + (124 << 16) + (125 << 8) + 126)
			ASSERT_ALWAYS(a.port == 5)
		}catch(std::exception& e){
			ASSERT_INFO_ALWAYS(false, e.what())
		}
	}

	{
		ting::net::IPAddress a(123, 124, 125, 126, 5);
		ASSERT_ALWAYS(a.host.IPv4Host() == (123 << 24) + (124 << 16) + (125 << 8) + 126)
		ASSERT_ALWAYS(a.port == 5)
	}

	//test copy constructor and operator=()
	{
		ting::net::IPAddress a(123, 124, 125, 126, 5);
		ASSERT_ALWAYS(a.host.IPv4Host() == (123 << 24) + (124 << 16) + (125 << 8) + 126)
		ASSERT_ALWAYS(a.port == 5)

		ting::net::IPAddress a1(a);
		ASSERT_ALWAYS(a1.host.IPv4Host() == (123 << 24) + (124 << 16) + (125 << 8) + 126)
		ASSERT_ALWAYS(a1.port == 5)

		ting::net::IPAddress a2;
		a2 = a1;
		ASSERT_ALWAYS(a2.host.IPv4Host() == (123 << 24) + (124 << 16) + (125 << 8) + 126)
		ASSERT_ALWAYS(a2.port == 5)

		ASSERT_ALWAYS(a == a1)
		ASSERT_ALWAYS(a == a2)
	}
}

}//~namespace



namespace BasicUDPSocketsTest{

void Run(){

	ting::net::UDPSocket recvSock;

	try{
		recvSock.Open(13666);
	}catch(ting::net::Exc &e){
		ASSERT_INFO_ALWAYS(false, e.What())
	}

	ASSERT_ALWAYS(recvSock.GetLocalPort() == 13666)

	ting::net::UDPSocket sendSock;

	try{
		sendSock.Open();

		ting::StaticBuffer<ting::u8, 4> data;
		data[0] = '0';
		data[1] = '1';
		data[2] = '2';
		data[3] = '4';
		unsigned bytesSent = 0;

		ting::net::IPAddress addr("127.0.0.1", 13666);
		ASSERT_ALWAYS(addr.host.IPv4Host() == 0x7f000001)

		for(unsigned i = 0; i < 10; ++i){
			bytesSent = sendSock.Send(data, addr);
			ASSERT_ALWAYS(bytesSent == 4 || bytesSent == 0)
			if(bytesSent == 4)
				break;
			
			ting::mt::Thread::Sleep(100);
		}
		ASSERT_ALWAYS(bytesSent == 4)
	}catch(ting::net::Exc &e){
		ASSERT_INFO_ALWAYS(false, e.What())
	}

	try{
		ting::StaticBuffer<ting::u8, 1024> buf;
		
		unsigned bytesReceived = 0;
		for(unsigned i = 0; i < 10; ++i){
			ting::net::IPAddress ip;
			bytesReceived = recvSock.Recv(buf, ip);
			ASSERT_ALWAYS(bytesReceived == 0 || bytesReceived == 4)//all or nothing
			if(bytesReceived == 4){
				ASSERT_INFO_ALWAYS(ip.host.IPv4Host() == 0x7f000001, "ip.host.IPv4Host() = " << std::hex << ip.host.IPv4Host() << std::dec)
				break;
			}
			
			ting::mt::Thread::Sleep(100);
		}
		ASSERT_ALWAYS(bytesReceived == 4)
		ASSERT_ALWAYS(buf[0] == '0')
		ASSERT_ALWAYS(buf[1] == '1')
		ASSERT_ALWAYS(buf[2] == '2')
		ASSERT_ALWAYS(buf[3] == '4')
	}catch(ting::net::Exc& e){
		ASSERT_INFO_ALWAYS(false, e.What())
	}
}

}//~namespace



namespace TestUDPSocketWaitForWriting{

void Run(){

	ting::net::UDPSocket sendSock;

	try{
		sendSock.Open();

		ting::WaitSet ws(1);
		
		ws.Add(&sendSock, ting::Waitable::READ_AND_WRITE);
		
		if(ws.WaitWithTimeout(3000, 0) == 0){
			//if timeout was hit
//NOTE: for some reason waiting for writing to UDP socket does not work on Win32 (aaarrrggghh).
#if M_OS != M_OS_WINDOWS
			ASSERT_ALWAYS(false)
#endif
		}else{
			ASSERT_ALWAYS(sendSock.CanWrite())
			ASSERT_ALWAYS(!sendSock.CanRead())
		}
		
		ws.Remove(&sendSock);
	}catch(ting::net::Exc &e){
		ASSERT_INFO_ALWAYS(false, e.What())
	}

}

}//~namespace



namespace TestIPAddress{

void Run(){
	try{//test IP-address without port string parsing
		try{//test correct string
			ting::net::IPAddress ip("127.0.0.1", 80);
			ASSERT_ALWAYS(ip.host.IPv4Host() == 0x7f000001)
			ASSERT_ALWAYS(ip.port == 80)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			ASSERT_ALWAYS(false)
		}
		
		try{//test correct string
			ting::net::IPAddress ip("127.0.0.1:23ddqwd", 80);
			ASSERT_ALWAYS(ip.host.IPv4Host() == 0x7f000001)
			ASSERT_ALWAYS(ip.port == 80)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			ASSERT_ALWAYS(true)
		}
		try{//test correct string
			ting::net::IPAddress ip("127.0.0.2555:23ddqwd", 80);
			ASSERT_ALWAYS(ip.host.IPv4Host() == 0x7f0000ff)
			ASSERT_ALWAYS(ip.port == 80)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			ASSERT_ALWAYS(true)
		}
		
		try{//test incorrect string
			ting::net::IPAddress ip("127.0.1803:65536");
			ASSERT_ALWAYS(false)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			//should get here
		}catch(...){
			ASSERT_ALWAYS(false)
		}
		
		try{//test incorrect string
			ting::net::IPAddress ip("127.0.270.1:65536");
			ASSERT_ALWAYS(false)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			//should get here
		}catch(...){
			ASSERT_ALWAYS(false)
		}
	}catch(...){
		ASSERT_ALWAYS(false)
	}
	
	try{//test IP-address with port string parsing
		try{//test correct string
			ting::net::IPAddress ip("127.0.0.1:80");
			ASSERT_ALWAYS(ip.host.IPv4Host() == 0x7f000001)
			ASSERT_ALWAYS(ip.port == 80)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			ASSERT_ALWAYS(false)
		}
		
		try{//test incorrect string
			ting::net::IPAddress ip("127.0.0.1803:43");
			ASSERT_ALWAYS(false)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			//should get here
		}catch(...){
			ASSERT_ALWAYS(false)
		}
		
		try{//test incorrect string
			ting::net::IPAddress ip("127.0.0.180p43");
			ASSERT_ALWAYS(false)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			//should get here
		}catch(...){
			ASSERT_ALWAYS(false)
		}
		
		try{//test incorrect string
			ting::net::IPAddress ip("127.0.0.180:123456");
			ASSERT_ALWAYS(false)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			//should get here
		}catch(...){
			ASSERT_ALWAYS(false)
		}
		
		try{//test incorrect string
			ting::net::IPAddress ip("127.0.0.180:72345");
			ASSERT_ALWAYS(false)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			//should get here
		}catch(...){
			ASSERT_ALWAYS(false)
		}
		
		try{//test incorrect string
			ting::net::IPAddress ip("127.0.0.1803:65536");
			ASSERT_ALWAYS(false)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			//should get here
		}catch(...){
			ASSERT_ALWAYS(false)
		}
		
		try{//test correct string
			ting::net::IPAddress ip("127.0.0.1:65535");
			ASSERT_ALWAYS(ip.host.IPv4Host() == 0x7f000001)
			ASSERT_ALWAYS(ip.port == 0xffff)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			ASSERT_ALWAYS(false)
		}
		
		try{//test correct string
			ting::net::IPAddress ip("127.0.0.1:0");
			ASSERT_ALWAYS(ip.host.IPv4Host() == 0x7f000001)
			ASSERT_ALWAYS(ip.port == 0)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			ASSERT_ALWAYS(false)
		}
		
		try{//test correct string
			ting::net::IPAddress ip("127.0.0.1:6535 ");
			ASSERT_ALWAYS(ip.host.IPv4Host() == 0x7f000001)
			ASSERT_ALWAYS(ip.port == 6535)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			ASSERT_ALWAYS(false)
		}
		
		try{//test correct string
			ting::net::IPAddress ip("127.0.0.1:6535dwqd 345");
			ASSERT_ALWAYS(ip.host.IPv4Host() == 0x7f000001)
			ASSERT_ALWAYS(ip.port == 6535)
		}catch(ting::net::IPAddress::BadIPAddressFormatExc& e){
			ASSERT_ALWAYS(false)
		}
	}catch(...){
		ASSERT_ALWAYS(false)
	}
	
	//Test IPv6
	
	try{
		ting::net::IPAddress ip("1002:3004:5006::7008:900a");
		TRACE(<< std::hex << ip.host.IPv4Host() << std::endl)
	}catch(...){
		ASSERT_ALWAYS(false)
	}
}

}//~namespace
