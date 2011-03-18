#include <ting/Timer.hpp>
#include <ting/Thread.hpp>
#include <ting/Socket.hpp>
#include <ting/WaitSet.hpp>
#include <ting/Buffer.hpp>
#include <ting/Array.hpp>



namespace BasicClientServerTest{

class ServerThread : public ting::MsgThread{
public:
	//override
	void Run(){
		try{
			ting::TCPServerSocket listenSock;

			listenSock.Open(13666);//start listening

			//Accept some connection
			ting::TCPSocket sock;
			while(!sock.IsValid() && !this->quitFlag){
				sock = listenSock.Accept();
				ting::Thread::Sleep(100);
				if(ting::Ptr<ting::Message> m = this->queue.PeekMsg()){
					m->Handle();
				}
			}

			ting::StaticBuffer<ting::u8, 4> data;
			data[0] = '0';
			data[1] = '1';
			data[2] = '2';
			data[3] = '4';
			sock.SendAll(data);
		}catch(ting::Socket::Exc &e){
			ASSERT_INFO_ALWAYS(false, "Network error: " << e.What())
		}
	}
};



void Run(){
	ServerThread serverThread;
	
	serverThread.Start();
	
	ting::Thread::Sleep(1000);
	
	try{
		ting::IPAddress ip("127.0.0.1", 13666);

		ting::TCPSocket sock;

		sock.Open(ip);

		ASSERT_ALWAYS(sock.IsValid())
		
		ting::StaticBuffer<ting::u8, 4> data;
		unsigned bytesReceived = 0;
		while(bytesReceived < 4){
			ASSERT_ALWAYS(bytesReceived < 4)
			bytesReceived += sock.Recv(data, bytesReceived);
			ASSERT_ALWAYS(bytesReceived <= 4)
			ting::Thread::Sleep(100);
		}
		
		ASSERT_ALWAYS(data[0] == '0')
		ASSERT_ALWAYS(data[1] == '1')
		ASSERT_ALWAYS(data[2] == '2')
		ASSERT_ALWAYS(data[3] == '4')
	}catch(ting::Socket::Exc &e){
		ASSERT_INFO_ALWAYS(false, "Network error: " << e.What())
	}
	
	serverThread.Join();
}

}//~namespace



namespace SendDataContinuously{

void Run(){
	ting::TCPServerSocket serverSock;

	serverSock.Open(13666);


	ting::TCPSocket sockS;
	{
		ting::IPAddress ip("127.0.0.1", 13666);
		sockS.Open(ip);
	}

	//Accept some connection
	TRACE(<< "SendDataContinuously::Run(): accepting connection" << std::endl)
	ting::TCPSocket sockR;
	for(unsigned i = 0; i < 20 && sockR.IsNotValid(); ++i){
		ting::Thread::Sleep(100);
		sockR = serverSock.Accept();
	}

	ASSERT_ALWAYS(sockS.IsValid())
	ASSERT_ALWAYS(sockR.IsValid())

	//Here we have 2 sockets sockS and sockR

	{
		ting::IPAddress addrS = sockS.GetRemoteAddress();
		ting::IPAddress addrR = sockR.GetRemoteAddress();
		TRACE(<< "SendDataContinuously::Run(): addrS = " << std::hex << addrS.host << ":" << addrS.port << std::endl)
		TRACE(<< "SendDataContinuously::Run(): addrR = " << std::hex << addrR.host << ":" << addrR.port << std::endl)
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


	ting::u32 startTime = ting::GetTicks();
	
	while(ting::GetTicks() - startTime < 10000){
		ting::StaticBuffer<ting::Waitable*, 2> triggered;

		unsigned numTriggered = ws.WaitWithTimeout(1000, &triggered);
//		unsigned numTriggered = ws.Wait(&triggered);

		ASSERT_ALWAYS(numTriggered <= 2)

		if(numTriggered == 0){
			TRACE(<< "SendDataContinuously::Run(): 0 triggered" << std::endl)
			continue;
		}

		//If 2 waitables have triggered they should be 2 different waitables.
		if(numTriggered == 2){
			TRACE(<< "SendDataContinuously::Run(): 2 triggered" << std::endl)
			ASSERT_ALWAYS(triggered[0] != triggered[1])
		}else{
			ASSERT_ALWAYS(numTriggered == 1)
			TRACE(<< "SendDataContinuously::Run(): 1 triggered" << std::endl)
		}

		for(unsigned i = 0; i < numTriggered; ++i){
			if(triggered[i] == &sockS){
				ASSERT_ALWAYS(triggered[i] != &sockR)

				TRACE(<< "SendDataContinuously::Run(): sockS triggered" << std::endl)
				ASSERT_ALWAYS(!sockS.CanRead())
				ASSERT_ALWAYS(!sockS.ErrorCondition())
				ASSERT_ALWAYS(sockS.CanWrite())

				ASSERT_ALWAYS(bytesSent <= sendBuffer.Size())

				if(sendBuffer.Size() == bytesSent){
					ting::Array<ting::u8> buf(0xffff + 1);
					STATIC_ASSERT(sizeof(ting::u32) == 4)
					ASSERT_INFO_ALWAYS((buf.Size() % sizeof(ting::u32)) == 0, "buf.Size() = " << buf.Size() << " (buf.Size() % sizeof(ting::u32)) = " << (buf.Size() % sizeof(ting::u32)))

					ting::u8* p = buf.Begin();
					for(; p != buf.End(); p += sizeof(ting::u32)){
						ASSERT_INFO_ALWAYS(p < (buf.End() - (sizeof(ting::u32) - 1)), "p = " << p << " buf.End() = " << buf.End())
						ting::Serialize32(scnt, p);
						++scnt;
					}
					ASSERT_ALWAYS(p == buf.End())
				}

				try{
					unsigned res = sockS.Send(sendBuffer, bytesSent);
					bytesSent += res;
					if(res == 0){
						TRACE(<< "SendDataContinuously::Run(): 0 bytes sent" << std::endl)
						//ting::Thread::Sleep(2000);
						ASSERT_ALWAYS(res > 0) //since it was CanWrite() we should be able to write at least something
					}else{
						//ting::Thread::Sleep(200);
						TRACE(<< "SendDataContinuously::Run(): " << res << " bytes sent" << std::endl)
					}
					ASSERT_ALWAYS(!sockS.CanWrite())
				}catch(ting::Socket::Exc& e){
					ASSERT_INFO_ALWAYS(false, "sockS.Send() failed: " << e.What())
				}
				ASSERT_ALWAYS(bytesSent <= sendBuffer.Size())
			}else if(triggered[i] == &sockR){
				ASSERT_ALWAYS(triggered[i] != &sockS)

				TRACE(<< "SendDataContinuously::Run(): sockR triggered" << std::endl)
				ASSERT_ALWAYS(sockR.CanRead())
				ASSERT_ALWAYS(!sockR.ErrorCondition())
				ASSERT_ALWAYS(!sockR.CanWrite())

				while(true){
					ting::StaticBuffer<ting::u8, 0x2000> buf; //8kb buffer
					unsigned numBytesReceived;
					try{
						numBytesReceived = sockR.Recv(buf);
					}catch(ting::Socket::Exc& e){
						ASSERT_INFO_ALWAYS(false, "sockR.Recv() failed: " << e.What())
					}
					ASSERT_ALWAYS(numBytesReceived <= buf.Size())

					if(numBytesReceived == 0){
						break;//~while(true)
					}

					for(const ting::u8* p = buf.Begin(); p != buf.End(); ++p){
						recvBuffer[recvBufBytes] = *p;
						++recvBufBytes;

						ASSERT_ALWAYS(recvBufBytes <= recvBuffer.Size())

						if(recvBufBytes == recvBuffer.Size()){
							recvBufBytes = 0;
							ting::u32 num = ting::Deserialize32(recvBuffer.Begin());
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

	ASSERT_ALWAYS(rcnt > 0) //check that at least anything was received
	ASSERT_ALWAYS(scnt > 0) //check that at least anything was sent
}

}//~namespace





int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<<"Socket test "<<std::endl)

	ting::SocketLib socketsLib;

	BasicClientServerTest::Run();
	SendDataContinuously::Run();

	TRACE_ALWAYS(<<"[PASSED]: Socket test"<<std::endl)

	return 0;
}
