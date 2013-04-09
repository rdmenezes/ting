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



/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once



#include "Exc.hpp"

#include "../types.hpp"



namespace ting{
namespace net{



/**
 * @brief a structure which holds IP address
 */
class IPAddress{
public:
	//TODO: doxygen
	class BadIPAddressFormatExc : public ting::net::Exc{
	public:
		BadIPAddressFormatExc(){}
	};
	
	//TODO: doxygen
	class Host{
		u32 host[4];//IPv6 address
	public:
		
		//TODO: doxygen
		inline u32 Quad0()const throw(){
			return this->host[0];
		}
		
		//TODO: doxygen
		inline u32 Quad1()const throw(){
			return this->host[1];
		}
		
		//TODO: doxygen
		inline u32 Quad2()const throw(){
			return this->host[2];
		}
		
		//TODO: doxygen
		inline u32 Quad3()const throw(){
			return this->host[3];
		}
		
		//TODO: doxygen
		inline void Init(u32 a0, u32 a1, u32 a2, u32 a3)throw(){
			this->host[0] = a0;
			this->host[1] = a1;
			this->host[2] = a2;
			this->host[3] = a3;
		}
		
		//TODO: doxygen
		inline void Init(u32 h)throw(){
			this->Init(0, 0, 0xffff, h);
		}
		
		//TODO: doxygen
		inline void Init(u16 a0, u16 a1, u16 a2, u16 a3, u16 a4, u16 a5, u16 a6, u16 a7)throw(){
			this->Init(
					(u32(a0) << 16) | u32(a1),
					(u32(a2) << 16) | u32(a3),
					(u32(a4) << 16) | u32(a5),
					(u32(a6) << 16) | u32(a7)
				);
		}
		
		//TODO: doxygen
		inline void Init(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, u8 a5, u8 a6, u8 a7, u8 a8, u8 a9, u8 a10, u8 a11, u8 a12, u8 a13, u8 a14, u8 a15)throw(){
			this->Init(
					(u16(a0) << 8) | u16(a1),
					(u16(a2) << 8) | u16(a3),
					(u16(a4) << 8) | u16(a5),
					(u16(a6) << 8) | u16(a7),
					(u16(a8) << 8) | u16(a9),
					(u16(a10) << 8) | u16(a11),
					(u16(a12) << 8) | u16(a13),
					(u16(a14) << 8) | u16(a15)
				);
		}
	
		//TODO: doxygen
		class BadIPHostFormatExc : public BadIPAddressFormatExc{
		public:
			BadIPHostFormatExc(){}
		};
		
		//TODO: doxygen
		Host()throw(){}
		
		//TODO: doxygen
		Host(u32 h)throw(){
			this->Init(h);
		}
		
		//TODO: doxygen
		inline Host(u32 a0, u32 a1, u32 a2, u32 a3)throw(){
			this->Init(a0, a1, a2, a3);
		}
		
		//TODO: doxygen
		inline Host(u16 a0, u16 a1, u16 a2, u16 a3, u16 a4, u16 a5, u16 a6, u16 a7)throw(){
			this->Init(a0, a1, a2, a3, a4, a5, a6, a7);
		}
		
		//TODO: doxygen
		inline Host(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, u8 a5, u8 a6, u8 a7, u8 a8, u8 a9, u8 a10, u8 a11, u8 a12, u8 a13, u8 a14, u8 a15)throw(){
			this->Init(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
		}
		
		//TODO: doxygen
		static Host Parse(const char* ip);
		
		//TODO: doxygen
		static Host ParseIPv4(const char* ip);
		
		//TODO: doxygen
		static Host ParseIPv6(const char* ip);
		
		//TODO: doxygen
		inline bool IsIPv4()const throw(){
			return this->host[2] == 0xffff && this->host[1] == 0 && this->host[0] == 0;
		}
		
		//TODO: doxygen
		inline u32 IPv4Host()const throw(){
			return this->host[3];
		}
		
		//TODO: doxygen
		inline bool operator==(const Host& h){
			return (this->host[0] == h.host[0])
					&& (this->host[1] == h.host[1])
					&& (this->host[2] == h.host[2])
					&& (this->host[3] == h.host[3])
				;
		}
	};
	
	Host host;///< IPv6 address
	u16 port;///< IP port number
	
	//TODO: doxygen
	inline IPAddress()throw(){}

	/**
	 * @brief Create IPv4-address specifying exact IP-address and port number.
	 * @param h - IPv4 address. For example, 0x7f000001 represents "127.0.0.1" IP address value.
	 * @param p - IP port number.
	 */
	inline IPAddress(u32 h, u16 p)throw() :
			host(h),
			port(p)
	{}

	/**
	 * @brief Create IPv4-address specifying exact IP-address as 4 bytes and port number.
	 * The IPv4-address can be specified as 4 separate byte values, for example:
	 * @code
	 * ting::IPAddress ip(127, 0, 0, 1, 80); //"127.0.0.1" port 80
	 * @endcode
	 * @param h1 - 1st triplet of IP address.
	 * @param h2 - 2nd triplet of IP address.
	 * @param h3 - 3rd triplet of IP address.
	 * @param h4 - 4th triplet of IP address.
	 * @param p - IP port number.
	 */
	inline IPAddress(u8 h1, u8 h2, u8 h3, u8 h4, u16 p)throw() :
			host((u32(h1) << 24) + (u32(h2) << 16) + (u32(h3) << 8) + u32(h4)),
			port(p)
	{}

	//TODO: doxygen
	inline IPAddress(Host h, u16 p)throw() :
			host(h),
			port(p)
	{}
	
	//TODO: IPv6 doxygen
	/**
	 * @brief Create IP-address specifying IP-address as string and port number.
	 * The string passed as argument should contain properly formatted IP address at its beginning.
	 * It is OK if string contains something else after the IP-address.
	 * Only IP address is parsed, even if port number is specified after the IP-address it will not be parsed,
	 * instead the port number will be taken from the corresponding argument of the constructor.
	 * @param ip - IP-address null-terminated string. Example: "127.0.0.1".
	 * @param p - IP-port number.
	 * @throw BadIPAddressFormatExc - when passed string does not contain properly formatted IP-address.
	 */
	IPAddress(const char* ip, u16 p);
	
	//TODO: IPv6 doxygen
	/**
	 * @brief Create IP-address specifying IP-address as string and port number.
	 * The string passed for parsing should contain the IP-address with the port number.
	 * If there is no port number specified after the IP-address the format of the IP-address
	 * is regarded as invalid and corresponding exception is thrown.
     * @param ip - null-terminated string representing IP-address with port number, e.g. "127.0.0.1:80".
	 * @throw BadIPAddressFormatExc - when passed string does not contain properly formatted IP-address.
     */
	IPAddress(const char* ip);

	/**
	 * @brief compares two IP addresses for equality.
	 * @param ip - IP address to compare with.
	 * @return true if hosts and ports of the two IP addresses are equal accordingly.
	 * @return false otherwise.
	 */
	inline bool operator==(const IPAddress& ip){
		return (this->host == ip.host) && (this->port == ip.port);
	}
};//~class IPAddress



}//~namespace
}//~namespace
