/* The MIT License:

Copyright (c) 2009-2013 Ivan Gagis <igagis@gmail.com>

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

// Home page: http://ting.googlecode.com



#include "UDPSocket.hpp"

#if M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX || M_OS == M_OS_SOLARIS
#	include <netinet/in.h>
#endif



using namespace ting::net;



void UDPSocket::Open(u16 port){
	if(this->IsValid()){
		throw net::Exc("UDPSocket::Open(): the socket is already opened");
	}

#if M_OS == M_OS_WINDOWS
	this->CreateEventForWaitable();
#endif

	this->socket = ::socket(PF_INET6, SOCK_DGRAM, 0);
	
	if(this->socket == DInvalidSocket()){
#if M_OS == M_OS_WINDOWS
		this->CloseEventForWaitable();
#endif
		throw net::Exc("UDPSocket::Open(): ::socket() failed");
	}

	bool ipv4 = false;
	
	//turn off IPv6 only mode to allow also accepting IPv4 connections
	{
#if M_OS == M_OS_WINDOWS
		char no = 0;
		const char* noPtr = &no;
#else
		int no = 0;
		void* noPtr = &no;
#endif
		if(setsockopt(this->socket, IPPROTO_IPV6, IPV6_V6ONLY, noPtr, sizeof(no)) != 0){
			//Dual stack is not supported, proceed with IPv4 only.
			
			this->Close();//close IPv6 socket
			
			//create IPv4 socket
			
#if M_OS == M_OS_WINDOWS
			this->CreateEventForWaitable();
#endif			
			
			this->socket = ::socket(PF_INET, SOCK_DGRAM, 0);
	
			if(this->socket == DInvalidSocket()){
#if M_OS == M_OS_WINDOWS
				this->CloseEventForWaitable();
#endif
				throw net::Exc("TCPServerSocket::Open(): Couldn't create socket");
			}
			
			ipv4 = true;
		}
	}
	
	//Bind locally, if appropriate
	if(port != 0){
		sockaddr_storage sockAddr;
		socklen_t sockAddrLen;

		if(ipv4){
			sockaddr_in& sa = reinterpret_cast<sockaddr_in&>(sockAddr);
			memset(&sa, 0, sizeof(sa));
			sa.sin_family = AF_INET;
			sa.sin_addr.s_addr = INADDR_ANY;
			sa.sin_port = htons(port);
			sockAddrLen = sizeof(sa);
		}else{
			sockaddr_in6& sa = reinterpret_cast<sockaddr_in6&>(sockAddr);
			memset(&sa, 0, sizeof(sa));
			sa.sin6_family = AF_INET6;
			sa.sin6_addr = in6addr_any;//'in6addr_any' allows accepting both IPv4 and IPv6 connections!!!
			sa.sin6_port = htons(port);
			sockAddrLen = sizeof(sa);
		}

		// Bind the socket for listening
		if(::bind(
				this->socket,
				reinterpret_cast<struct sockaddr*>(&sockAddr),
				sockAddrLen
			) == DSocketError())
		{
			this->Close();
			
#if M_OS == M_OS_WINDOWS
			int errorCode = WSAGetLastError();
#else
			int errorCode = errno;
#endif
			
				std::stringstream ss;
				ss << "UDPSocket::Open(): could not bind to local port, error code = " << errorCode << ": ";
#if M_COMPILER == M_COMPILER_MSVC
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
			throw net::Exc(ss.str());
		}
	}

	this->SetNonBlockingMode();

	//Allow broadcasting
#if M_OS == M_OS_WINDOWS || M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX || M_OS == M_OS_SOLARIS
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
			throw net::Exc("UDPSocket::Open(): failed setting broadcast option");
		}
	}
#else
#	error "Unsupported OS"
#endif

	this->ClearAllReadinessFlags();
}



size_t UDPSocket::Send(const ting::Buffer<const ting::u8>& buf, const IPAddress& destinationIP){
	if(!this->IsValid()){
		throw net::Exc("UDPSocket::Send(): socket is not opened");
	}

	this->ClearCanWriteFlag();

	sockaddr_storage sockAddr;
	socklen_t sockAddrLen;
	
	if(destinationIP.host.IsIPv4()){
		sockaddr_in& a = reinterpret_cast<sockaddr_in&>(sockAddr);
		memset(&a, 0, sizeof(a));
		a.sin_family = AF_INET;
		a.sin_addr.s_addr = htonl(destinationIP.host.IPv4Host());
		a.sin_port = htons(destinationIP.port);
		sockAddrLen = sizeof(sockaddr_in);
	}else{
		sockaddr_in6& a = reinterpret_cast<sockaddr_in6&>(sockAddr);
		memset(&a, 0, sizeof(a));
		a.sin6_family = AF_INET6;
#if M_OS == M_OS_MACOSX || M_OS == M_OS_WINDOWS || (M_OS == M_OS_LINUX && defined(__ANDROID__))
		a.sin6_addr.s6_addr[0] = destinationIP.host.Quad0() >> 24;
		a.sin6_addr.s6_addr[1] = (destinationIP.host.Quad0() >> 16) & 0xff;
		a.sin6_addr.s6_addr[2] = (destinationIP.host.Quad0() >> 8) & 0xff;
		a.sin6_addr.s6_addr[3] = destinationIP.host.Quad0() & 0xff;
		a.sin6_addr.s6_addr[4] = destinationIP.host.Quad1() >> 24;
		a.sin6_addr.s6_addr[5] = (destinationIP.host.Quad1() >> 16) & 0xff;
		a.sin6_addr.s6_addr[6] = (destinationIP.host.Quad1() >> 8) & 0xff;
		a.sin6_addr.s6_addr[7] = destinationIP.host.Quad1() & 0xff;
		a.sin6_addr.s6_addr[8] = destinationIP.host.Quad2() >> 24;
		a.sin6_addr.s6_addr[9] = (destinationIP.host.Quad2() >> 16) & 0xff;
		a.sin6_addr.s6_addr[10] = (destinationIP.host.Quad2() >> 8) & 0xff;
		a.sin6_addr.s6_addr[11] = destinationIP.host.Quad2() & 0xff;
		a.sin6_addr.s6_addr[12] = destinationIP.host.Quad3() >> 24;
		a.sin6_addr.s6_addr[13] = (destinationIP.host.Quad3() >> 16) & 0xff;
		a.sin6_addr.s6_addr[14] = (destinationIP.host.Quad3() >> 8) & 0xff;
		a.sin6_addr.s6_addr[15] = destinationIP.host.Quad3() & 0xff;
#else
		a.sin6_addr.__in6_u.__u6_addr32[0] = htonl(destinationIP.host.Quad0());
		a.sin6_addr.__in6_u.__u6_addr32[1] = htonl(destinationIP.host.Quad1());
		a.sin6_addr.__in6_u.__u6_addr32[2] = htonl(destinationIP.host.Quad2());
		a.sin6_addr.__in6_u.__u6_addr32[3] = htonl(destinationIP.host.Quad3());
#endif
		a.sin6_port = htons(destinationIP.port);
		sockAddrLen = sizeof(sockaddr_in6);
	}

	ssize_t len;

	while(true){
		len = ::sendto(
				this->socket,
				reinterpret_cast<const char*>(buf.Begin()),
				buf.Size(),
				0,
				reinterpret_cast<struct sockaddr*>(&sockAddr),
				sockAddrLen
			);

		if(len == DSocketError()){
#if M_OS == M_OS_WINDOWS
			int errorCode = WSAGetLastError();
			
			if(errorCode == WSAEAFNOSUPPORT){
				throw net::Exc("Address family is not supported by protocol family. Note, that libting on WinXP does not support IPv6.");
			}
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
#if M_COMPILER == M_COMPILER_MSVC
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
				throw net::Exc(ss.str());
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



size_t UDPSocket::Recv(const ting::Buffer<ting::u8>& buf, IPAddress &out_SenderIP){
	if(!this->IsValid()){
		throw net::Exc("UDPSocket::Recv(): socket is not opened");
	}

	//The "can read" flag shall be cleared even if this function fails.
	//This is to avoid subsequent calls to Recv() because of it indicating
	//that there's an activity.
	//So, do it at the beginning of the function.
	this->ClearCanReadFlag();

	sockaddr_storage sockAddr;

#if M_OS == M_OS_WINDOWS
	int sockLen = sizeof(sockAddr);
#elif M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX || M_OS == M_OS_SOLARIS
	socklen_t sockLen = sizeof(sockAddr);
#else
#	error "Unsupported OS"
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
#if M_OS == M_OS_WINDOWS
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
#if M_COMPILER == M_COMPILER_MSVC
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
				throw net::Exc(ss.str());
			}
		}
		break;
	}//~while

	ASSERT(buf.Size() <= size_t(ting::DMaxInt))
	ASSERT_INFO(len <= int(buf.Size()), "len = " << len)

	if(sockAddr.ss_family == AF_INET){
		sockaddr_in& a = reinterpret_cast<sockaddr_in&>(sockAddr);
		out_SenderIP = IPAddress(
				ntohl(a.sin_addr.s_addr),
				ting::u16(ntohs(a.sin_port))
			);
	}else{
		ASSERT_INFO(sockAddr.ss_family == AF_INET6, "sockAddr.ss_family = " << unsigned(sockAddr.ss_family) << " AF_INET = " << AF_INET << " AF_INET6 = " << AF_INET6)
		sockaddr_in6& a = reinterpret_cast<sockaddr_in6&>(sockAddr);
		out_SenderIP = IPAddress(
				IPAddress::Host(
#if M_OS == M_OS_MACOSX || M_OS == M_OS_WINDOWS || (M_OS == M_OS_LINUX && defined(__ANDROID__))
						(ting::u32(a.sin6_addr.s6_addr[0]) << 24) | (ting::u32(a.sin6_addr.s6_addr[1]) << 16) | (ting::u32(a.sin6_addr.s6_addr[2]) << 8) | ting::u32(a.sin6_addr.s6_addr[3]),
						(ting::u32(a.sin6_addr.s6_addr[4]) << 24) | (ting::u32(a.sin6_addr.s6_addr[5]) << 16) | (ting::u32(a.sin6_addr.s6_addr[6]) << 8) | ting::u32(a.sin6_addr.s6_addr[7]),
						(ting::u32(a.sin6_addr.s6_addr[8]) << 24) | (ting::u32(a.sin6_addr.s6_addr[9]) << 16) | (ting::u32(a.sin6_addr.s6_addr[10]) << 8) | ting::u32(a.sin6_addr.s6_addr[11]),
						(ting::u32(a.sin6_addr.s6_addr[12]) << 24) | (ting::u32(a.sin6_addr.s6_addr[13]) << 16) | (ting::u32(a.sin6_addr.s6_addr[14]) << 8) | ting::u32(a.sin6_addr.s6_addr[15])
#else
						ting::u32(ntohl(a.sin6_addr.__in6_u.__u6_addr32[0])),
						ting::u32(ntohl(a.sin6_addr.__in6_u.__u6_addr32[1])),
						ting::u32(ntohl(a.sin6_addr.__in6_u.__u6_addr32[2])),
						ting::u32(ntohl(a.sin6_addr.__in6_u.__u6_addr32[3]))
#endif
					),
				ting::u16(ntohs(a.sin6_port))
			);
	}
	
	ASSERT(len >= 0)
	return size_t(len);
}



#if M_OS == M_OS_WINDOWS
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
