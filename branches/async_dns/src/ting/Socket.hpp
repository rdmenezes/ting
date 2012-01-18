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

// Homepage: http://ting.googlecode.com



/**
 * @file Socket.hpp
 * @author Ivan Gagis <igagis@gmail.com>
 * @brief Main header file of the socket network library.
 * This is the main header file of socket network library, cross platfrom C++ Sockets wrapper.
 */

#pragma once


#include <string>
#include <sstream>


#if defined(WIN32)

#include <winsock2.h>
#include <windows.h>

#elif defined(__linux__) || defined(__APPLE__)
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#else
#error "Unsupported OS"
#endif



#include "Singleton.hpp"
#include "Exc.hpp"
#include "types.hpp"
#include "WaitSet.hpp"
#include "debug.hpp"
#include "Thread.hpp"
#include "utils.hpp"



/**
 * @brief the main namespace of ting library.
 * All the declarations of ting library are made inside this namespace.
 */
namespace ting{
namespace net{



/**
 * @brief Basic exception class.
 * This is a basic exception class for network related errors.
 */
class Exc : public ting::Exc{
public:
	/**
	 * @brief Exception constructor.
	 * @param message - human friendly error description.
	 */
	inline Exc(const std::string& message) :
			ting::Exc(message)
	{}
};



/**
 * @brief Basic socket class.
 * This is a base class for all socket types such as TCP sockets or UDP sockets.
 */
class Socket : public Waitable{
protected:
#ifdef WIN32
	typedef SOCKET T_Socket;

	inline static T_Socket DInvalidSocket(){
		return INVALID_SOCKET;
	}

	inline static int DSocketError(){
		return SOCKET_ERROR;
	}

	inline static int DEIntr(){
		return WSAEINTR;
	}
	
	inline static int DEAgain(){
		return WSAEWOULDBLOCK;
	}

	inline static int DEInProgress(){
		return WSAEWOULDBLOCK;
	}
#elif defined(__linux__) || defined(__APPLE__)
	typedef int T_Socket;

	inline static T_Socket DInvalidSocket(){
		return -1;
	}

	inline static T_Socket DSocketError(){
		return -1;
	}

	inline static int DEIntr(){
		return EINTR;
	}

	inline static int DEAgain(){
		return EAGAIN;
	}

	inline static int DEInProgress(){
		return EINPROGRESS;
	}
#else
#error "Unsupported OS"
#endif

#ifdef WIN32
	WSAEVENT eventForWaitable;
#endif

	T_Socket socket;

	Socket() :
#ifdef WIN32
			eventForWaitable(WSA_INVALID_EVENT),
#endif
			socket(DInvalidSocket())
	{
//		TRACE(<< "Socket::Socket(): invoked " << this << std::endl)
	}



	//same as std::auto_ptr
	Socket& operator=(const Socket& s);



	void DisableNaggle();



	void SetNonBlockingMode();



public:
	Socket(const Socket& s);

	
	
	virtual ~Socket();



	/**
	 * @brief Tells whether the socket is opened or not.
	 * @return Returns true if the socket is opened or false otherwise.
	 */
	inline bool IsValid()const{
		return this->socket != DInvalidSocket();
	}



	/**
	 * @brief Tells whether the socket is opened or not.
	 * @return inverse of IsValid().
	 */
	inline bool IsNotValid()const{
		return !this->IsValid();
	}



	/**
	 * @brief Closes the socket disconnecting it if necessary.
	 */
	void Close();



	/**
	 * @brief Returns local port this socket is bound to.
	 * @return local port number to which this socket is bound,
	 *         0 means that the socket is not bound to a port.
	 */
	u16 GetLocalPort();



#ifdef WIN32
private:
	//override
	HANDLE GetHandle();

	//override
	bool CheckSignalled();

protected:
	void CreateEventForWaitable();

	void CloseEventForWaitable();

	void SetWaitingEventsForWindows(long flags);



#else
private:
	//override
	int GetHandle();
#endif
};//~class Socket



/**
 * @brief a structure which holds IP address
 */
class IPAddress{
public:
	ting::Inited<u32, 0> host;///< IP address
	ting::Inited<u16, 0> port;///< IP port number

	inline IPAddress(){}

	/**
	 * @brief Create IP address specifying exact ip address and port number.
	 * @param h - IP address. For example, 0x7f000001 represents "127.0.0.1" IP address value.
	 * @param p - IP port number.
	 */
	inline IPAddress(u32 h, u16 p) :
			host(h),
			port(p)
	{}

	/**
	 * @brief Create IP address specifying exact ip address as 4 bytes and port number.
	 * The ip address can be specified as 4 separate byte values, for example:
	 * @code
	 * ting::IPAddress ip(127, 0, 0, 1, 80); //"127.0.0.1" port 80
	 * @endcode
	 * @param h1 - 1st triplet of IP address.
	 * @param h2 - 2nd triplet of IP address.
	 * @param h3 - 3rd triplet of IP address.
	 * @param h4 - 4th triplet of IP address.
	 * @param p - IP port number.
	 */
	inline IPAddress(u8 h1, u8 h2, u8 h3, u8 h4, u16 p) :
			host((u32(h1) << 24) + (u32(h2) << 16) + (u32(h3) << 8) + u32(h4)),
			port(p)
	{}

	/**
	 * @brief Create IP address specifying ip address as string and port number.
	 * @param ip - IP address null-terminated string. Example: "127.0.0.1".
	 * @param p - IP port number.
	 */
	inline IPAddress(const char* ip, u16 p) :
			host(IPAddress::ParseString(ip)),
			port(p)
	{}

	/**
	 * @brief compares two IP addresses for equality.
	 * @param ip - IP address to compare with.
	 * @return true if hosts and ports of the two IP addresses are equal accordingly.
	 * @return false otherwise.
	 */
	inline bool operator==(const IPAddress& ip){
		return (this->host == ip.host) && (this->port == ip.port);
	}
	
private:
	//parse IP address from string
	//TODO: need to support port parsing also
	//TODO: move this function completely to cpp
	static u32 ParseString(const char* ip);
};//~class IPAddress



/**
 * @brief Socket library singleton class.
 * This is a Socket library singleton class. Creating an object of this class initializes the library
 * while destroying this object de-initializes it. So, the convenient way of initializing the library
 * is to create an object of this class on the stack. Thus, when the object goes out of scope its
 * destructor will be called and the library will be de-initialized automatically.
 * This is what C++ RAII is all about.
 */
class Lib : public IntrusiveSingleton<Lib>{
	friend class IntrusiveSingleton<Lib>;
	static IntrusiveSingleton<Lib>::T_Instance instance;
	
public:
	Lib();

	~Lib();
};



/**
 * @brief Class for resolving IP-address of the host by its domain name.
 * This class allows asynchronous DNS lookup.
 * One has to derive his/her own class from this class to override the
 * OnCompleted_ts() method which will be called upon the DNS lookup operation has finished.
 */
class HostNameResolver{
	//no copying
	HostNameResolver(const HostNameResolver&);
	HostNameResolver& operator=(const HostNameResolver&);
	
public:
	virtual ~HostNameResolver();
	
	/**
	 * @brief Basic DNS lookup exception.
     * @param message - human friendly error description.
     */
	class Exc : public net::Exc{
	public:
		inline Exc(const std::string& message) :
				ting::net::Exc(message)
		{}
	};
	
	class DomainNameTooLongExc : public Exc{
	public:
		DomainNameTooLongExc() :
				Exc("Too long domain name, it should not exceed 253 characters according to RFC 2181")
		{}
	};
	
	class TooMuchRequestsExc : public Exc{
	public:
		TooMuchRequestsExc() :
				Exc("Too much active DNS lookup requests in progress")
		{}
	};
	
	class AlreadyInProgressExc : public Exc{
	public:
		AlreadyInProgress() :
				Exc("DNS lookup operation is already in progress")
		{}
	};
	
	/**
	 * @brief Start asynchronous IP-address resolving.
	 * The method is thread-safe.
     * @param hostName - host name to resolve IP-address for.
     * @param timeoutMillis - timeout for waiting for DNS server response in milliseconds.
	 * @throw DomainNameTooLongExc when supplied for resolution domain name is too long.
	 * @throw TooMuchRequestsExc when there are too much active DNS lookup requests are in progress, no resources for another one.
	 * @throw AlreadyInProgressExc when DNS lookup operation served by this resolver object is already in progress.
     */
	void Resolve_ts(const std::string& hostName, ting::u32 timeoutMillis = 20000);
	
	/**
	 * @brief Cancel current DNS lookup operation.
	 * The method is thread-safe.
	 * @return true - if the ongoing DNS lookup operation was canceled.
	 * @return false - if there was no ongoing DNS lookup operation to cancel.
	 *                 This means that the DNS lookup operation was not started
	 *                 or has finished before the Cancel_ts() method was called.
     */
	bool Cancel_ts();
	
	/**
	 * @brief Enumeration of the DNS lookup operation result.
	 */
	enum E_Result{
		/**
		 * @brief DNS lookup operation completed successfully.
		 */
		OK,
		
		/**
		 * @brief Timeout hit while waiting for the response from DNS server.
		 */
		TIMEOUT,
		
		/**
		 * @brief DNS server reported that there is not such host.
		 */
		NO_SUCH_HOST,
		
		/**
		 * @brief Error occurred while DNS lookup operation.
		 */
		ERROR
	};
	
	/**
	 * @brief callback method called upon DNS lookup operation has finished.
	 * Note, that the method has to be thread-safe.
     * @param result - the result of DNS lookup operation.
	 * @param ip - resolved IP-address. This value can later be used to create the
	 *             ting::IPAddress object.
     */
	virtual void OnCompleted_ts(E_Result result, ting::u32 ip) = 0;
};



/**
 * @brief a class which represents a TCP socket.
 */
class TCPSocket : public Socket{
	friend class TCPServerSocket;
public:
	
	/**
	 * @brief Constructs an invalid TCP socket object.
	 */
	TCPSocket(){
//		TRACE(<< "TCPSocket::TCPSocket(): invoked " << this << std::endl)
	}

	
	
	/**
	 * @brief A copy constructor.
	 * Copy constructor creates a new socket object which refers to the same socket as s.
	 * After constructor completes the s becomes invalid.
	 * In other words, the behavior of copy constructor is similar to one of std::auto_ptr class from standard C++ library.
	 * @param s - other TCP socket to make a copy from.
	 */
	//copy constructor
	TCPSocket(const TCPSocket& s) :
			Socket(s)
	{
//		TRACE(<< "TCPSocket::TCPSocket(copy): invoked " << this << std::endl)
	}

	
	
	/**
	 * @brief Assignment operator, works similar to std::auto_ptr::operator=().
	 * After this assignment operator completes this socket object refers to the socket the s object referred, s become invalid.
	 * It works similar to std::auto_ptr::operator=() from standard C++ library.
	 * @param s - socket to assign from.
	 */
	inline TCPSocket& operator=(const TCPSocket& s){
		this->Socket::operator=(s);
		return *this;
	}

	
	
	/**
	 * @brief Connects the socket.
	 * This method connects the socket to remote TCP server socket.
	 * @param ip - IP address.
	 * @param disableNaggle - enable/disable Naggle algorithm.
	 */
	void Open(const IPAddress& ip, bool disableNaggle = false);



	/**
	 * @brief Send data to connected socket.
	 * Sends data on connected socket. This method does not guarantee that the whole
	 * buffer will be sent completely, it will return the number of bytes actually sent.
	 * @param buf - pointer to the buffer with data to send.
	 * @param offset - offset inside the buffer from where to start sending the data.
	 * @return the number of bytes actually sent.
	 */
	size_t Send(const ting::Buffer<u8>& buf, size_t offset = 0);



	/**
	 * @brief Send data to connected socket.
	 * Sends data on connected socket. This method blocks until all data is completely sent.
	 * @param buf - the buffer with data to send.
	 */
	void SendAll(const ting::Buffer<u8>& buf);



	/**
	 * @brief Receive data from connected socket.
	 * Receives data available on the socket.
	 * If there is no data available this function does not block, instead it returns 0,
	 * indicating that 0 bytes were received.
	 * If previous WaitSet::Wait() indicated that socket is ready for reading
	 * and TCPSocket::Recv() returns 0, then connection was closed by peer.
	 * @param buf - pointer to the buffer where to put received data.
	 * @param offset - offset inside the buffer where to start putting data from.
	 * @return the number of bytes written to the buffer.
	 */
	size_t Recv(ting::Buffer<u8>& buf, size_t offset = 0);

	
	
	/**
	 * @brief Get local IP address and port.
	 * @return IP address and port of the local socket.
	 */
	IPAddress GetLocalAddress();
	
	
	
	/**
	 * @brief Get remote IP address and port.
	 * @return IP address and port of the peer socket.
	 */
	IPAddress GetRemoteAddress();



#ifdef WIN32
private:
	//override
	void SetWaitingEvents(u32 flagsToWaitFor);
#endif

};//~class TCPSocket



/**
 * @brief a class which represents a TCP server socket.
 * TCP server socket is the socket which can listen for new connections
 * and accept them creating an ordinary TCP socket for it.
 */
class TCPServerSocket : public Socket{
	ting::Inited<bool, false> disableNaggle;//this flag indicates if accepted sockets should be created with disabled Naggle
public:
	/**
	 * @brief Creates an invalid (unopened) TCP server socket.
	 */
	TCPServerSocket(){}

	
	
	/**
	 * @brief A copy constructor.
	 * Copy constructor creates a new socket object which refers to the same socket as s.
	 * After constructor completes the s becomes invalid.
	 * In other words, the behavior of copy constructor is similar to one of std::auto_ptr class from standard C++ library.
	 * @param s - other TCP socket to make a copy from.
	 */
	//copy constructor
	TCPServerSocket(const TCPServerSocket& s) :
			Socket(s),
			disableNaggle(s.disableNaggle)
	{}

	
	
	/**
	 * @brief Assignment operator, works similar to std::auto_ptr::operator=().
	 * After this assignment operator completes this socket object refers to the socket the s object referred, s become invalid.
	 * It works similar to std::auto_ptr::operator=() from standard C++ library.
	 * @param s - socket to assign from.
	 */
	TCPServerSocket& operator=(const TCPServerSocket& s){
		this->disableNaggle = s.disableNaggle;
		this->Socket::operator=(s);
		return *this;
	}

	
	
	/**
	 * @brief Connects the socket or starts listening on it.
	 * This method starts listening on the socket for incoming connections.
	 * @param port - IP port number to listen on.
	 * @param disableNaggle - enable/disable Naggle algorithm for all accepted connections.
	 * @param queueLength - the maximum length of the queue of pending connections.
	 */
	void Open(u16 port, bool disableNaggle = false, u16 queueLength = 50);
	
	
	
	/**
	 * @brief Accepts one of the pending connections, non-blocking.
	 * Accepts one of the pending connections and returns a TCP socket object which represents
	 * either a valid connected socket or an invalid socket object.
	 * This function does not block if there is no any pending connections, it just returns invalid
	 * socket object in this case. One can periodically check for incoming connections by calling this method.
	 * @return TCPSocket object. One can later check if the returned socket object
	 *         is valid or not by calling Socket::IsValid() method on that object.
	 *         - if the socket is valid then it is a newly connected socket, further it can be used to send or receive data.
	 *         - if the socket is invalid then there was no any connections pending, so no connection was accepted.
	 */
	TCPSocket Accept();



#ifdef WIN32
private:
	//override
	void SetWaitingEvents(u32 flagsToWaitFor);
#endif
};//~class TCPServerSocket



class UDPSocket : public Socket{
public:
	UDPSocket(){}



	UDPSocket(const UDPSocket& s) :
			Socket(s)
	{}



	/**
	 * @brief Assignment operator, works similar to std::auto_ptr::operator=().
	 * After this assignment operator completes this socket object refers to the socket the s object referred before, s become invalid.
	 * It works similar to std::auto_ptr::operator=() from standard C++ library.
	 * @param s - socket to assign from.
	 */
	UDPSocket& operator=(const UDPSocket& s){
		this->Socket::operator=(s);
		return *this;
	}


	
	/**
	 * @brief Open the socket.
	 * This method opens the socket, this socket can further be used to send or receive data.
	 * After the socket is opened it becomes a valid socket and Socket::IsValid() will return true for such socket.
	 * After the socket is closed it becomes invalid.
	 * In other words, a valid socket is an opened socket.
	 * In case of errors this method throws net::Exc.
	 * @param port - IP port number on which the socket will listen for incoming datagrams.
	 *				 If 0 is passed then system will assign some free port if any. If there
	 *               are no free ports, then it is an error and an exception will be thrown.
	 * This is useful for server-side sockets, for client-side sockets use UDPSocket::Open().
	 */
	void Open(u16 port);


	
	void Open(){
		this->Open(0);
	}



	/**
	 * @brief Send datagram over UDP socket.
	 * The datagram is sent to UDP socket all at once. If the datagram cannot be
	 * sent at once at the current moment, 0 will be returned.
	 * Note, that underlying protocol limits the maximum size of the datagram,
	 * trying to send the bigger datagram will result in an exception to be thrown.
	 * @param buf - buffer containing the datagram to send.
	 * @param destinationIP - the destination IP address to send the datagram to.
	 * @return number of bytes actually sent. Actually it is either 0 or the size of the
	 *         datagram passed in as argument.
	 */
	size_t Send(const ting::Buffer<u8>& buf, const IPAddress& destinationIP);



	/**
	 * @brief Receive datagram.
	 * Writes a datagram to the given buffer at once if it is available.
	 * If there is no received datagram available a 0 will be returned.
	 * Note, that it will always write out the whole datagram at once. I.e. it is either all or nothing.
	 * Except for the case when the given buffer is not large enough to store the datagram,
	 * in which case the datagram is truncated to the size of the buffer and the rest of the data is lost.
     * @param buf - reference to the buffer the received datagram will be stored to. The buffer
	 *              should be large enough to store the whole datagram. If datagram
	 *              does not fit the passed buffer, then the datagram tail will be truncated
	 *              and this tail data will be lost.
     * @param out_SenderIP - reference to the IP-address structure where the IP-address
	 *                       of the sender will be stored.
     * @return number of bytes stored in the output buffer.
     */
	size_t Recv(ting::Buffer<u8>& buf, IPAddress &out_SenderIP);



#ifdef WIN32
private:
	//override
	void SetWaitingEvents(u32 flagsToWaitFor);
#endif
};//~class UDPSocket


}//~namespace
}//~namespace


/*
 * @mainpage ting::Socket library
 *
 * @section sec_about About
 * <b>tin::Socket</b> is a simple cross platform C++ wrapper above sockets networking API designed for games.
 *
 * @section sec_getting_started Getting started
 * @ref page_usage_tutorial "library usage tutorial" - quick start tutorial
 */

/*
 * @page page_usage_tutorial ting::Socket usage tutorial
 *
 * TODO: write usage tutorial
 */
