/* The MIT License:

Copyright (c) 2009-2012 Ivan Gagis <igagis@gmail.com>

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



#include "TCPSocket.hpp"
#include "../util.hpp"

#if M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX || M_OS == M_OS_SOLARIS
#	include <netinet/in.h>
#endif



using namespace ting::net;



void TCPSocket::Open(const IPAddress& ip, bool disableNaggle){
	if(this->IsValid()){
		throw net::Exc("TCPSocket::Open(): socket already opened");
	}

	//create event for implementing Waitable
#if M_OS == M_OS_WINDOWS
	this->CreateEventForWaitable();
#endif

	this->socket = ::socket(
			ip.host.IsIPv4() ? PF_INET : PF_INET6,
			SOCK_STREAM,
			0
		);
	if(this->socket == DInvalidSocket()){
#if M_OS == M_OS_WINDOWS
		this->CloseEventForWaitable();
#endif
		throw net::Exc("TCPSocket::Open(): Couldn't create socket");
	}

	//Disable Naggle algorithm if required
	if(disableNaggle){
		this->DisableNaggle();
	}

	this->SetNonBlockingMode();

	this->ClearAllReadinessFlags();

	//Connecting to remote host
	sockaddr_storage sockAddr;
	
	if(ip.host.IsIPv4()){
		sockaddr_in &sa = reinterpret_cast<sockaddr_in&>(sockAddr);
		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = htonl(ip.host.IPv4Host());
		sa.sin_port = htons(ip.port);
	}else{
		sockaddr_in6 &sa = reinterpret_cast<sockaddr_in6&>(sockAddr);
		memset(&sa, 0, sizeof(sa));
		sa.sin6_family = AF_INET6;
#if M_OS == M_OS_MACOSX
//TODO:
		sa.sin6_addr.s6_addr32[0] = htonl(ip.host.Quad0());
		sa.sin6_addr.s6_addr32[1] = htonl(ip.host.Quad1());
		sa.sin6_addr.s6_addr32[2] = htonl(ip.host.Quad2());
		sa.sin6_addr.s6_addr32[3] = htonl(ip.host.Quad3());
#else
		sa.sin6_addr.__in6_u.__u6_addr32[0] = htonl(ip.host.Quad0());
		sa.sin6_addr.__in6_u.__u6_addr32[1] = htonl(ip.host.Quad1());
		sa.sin6_addr.__in6_u.__u6_addr32[2] = htonl(ip.host.Quad2());
		sa.sin6_addr.__in6_u.__u6_addr32[3] = htonl(ip.host.Quad3());
#endif
		sa.sin6_port = htons(ip.port);
	}

	// Connect to the remote host
	if(connect(
			this->socket,
			reinterpret_cast<sockaddr *>(&sockAddr),
			sizeof(sockAddr)
		) == DSocketError())
	{
#if M_OS == M_OS_WINDOWS
		int errorCode = WSAGetLastError();
#elif M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX || M_OS == M_OS_SOLARIS
		int errorCode = errno;
#else
#	error "Unsupported OS"
#endif
		if(errorCode == DEIntr()){
			//do nothing, for non-blocking socket the connection request still should remain active
		}else if(errorCode == DEInProgress()){
			//do nothing, this is not an error, we have non-blocking socket
		}else{
			std::stringstream ss;
			ss << "TCPSocket::Open(): connect() failed, error code = " << errorCode << ": ";
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
			this->Close();
			throw net::Exc(ss.str());
		}
	}
}



size_t TCPSocket::Send(const ting::Buffer<const ting::u8>& buf, size_t offset){
	if(!this->IsValid()){
		throw net::Exc("TCPSocket::Send(): socket is not opened");
	}

	this->ClearCanWriteFlag();

	ASSERT_INFO((
			(buf.Begin() + offset) <= (buf.End() - 1))
					|| ((buf.Size() == 0) && (offset == 0))
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
#if M_OS == M_OS_WINDOWS
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

	ASSERT(len >= 0)
	return size_t(len);
}



size_t TCPSocket::Recv(const ting::Buffer<ting::u8>& buf, size_t offset){
	//the 'can read' flag shall be cleared even if this function fails to avoid subsequent
	//calls to Recv() because it indicates that there's activity.
	//So, do it at the beginning of the function.
	this->ClearCanReadFlag();

	if(!this->IsValid()){
		throw net::Exc("TCPSocket::Recv(): socket is not opened");
	}

	ASSERT_INFO(
			((buf.Begin() + offset) <= (buf.End() - 1))
					|| ((buf.Size() == 0) && (offset == 0))
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
				ss << "TCPSocket::Recv(): recv() failed, error code = " << errorCode << ": ";
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

	ASSERT(len >= 0)
	return size_t(len);
}



namespace{

template <bool localAddress> IPAddress GetSockAddress(int sckt){
	sockaddr_storage addr;

#if M_OS == M_OS_WINDOWS
	int len = sizeof(addr);
#else
	socklen_t len = sizeof(addr);
#endif

	if(localAddress){
		if(getsockname(sckt, reinterpret_cast<sockaddr*>(&addr), &len) < 0){
			throw ting::net::Exc("Socket::GetLocalAddress(): getsockname() failed");
		}
	}else{
		if(getpeername(sckt, reinterpret_cast<sockaddr*>(&addr), &len) < 0){
			throw ting::net::Exc("Socket::GetRemoteAddress(): getpeername() failed");
		}
	}
	

	
	if(addr.ss_family == AF_INET){
		sockaddr_in &a = reinterpret_cast<sockaddr_in&>(addr);
		return IPAddress(
			ting::u32(ntohl(a.sin_addr.s_addr)),
			ting::u16(ntohs(a.sin_port))
		);
	}else{
		ASSERT(addr.ss_family == AF_INET6)
		
		sockaddr_in6 &a = reinterpret_cast<sockaddr_in6&>(addr);
		
		return IPAddress(
				IPAddress::Host(
						ting::u32(ntohl(a.sin6_addr.__in6_u.__u6_addr32[0])),
						ting::u32(ntohl(a.sin6_addr.__in6_u.__u6_addr32[1])),
						ting::u32(ntohl(a.sin6_addr.__in6_u.__u6_addr32[2])),
						ting::u32(ntohl(a.sin6_addr.__in6_u.__u6_addr32[3]))
					),
				ting::u16(ntohs(a.sin6_port))
			);
	}
}

}//~namespace



IPAddress TCPSocket::GetLocalAddress(){
	if(!this->IsValid()){
		throw net::Exc("Socket::GetLocalAddress(): socket is not valid");
	}

	return GetSockAddress<true>(this->socket);
}



IPAddress TCPSocket::GetRemoteAddress(){
	if(!this->IsValid()){
		throw net::Exc("TCPSocket::GetRemoteAddress(): socket is not valid");
	}

	return GetSockAddress<false>(this->socket);
}



#if M_OS == M_OS_WINDOWS
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
