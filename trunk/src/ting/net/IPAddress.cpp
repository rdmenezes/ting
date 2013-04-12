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


#include <sstream>

#include "IPAddress.hpp"

#include "../config.hpp"
#include "../Buffer.hpp"

#if M_OS == M_OS_LINUX
#	include <arpa/inet.h>
#elif M_OS == M_OS_WINDOWS
#	include <Ws2tcpip.h>
#else
#	error "Unknown OS"
#endif



using namespace ting::net;



namespace{

bool IsIPv4String(const char* ip){
	for(const char* p = ip; *p != 0; ++p){
		if(*p == '.'){
			return true;
		}
		if(*p == ':'){
			return false;
		}
	}
	return false;
}

}//~namespace



//static
IPAddress::Host IPAddress::Host::Parse(const char* ip){
	if(IsIPv4String(ip)){
		return Host::ParseIPv4(ip);
	}else{
		return Host::ParseIPv6(ip);
	}
}



//static
IPAddress::Host IPAddress::Host::ParseIPv4(const char* ip){
	sockaddr_in a;
	
#if M_OS == M_OS_LINUX
	int res = inet_pton(
#elif M_OS == M_OS_WINDOWS
	int res = InetPton(
#else
#	error "Unknown OS"
#endif
			AF_INET,
			ip,
			&a.sin_addr
		);
	
	if(res != 1){
		throw BadIPHostFormatExc();
	}
	
	return Host(ntohl(a.sin_addr.s_addr));
}



//static
IPAddress::Host IPAddress::Host::ParseIPv6(const char* ip){
	sockaddr_in6 a;
		
#if M_OS == M_OS_LINUX
	int res = inet_pton(
#elif M_OS == M_OS_WINDOWS
	int res = InetPton(
#else
#	error "Unknown OS"
#endif
			AF_INET6,
			ip,
			&a.sin6_addr
		);
	
	if(res != 1){
		throw BadIPHostFormatExc();
	}
	
	return Host(
			a.sin6_addr.__in6_u.__u6_addr8[0],
			a.sin6_addr.__in6_u.__u6_addr8[1],
			a.sin6_addr.__in6_u.__u6_addr8[2],
			a.sin6_addr.__in6_u.__u6_addr8[3],
			a.sin6_addr.__in6_u.__u6_addr8[4],
			a.sin6_addr.__in6_u.__u6_addr8[5],
			a.sin6_addr.__in6_u.__u6_addr8[6],
			a.sin6_addr.__in6_u.__u6_addr8[7],
			a.sin6_addr.__in6_u.__u6_addr8[8],
			a.sin6_addr.__in6_u.__u6_addr8[9],
			a.sin6_addr.__in6_u.__u6_addr8[10],
			a.sin6_addr.__in6_u.__u6_addr8[11],
			a.sin6_addr.__in6_u.__u6_addr8[12],
			a.sin6_addr.__in6_u.__u6_addr8[13],
			a.sin6_addr.__in6_u.__u6_addr8[14],
			a.sin6_addr.__in6_u.__u6_addr8[15]
		);
}



IPAddress::IPAddress(const char* ip, u16 p) :
		host(Host::Parse(ip)),
		port(p)
{}



IPAddress::IPAddress(const char* ip){
	if(*ip == 0){//if zero length string
		throw BadIPAddressFormatExc();
	}
	
	if(*ip == '['){//IPv6 with port
		ting::StaticBuffer<char, (4 * 6 + 6 + (3 * 4 + 3) + 1)> buf;
		
		++ip;
		
		char* dst;
		for(dst = buf.Begin(); *ip != ']'; ++dst, ++ip){
			if(*ip == 0 || !buf.Overlaps(dst + 1)){
				throw BadIPAddressFormatExc();
			}
			
			*dst = *ip;
		}
		
		ASSERT(buf.Overlaps(dst))
		*dst = 0;//null-terminate
				
		this->host = Host::ParseIPv6(buf.Begin());
		
		++ip;//move to port ':' separator
	}else{
		//IPv4 or IPv6 without port
		
		if(IsIPv4String(ip)){
			ting::StaticBuffer<char, (3 * 4 + 3 + 1)> buf;
			
			char* dst;
			for(dst = buf.Begin(); *ip != ':' && *ip != 0; ++dst, ++ip){
				if(!buf.Overlaps(dst + 1)){
					throw BadIPAddressFormatExc();
				}

				*dst = *ip;
			}

			ASSERT(buf.Overlaps(dst))
			*dst = 0;//null-terminate

			this->host = Host::ParseIPv4(buf.Begin());
		}else{
			//IPv6 without port
			this->host = Host::ParseIPv6(ip);
			this->port = 0;
			return;
		}
	}
	
	//parse port
	
	if(*ip != ':'){
		if(*ip == 0){
			this->port = 0;
			return;
		}else{
	//		TRACE(<< "no colon, *ip = " << (*ip) << std::endl)
			throw ting::net::IPAddress::BadIPAddressFormatExc();
		}
	}
	
	++ip;
	
	//move to the end of port number, maximum 5 digits.
	for(unsigned i = 0; '0' <= *ip && *ip <= '9' && i < 5; ++i, ++ip){
	}
	if('0' <= *ip && *ip <= '9'){//if still have one more digit
//		TRACE(<< "still have one more digit" << std::endl)
		throw ting::net::IPAddress::BadIPAddressFormatExc();
	}
	
	--ip;
	
	ting::u32 port = 0;
	
	for(unsigned i = 0; *ip != ':'; ++i, --ip){
		ting::u32 pow = 1;
		for(unsigned j = 0; j < i; ++j){
			pow *= 10;
		}
	
		ASSERT('0' <= *ip && *ip <= '9')
		
		port += (*ip - '0') * pow;
	}
	
	if(port > 0xffff){
//		TRACE(<< "port number is bigger than 0xffff" << std::endl)
		throw ting::net::IPAddress::BadIPAddressFormatExc();
	}
	
	this->port = ting::u16(port);
}



std::string IPAddress::Host::ToString()const{
	std::stringstream ss;
	if(this->IsIPv4()){
		for(unsigned i = 4;;){
			--i;
			ss << (((this->IPv4Host()) >> (8 * i)) & 0xff);
			if(i == 0){
				break;
			}
			ss << '.';
		}
	}else{
		ss << std::hex;
		for(unsigned i = 8;;){
			--i;
			ss << ((this->host[(i * 2) / 4] >> (16 * (i % 2))) & 0xffff);
			if(i == 0){
				break;
			}
			ss << ':';
		}
	}
	return ss.str();
}
