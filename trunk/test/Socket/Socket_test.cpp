#include <ting/Thread.hpp>
#include <ting/Socket.hpp>



namespace BasicClientServerTest{

class ServerThread : public ting::MsgThread{
public:
	//override
	void Run(){
		try{
			ting::TCPServerSocket listenSock;

			listenSock.Open(13666);//start listening on 80th port

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
		ting::IPAddress ip("127.0.0.1", 13666);//we will connect to localhost 80th port

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





int main(int argc, char *argv[]){
//	TRACE_ALWAYS(<<"Socket test "<<std::endl)

	ting::SocketLib socketsLib;

	BasicClientServerTest::Run();

	TRACE_ALWAYS(<<"[PASSED]: Socket test"<<std::endl)

	return 0;
}
