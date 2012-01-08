/* The MIT License:

Copyright (c) 2009-2011 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// Homepage: http://ting.googlecode.com



#include "Socket.hpp"



using namespace ting;



SocketLib::SocketLib(){
#ifdef WIN32
	WORD versionWanted = MAKEWORD(2,2);
	WSADATA wsaData;
	if(WSAStartup(versionWanted, &wsaData) != 0 ){
		throw Socket::Exc("SocketLib::SocketLib(): Winsock 2.2 initialization failed");
	}
#else //assume linux/unix
	// SIGPIPE is generated when a remote socket is closed
	void (*handler)(int);
	handler = signal(SIGPIPE, SIG_IGN);
	if(handler != SIG_DFL){
		signal(SIGPIPE, handler);
	}
#endif
}



SocketLib::~SocketLib(){
#ifdef WIN32
	// Clean up windows networking
	if(WSACleanup() == SOCKET_ERROR)
		if(WSAGetLastError() == WSAEINPROGRESS){
			WSACancelBlockingCall();
			WSACleanup();
		}
#else //assume linux/unix
	// Restore the SIGPIPE handler
	void (*handler)(int);
	handler = signal(SIGPIPE, SIG_DFL);
	if(handler != SIG_IGN){
		signal(SIGPIPE, handler);
	}
#endif
}



IPAddress SocketLib::GetHostByName(const char *hostName, u16 port){
	if(!hostName)
		throw Socket::Exc("SocketLib::GetHostByName(): pointer passed as argument is 0");

	IPAddress addr;
	addr.host = inet_addr(hostName);
	if(addr.host == INADDR_NONE){
		struct hostent *hp;
		hp = gethostbyname(hostName);
		if(hp){
			memcpy(&(addr.host), hp->h_addr, sizeof(addr.host)/* hp->h_length */);
		}else{
			throw Socket::Exc("SocketLib::GetHostByName(): gethostbyname() failed");
		}
	}
	addr.port = port;
	return addr;
}



Socket::Socket(const Socket& s) :
		ting::Waitable(),
		//NOTE: operator=() will call Close, so the socket should be in invalid state!!!
		//Therefore, initialize variables to invalid values.
#ifdef WIN32
		eventForWaitable(WSA_INVALID_EVENT),
#endif
		socket(DInvalidSocket())
{
//		TRACE(<< "Socket::Socket(copy): invoked " << this << std::endl)
	this->operator=(s);
}



Socket::~Socket(){
	this->Close();
}



void Socket::Close(){
//		TRACE(<< "Socket::Close(): invoked " << this << std::endl)
	if(this->IsValid()){
		ASSERT(!this->IsAdded()) //make sure the socket is not added to WaitSet

#ifdef WIN32
		//Closing socket in Win32.
		//refer to http://tangentsoft.net/wskfaq/newbie.html#howclose for details
		shutdown(this->socket, SD_BOTH);
		closesocket(this->socket);

		this->CloseEventForWaitable();
#else //assume linux/unix
		close(this->socket);
#endif
	}
	this->ClearAllReadinessFlags();
	this->socket = DInvalidSocket();
}



//same as std::auto_ptr
Socket& Socket::operator=(const Socket& s){
//	TRACE(<< "Socket::operator=(): invoked " << this << std::endl)
	if(this == &s)//detect self-assignment
		return *this;

	//first, assign as Waitable, it may throw an exception
	//if the waitable is added to some waitset
	this->Waitable::operator=(s);

	this->Close();
	this->socket = s.socket;

#ifdef WIN32
	this->eventForWaitable = s.eventForWaitable;
	const_cast<Socket&>(s).eventForWaitable = WSA_INVALID_EVENT;
#endif

	const_cast<Socket&>(s).socket = DInvalidSocket();
	return *this;
}



void Socket::DisableNaggle(){
	if(!this->IsValid())
		throw Socket::Exc("Socket::DisableNaggle(): socket is not valid");

#if defined(__linux__) || defined(__APPLE__) || defined(WIN32)
	{
		int yes = 1;
		setsockopt(this->socket, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(yes));
	}
#else
#error "Unsupported OS"
#endif
}



void Socket::SetNonBlockingMode(){
	if(!this->IsValid())
		throw Socket::Exc("Socket::SetNonBlockingMode(): socket is not valid");

#if defined(__linux__) || defined(__APPLE__)
	{
		int flags = fcntl(this->socket, F_GETFL, 0);
		if(flags == -1){
			throw Socket::Exc("Socket::SetNonBlockingMode(): fcntl(F_GETFL) failed");
		}
		if(fcntl(this->socket, F_SETFL, flags | O_NONBLOCK) != 0){
			throw Socket::Exc("Socket::SetNonBlockingMode(): fcntl(F_SETFL) failed");
		}
	}
#elif defined(WIN32)
	{
		u_long mode = 1;
		if(ioctlsocket(this->socket, FIONBIO, &mode) != 0){
			throw Socket::Exc("Socket::SetNonBlockingMode(): ioctlsocket(FIONBIO) failed");
		}
	}
#else
#error "Unsupported OS"
#endif
}



u16 Socket::GetLocalPort(){
	if(!this->IsValid())
		throw Socket::Exc("Socket::GetLocalPort(): socket is not valid");

	sockaddr_in addr;

#ifdef WIN32
	int len = sizeof(addr);
#else//assume linux/unix
	socklen_t len = sizeof(addr);
#endif

	if(getsockname(
			this->socket,
			reinterpret_cast<sockaddr*>(&addr),
			&len
		) < 0)
	{
		throw Socket::Exc("Socket::GetLocalPort(): getsockname() failed");
	}

	return u16(ntohs(addr.sin_port));
}



#ifdef WIN32

//override
HANDLE Socket::GetHandle(){
	//return event handle
	return this->eventForWaitable;
}



//override
bool Socket::CheckSignalled(){
	WSANETWORKEVENTS events;
	memset(&events, 0, sizeof(events));
	ASSERT(this->IsValid())
	if(WSAEnumNetworkEvents(this->socket, this->eventForWaitable, &events) != 0){
		throw Socket::Exc("Socket::CheckSignalled(): WSAEnumNetworkEvents() failed");
	}

	//NOTE: sometimes no events are reported, don't know why.
//		ASSERT(events.lNetworkEvents != 0)

	if((events.lNetworkEvents & FD_CLOSE) != 0){
		this->SetErrorFlag();
	}

	if((events.lNetworkEvents & FD_READ) != 0){
		this->SetCanReadFlag();
		if(events.iErrorCode[ASSCOND(FD_READ_BIT, < FD_MAX_EVENTS)] != 0){
			this->SetErrorFlag();
		}
	}

	if((events.lNetworkEvents & FD_ACCEPT) != 0){
		this->SetCanReadFlag();
		if(events.iErrorCode[ASSCOND(FD_ACCEPT_BIT, < FD_MAX_EVENTS)] != 0){
			this->SetErrorFlag();
		}
	}

	if((events.lNetworkEvents & FD_WRITE) != 0){
		this->SetCanWriteFlag();
		if(events.iErrorCode[ASSCOND(FD_WRITE_BIT, < FD_MAX_EVENTS)] != 0){
			this->SetErrorFlag();
		}
	}

	if((events.lNetworkEvents & FD_CONNECT) != 0){
		this->SetCanWriteFlag();
		if(events.iErrorCode[ASSCOND(FD_CONNECT_BIT, < FD_MAX_EVENTS)] != 0){
			this->SetErrorFlag();
		}
	}

#ifdef DEBUG
	//if some event occured then some of readiness flags should be set
	if(events.lNetworkEvents != 0){
		ASSERT_ALWAYS(this->readinessFlags != 0)
	}
#endif

	return this->Waitable::CheckSignalled();
}



void Socket::CreateEventForWaitable(){
	ASSERT(this->eventForWaitable == WSA_INVALID_EVENT)
	this->eventForWaitable = WSACreateEvent();
	if(this->eventForWaitable == WSA_INVALID_EVENT){
		throw Socket::Exc("Socket::CreateEventForWaitable(): could not create event (Win32) for implementing Waitable");
	}
}



void Socket::CloseEventForWaitable(){
	ASSERT(this->eventForWaitable != WSA_INVALID_EVENT)
	WSACloseEvent(this->eventForWaitable);
	this->eventForWaitable = WSA_INVALID_EVENT;
}



void Socket::SetWaitingEventsForWindows(long flags){
	ASSERT_INFO(this->IsValid() && (this->eventForWaitable != WSA_INVALID_EVENT), "HINT: Most probably, you are trying to remove the _closed_ socket from WaitSet. If so, you should first remove the socket from WaitSet and only then call the Close() method.")

	if(WSAEventSelect(
			this->socket,
			this->eventForWaitable,
			flags
		) != 0)
	{
		throw Socket::Exc("Socket::SetWaitingEventsForWindows(): could not associate event (Win32) with socket");
	}
}



#elif defined(__linux__) || defined(__APPLE__)

//override
int Socket::GetHandle(){
	return this->socket;
}



#else
#error "unsupported OS"
#endif



//static
ting::u32 IPAddress::ParseString(const char* ip){
	//TODO: there already is an IP parsing function in BSD sockets, consider using it here
	if(!ip)
		throw Socket::Exc("IPAddress::ParseString(): pointer passed as argument is 0");

	struct lf{ //local functions
		inline static void ThrowInvalidIP(){
			throw Socket::Exc("IPAddress::ParseString(): string is not a valid IP address");
		}
	};
	
	u32 h = 0;//parsed host
	const char *curp = ip;
	for(unsigned t = 0; t < 4; ++t){
		unsigned digits[3];
		unsigned numDgts;
		for(numDgts = 0; numDgts < 3; ++numDgts){
			if( *curp == '.' || *curp == 0 ){
				if(numDgts==0)
					lf::ThrowInvalidIP();
				break;
			}else{
				if(*curp < '0' || *curp > '9')
					lf::ThrowInvalidIP();
				digits[numDgts] = unsigned(*curp) - unsigned('0');
			}
			++curp;
		}

		if(t < 3 && *curp != '.')//unexpected delimiter or unexpected end of string
			lf::ThrowInvalidIP();
		else if(t == 3 && *curp != 0)
			lf::ThrowInvalidIP();

		unsigned xxx = 0;
		for(unsigned i = 0; i < numDgts; ++i){
			unsigned ord = 1;
			for(unsigned j = 1; j < numDgts - i; ++j)
			   ord *= 10;
			xxx += digits[i] * ord;
		}
		if(xxx > 255)
			lf::ThrowInvalidIP();

		h |= (xxx << (8 * (3 - t)));

		++curp;
	}
	return h;
}



void TCPSocket::Open(const IPAddress& ip, bool disableNaggle){
	if(this->IsValid())
		throw Socket::Exc("TCPSocket::Open(): socket already opened");

	//create event for implementing Waitable
#ifdef WIN32
	this->CreateEventForWaitable();
#endif

	this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if(this->socket == DInvalidSocket()){
#ifdef WIN32
		this->CloseEventForWaitable();
#endif
		throw Socket::Exc("TCPSocket::Open(): Couldn't create socket");
	}

	//Disable Naggle algorithm if required
	if(disableNaggle)
		this->DisableNaggle();

	this->SetNonBlockingMode();

	this->ClearAllReadinessFlags();

	//Connecting to remote host
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = htonl(ip.host);
	sockAddr.sin_port = htons(ip.port);

	// Connect to the remote host
	if(connect(
			this->socket,
			reinterpret_cast<sockaddr *>(&sockAddr),
			sizeof(sockAddr)
		) == DSocketError())
	{
#ifdef WIN32
		int errorCode = WSAGetLastError();
#else //linux/unix
		int errorCode = errno;
#endif
		if(errorCode == DEIntr()){
			//do nothing, for non-blocking socket the connection request still should remain active
		}else if(errorCode == DEInProgress()){
			//do nothing, this is not an error, we have non-blocking socket
		}else{
			std::stringstream ss;
			ss << "TCPSocket::Open(): connect() failed, error code = " << errorCode << ": ";
#ifdef _MSC_VER //if MSVC compiler
			{
				const size_t msgbufSize = 0xff;
				char msgbuf[msgbufSize];
				strerror_s(msgbuf, msgbufSize, errorCode);
				msgbuf[msgbufSize - 1] = 0;//make sure the string is null-terminated
				ss << msgbuf;
			}
#else
			ss << strerror(errorCode);
#endif
			this->Close();
			throw Socket::Exc(ss.str());
		}
	}
}



size_t TCPSocket::Send(const ting::Buffer<u8>& buf, size_t offset){
	if(!this->IsValid()){
		throw Socket::Exc("TCPSocket::Send(): socket is not opened");
	}

	this->ClearCanWriteFlag();

	ASSERT_INFO((
			(buf.Begin() + offset) <= (buf.End() - 1)) ||
			((buf.Size() == 0) && (offset == 0))
		,
			"buf.Begin() = " << reinterpret_cast<const void*>(buf.Begin())
			<< " offset = " << offset
			<< " buf.End() = " << reinterpret_cast<const void*>(buf.End())
		)

	ssize_t len;

	while(true){
		len = send(
				this->socket,
				reinterpret_cast<const char*>(buf.Begin() + offset),
				buf.Size() - offset,
				0
			);
		if(len == DSocketError()){
#ifdef WIN32
			int errorCode = WSAGetLastError();
#else
			int errorCode = errno;
#endif
			if(errorCode == DEIntr()){
				continue;
			}else if(errorCode == DEAgain()){
				//can't send more bytes, return 0 bytes sent
				len = 0;
			}else{
				std::stringstream ss;
				ss << "TCPSocket::Send(): send() failed, error code = " << errorCode << ": ";
#ifdef _MSC_VER //if MSVC compiler
				{
					const size_t msgbufSize = 0xff;
					char msgbuf[msgbufSize];
					strerror_s(msgbuf, msgbufSize, errorCode);
					msgbuf[msgbufSize - 1] = 0;//make sure the string is null-terminated
					ss << msgbuf;
				}
#else
				ss << strerror(errorCode);
#endif
				throw Socket::Exc(ss.str());
			}
		}
		break;
	}//~while

	ASSERT(len >= 0)
	return size_t(len);
}



void TCPSocket::SendAll(const ting::Buffer<u8>& buf){
	if(!this->IsValid())
		throw Socket::Exc("TCPSocket::Send(): socket is not opened");

	DEBUG_CODE(int left = int(buf.Size());)
	ASSERT(left >= 0)

	size_t offset = 0;

	while(true){
		int res = this->Send(buf, offset);
		DEBUG_CODE(left -= res;)
		ASSERT(left >= 0)
		offset += res;
		if(offset == buf.Size()){
			break;
		}
		//give 30ms to allow data from send buffer to be sent
		Thread::Sleep(30);
	}

	ASSERT(left == 0)
}



size_t TCPSocket::Recv(ting::Buffer<u8>& buf, size_t offset){
	//the 'can read' flag shall be cleared even if this function fails to avoid subsequent
	//calls to Recv() because it indicates that there's activity.
	//So, do it at the beginning of the function.
	this->ClearCanReadFlag();

	if(!this->IsValid())
		throw Socket::Exc("TCPSocket::Send(): socket is not opened");

	ASSERT_INFO(
			((buf.Begin() + offset) <= (buf.End() - 1)) ||
			((buf.Size() == 0) && (offset == 0))
		,
			"buf.Begin() = " << reinterpret_cast<void*>(buf.Begin())
			<< " offset = " << offset
			<< " buf.End() = " << reinterpret_cast<void*>(buf.End())
		)

	ssize_t len;

	while(true){
		len = recv(
				this->socket,
				reinterpret_cast<char*>(buf.Begin() + offset),
				buf.Size() - offset,
				0
			);
		if(len == DSocketError()){
#ifdef WIN32
			int errorCode = WSAGetLastError();
#else
			int errorCode = errno;
#endif
			if(errorCode == DEIntr()){
				continue;
			}else if(errorCode == DEAgain()){
				//no data available, return 0 bytes received
				len = 0;
			}else{
				std::stringstream ss;
				ss << "TCPSocket::Recv(): recv() failed, error code = " << errorCode << ": ";
#ifdef _MSC_VER //if MSVC compiler
				{
					const size_t msgbufSize = 0xff;
					char msgbuf[msgbufSize];
					strerror_s(msgbuf, msgbufSize, errorCode);
					msgbuf[msgbufSize - 1] = 0;//make sure the string is null-terminated
					ss << msgbuf;
				}
#else
				ss << strerror(errorCode);
#endif
				throw Socket::Exc(ss.str());
			}
		}
		break;
	}//~while

	ASSERT(len >= 0)
	return size_t(len);
}



IPAddress TCPSocket::GetLocalAddress(){
	if(!this->IsValid())
		throw Socket::Exc("Socket::GetLocalPort(): socket is not valid");

	sockaddr_in addr;

#ifdef WIN32
	int len = sizeof(addr);
#else
	socklen_t len = sizeof(addr);
#endif

	if(getsockname(
			this->socket,
			reinterpret_cast<sockaddr*>(&addr),
			&len
		) < 0)
	{
		throw Socket::Exc("Socket::GetLocalPort(): getsockname() failed");
	}

	return IPAddress(
			u32(ntohl(addr.sin_addr.s_addr)),
			u16(ntohs(addr.sin_port))
		);
}



IPAddress TCPSocket::GetRemoteAddress(){
	if(!this->IsValid())
		throw Socket::Exc("TCPSocket::GetRemoteAddress(): socket is not valid");

	sockaddr_in addr;

#ifdef WIN32
	int len = sizeof(addr);
#else
	socklen_t len = sizeof(addr);
#endif

	if(getpeername(
			this->socket,
			reinterpret_cast<sockaddr*>(&addr),
			&len
		) < 0)
	{
		throw Socket::Exc("TCPSocket::GetRemoteAddress(): getpeername() failed");
	}

	return IPAddress(
			u32(ntohl(addr.sin_addr.s_addr)),
			u16(ntohs(addr.sin_port))
		);
}



#ifdef WIN32
//override
void TCPSocket::SetWaitingEvents(u32 flagsToWaitFor){
	long flags = FD_CLOSE;
	if((flagsToWaitFor & Waitable::READ) != 0){
		flags |= FD_READ;
		//NOTE: since it is not a TCPServerSocket, FD_ACCEPT is not needed here.
	}
	if((flagsToWaitFor & Waitable::WRITE) != 0){
		flags |= FD_WRITE | FD_CONNECT;
	}
	this->SetWaitingEventsForWindows(flags);
}
#endif



void TCPServerSocket::Open(u16 port, bool disableNaggle, u16 queueLength){
	if(this->IsValid())
		throw Socket::Exc("TCPServerSocket::Open(): socket already opened");

	this->disableNaggle = disableNaggle;

#ifdef WIN32
	this->CreateEventForWaitable();
#endif

	this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if(this->socket == DInvalidSocket()){
#ifdef WIN32
		this->CloseEventForWaitable();
#endif
		throw Socket::Exc("TCPServerSocket::Open(): Couldn't create socket");
	}

	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = INADDR_ANY;
	sockAddr.sin_port = htons(port);

	// allow local address reuse
	{
		int yes = 1;
		setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
	}

	// Bind the socket for listening
	if(bind(
			this->socket,
			reinterpret_cast<sockaddr*>(&sockAddr),
			sizeof(sockAddr)
		) == DSocketError())
	{
		this->Close();
		throw Socket::Exc("TCPServerSocket::Open(): Couldn't bind to local port");
	}

	if(listen(this->socket, int(queueLength)) == DSocketError()){
		this->Close();
		throw Socket::Exc("TCPServerSocket::Open(): Couldn't listen to local port");
	}

	this->SetNonBlockingMode();
}



TCPSocket TCPServerSocket::Accept(){
	if(!this->IsValid())
		throw Socket::Exc("TCPServerSocket::Accept(): the socket is not opened");

	this->ClearCanReadFlag();

	sockaddr_in sockAddr;

#ifdef WIN32
	int sock_alen = sizeof(sockAddr);
#else //linux/unix
	socklen_t sock_alen = sizeof(sockAddr);
#endif

	TCPSocket sock;//allocate a new socket object

	sock.socket = ::accept(
			this->socket,
			reinterpret_cast<sockaddr*>(&sockAddr),
#ifdef USE_GUSI_SOCKETS
			(unsigned int *)&sock_alen
#else
			&sock_alen
#endif
		);

	if(sock.socket == DInvalidSocket())
		return sock;//no connections to be accepted, return invalid socket

#ifdef WIN32
	sock.CreateEventForWaitable();

	//NOTE: accepted socket is associated with the same event object as the listening socket which accepted it.
	//Reassociate the socket with its own event object.
	sock.SetWaitingEvents(0);
#endif

	sock.SetNonBlockingMode();

	if(this->disableNaggle)
		sock.DisableNaggle();

	return sock;//return a newly created socket
}



#ifdef WIN32
//override
void TCPServerSocket::SetWaitingEvents(u32 flagsToWaitFor){
	if(flagsToWaitFor != 0 && flagsToWaitFor != Waitable::READ){
		throw ting::Exc("TCPServerSocket::SetWaitingEvents(): only Waitable::READ flag allowed");
	}

	long flags = FD_CLOSE;
	if((flagsToWaitFor & Waitable::READ) != 0){
		flags |= FD_ACCEPT;
	}
	this->SetWaitingEventsForWindows(flags);
}
#endif



void UDPSocket::Open(u16 port){
	if(this->IsValid())
		throw Socket::Exc("UDPSocket::Open(): the socket is already opened");

#ifdef WIN32
	this->CreateEventForWaitable();
#endif

	this->socket = ::socket(AF_INET, SOCK_DGRAM, 0);
	if(this->socket == DInvalidSocket()){
#ifdef WIN32
		this->CloseEventForWaitable();
#endif
		throw Socket::Exc("UDPSocket::Open(): ::socket() failed");
	}

	//Bind locally, if appropriate
	if(port != 0){
		struct sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = INADDR_ANY;
		sockAddr.sin_port = htons(port);

		// Bind the socket for listening
		if(::bind(
				this->socket,
				reinterpret_cast<struct sockaddr*>(&sockAddr),
				sizeof(sockAddr)
			) == DSocketError())
		{
			this->Close();
			throw Socket::Exc("UDPSocket::Open(): could not bind to local port");
		}
	}

	this->SetNonBlockingMode();

	//Allow broadcasting
#if defined(WIN32) || defined(__linux__) || defined(__APPLE__)
	{
		int yes = 1;
		if(setsockopt(
				this->socket,
				SOL_SOCKET,
				SO_BROADCAST,
				reinterpret_cast<char*>(&yes),
				sizeof(yes)
			) == DSocketError())
		{
			this->Close();
			throw Socket::Exc("UDPSocket::Open(): failed setting broadcast option");
		}
	}
#else
#error "Unsupported OS"
#endif

	this->ClearAllReadinessFlags();
}



size_t UDPSocket::Send(const ting::Buffer<u8>& buf, const IPAddress& destinationIP){
	if(!this->IsValid())
		throw Socket::Exc("UDPSocket::Send(): socket is not opened");

	this->ClearCanWriteFlag();

	sockaddr_in sockAddr;
	int sockLen = sizeof(sockAddr);

	sockAddr.sin_addr.s_addr = htonl(destinationIP.host);
	sockAddr.sin_port = htons(destinationIP.port);
	sockAddr.sin_family = AF_INET;


	ssize_t len;

	while(true){
		len = ::sendto(
				this->socket,
				reinterpret_cast<const char*>(buf.Begin()),
				buf.Size(),
				0,
				reinterpret_cast<struct sockaddr*>(&sockAddr),
				sockLen
			);

		if(len == DSocketError()){
#ifdef WIN32
			int errorCode = WSAGetLastError();
#else
			int errorCode = errno;
#endif
			if(errorCode == DEIntr()){
				continue;
			}else if(errorCode == DEAgain()){
				//can't send more bytes, return 0 bytes sent
				len = 0;
			}else{
				std::stringstream ss;
				ss << "UDPSocket::Send(): sendto() failed, error code = " << errorCode << ": ";
#ifdef _MSC_VER //if MSVC compiler
				{
					const size_t msgbufSize = 0xff;
					char msgbuf[msgbufSize];
					strerror_s(msgbuf, msgbufSize, errorCode);
					msgbuf[msgbufSize - 1] = 0;//make sure the string is null-terminated
					ss << msgbuf;
				}
#else
				ss << strerror(errorCode);
#endif
				throw Socket::Exc(ss.str());
			}
		}
		break;
	}//~while

	ASSERT(buf.Size() <= size_t(ting::DMaxInt))
	ASSERT_INFO(len <= int(buf.Size()), "res = " << len)
	ASSERT_INFO((len == int(buf.Size())) || (len == 0), "res = " << len)

	ASSERT(len >= 0)
	return size_t(len);
}



size_t UDPSocket::Recv(ting::Buffer<u8>& buf, IPAddress &out_SenderIP){
	if(!this->IsValid())
		throw Socket::Exc("UDPSocket::Recv(): socket is not opened");

	//The 'can read' flag shall be cleared even if this function fails.
	//This is to avoid subsequent calls to Recv() because of it indicating
	//that there's an activity.
	//So, do it at the beginning of the function.
	this->ClearCanReadFlag();

	sockaddr_in sockAddr;

#ifdef WIN32
	int sockLen = sizeof(sockAddr);
#elif defined(__linux__) || defined(__APPLE__)
	socklen_t sockLen = sizeof(sockAddr);
#else
#error "Unsupported OS"
#endif

	ssize_t len;

	while(true){
		len = ::recvfrom(
				this->socket,
				reinterpret_cast<char*>(buf.Begin()),
				buf.Size(),
				0,
				reinterpret_cast<sockaddr*>(&sockAddr),
				&sockLen
			);

		if(len == DSocketError()){
#ifdef WIN32
			int errorCode = WSAGetLastError();
#else
			int errorCode = errno;
#endif
			if(errorCode == DEIntr()){
				continue;
			}else if(errorCode == DEAgain()){
				//no data available, return 0 bytes received
				len = 0;
			}else{
				std::stringstream ss;
				ss << "UDPSocket::Recv(): recvfrom() failed, error code = " << errorCode << ": ";
#ifdef _MSC_VER //if MSVC compiler
				{
					const size_t msgbufSize = 0xff;
					char msgbuf[msgbufSize];
					strerror_s(msgbuf, msgbufSize, errorCode);
					msgbuf[msgbufSize - 1] = 0;//make sure the string is null-terminated
					ss << msgbuf;
				}
#else
				ss << strerror(errorCode);
#endif
				throw Socket::Exc(ss.str());
			}
		}
		break;
	}//~while

	ASSERT(buf.Size() <= size_t(ting::DMaxInt))
	ASSERT_INFO(len <= int(buf.Size()), "len = " << len)

	out_SenderIP.host = ntohl(sockAddr.sin_addr.s_addr);
	out_SenderIP.port = ntohs(sockAddr.sin_port);
	
	ASSERT(len >= 0)
	return size_t(len);
}



#ifdef WIN32
//override
void UDPSocket::SetWaitingEvents(u32 flagsToWaitFor){
	long flags = FD_CLOSE;
	if((flagsToWaitFor & Waitable::READ) != 0){
		flags |= FD_READ;
	}
	if((flagsToWaitFor & Waitable::WRITE) != 0){
		flags |= FD_WRITE;
	}
	this->SetWaitingEventsForWindows(flags);
}
#endif