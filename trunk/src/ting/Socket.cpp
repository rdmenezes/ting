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



#include <map>
#include <list>

#include "config.hpp"
#include "PoolStored.hpp"
#include "Timer.hpp"
#include "FSFile.hpp"

#include "Socket.hpp"



using namespace ting::net;



ting::IntrusiveSingleton<Lib>::T_Instance Lib::instance;



namespace{
namespace dns{

//forward declaration
struct Resolver;



//After the successful completion the 'p' points to the byte right after the host name.
//In case of unsuccessful completion 'p' is undefined.
std::string ParseHostNameFromDNSPacket(const ting::u8* & p, const ting::u8* end){
	std::string host;
			
	for(;;){
		if(p == end){
			return "";
		}

		ting::u8 len = *p;
		++p;

		if(len == 0){
			break;
		}

		if(host.size() != 0){//if not first label
			host += '.';
		}

		if(end - p < len){
			return "";
		}

		host += std::string(reinterpret_cast<const char*>(p), size_t(len));
		p += len;
		ASSERT(p <= end - 1 || p == end)
	}
//			TRACE(<< "host = " << host << std::endl)
	
	return host;
}



//this mutex is used to protect the dns::thread access.
ting::Mutex mutex;

typedef std::multimap<ting::u32, Resolver*> T_ResolversTimeMap;
typedef T_ResolversTimeMap::iterator T_ResolversTimeIter;

typedef std::map<ting::u16, Resolver*> T_IdMap;
typedef T_IdMap::iterator T_IdIter;

typedef std::list<Resolver*> T_RequestsToSendList;
typedef T_RequestsToSendList::iterator T_RequestsToSendIter;

typedef std::map<HostNameResolver*, ting::Ptr<Resolver> > T_ResolversMap;
typedef T_ResolversMap::iterator T_ResolversIter;



struct Resolver : public ting::PoolStored<Resolver, 10>{
	HostNameResolver* hnr;
	
	std::string hostName; //host name to resolve
	
	T_ResolversTimeMap* timeMap;
	T_ResolversTimeIter timeMapIter;
	
	ting::u16 id;
	T_IdIter idIter;
	
	T_RequestsToSendIter sendIter;
	
	ting::net::IPAddress dns;
};



class LookupThread : public ting::MsgThread{
	ting::net::UDPSocket socket;
	ting::WaitSet waitSet;
	
	T_ResolversTimeMap resolversByTime1, resolversByTime2;
	
public:
	ting::Mutex mutex;//this mutex is used to protect access to members of the thread object.
	
	//this variable is for joining and destroying previous thread object if there was any.
	ting::Ptr<ting::MsgThread> prevThread;
	
	//this is to indicate that the thread is exiting and new DNS lookup requests should be queued to
	//a new thread.
	ting::Inited<volatile bool, true> isExiting;//initially the thread is not running, so set to true
	
	//This variable is for detecting system clock ticks warp around.
	//True if last call to ting::GetTicks() returned value in first half.
	//False otherwise.
	bool lastTicksInFirstHalf;
	
	T_ResolversTimeMap& timeMap1;
	T_ResolversTimeMap& timeMap2;
	
	T_RequestsToSendList sendList;
	
	T_ResolversMap resolversMap;
	T_IdMap idMap;
	
	ting::net::IPAddress dns;
	
	//NOTE: call to this function should be protected by mutex.
	//throws HostNameResolver::TooMuchRequestsExc if all IDs are occupied.
	ting::u16 FindFreeId(){
		if(this->idMap.size() == 0){
			return 0;
		}
		
		if(this->idMap.begin()->first != 0){
			return this->idMap.begin()->first - 1;
		}
		
		if((--(this->idMap.end()))->first != ting::u16(-1)){
			return (--(this->idMap.end()))->first + 1;
		}
		
		T_IdIter i1 = this->idMap.begin();
		T_IdIter i2 = ++this->idMap.begin();
		for(; i2 != this->idMap.end(); ++i1, ++i2){
			if(i2->first - i1->first > 1){
				return i1->first + 1;
			}
		}
		
		throw HostNameResolver::TooMuchRequestsExc();
	}
	
	
	//NOTE: call to this function should be protected by mutex, to make sure the request is not canceled while sending.
	//returns true if request is sent, false otherwise.
	bool SendRequestToDNS(const dns::Resolver* r){
		ting::StaticBuffer<ting::u8, 512> buf; //RFC 1035 limits DNS request UDP packet size to 512 bytes.
		
		size_t packetSize =
				2 + //ID
				2 + //flags
				2 + //Number of questions
				2 + //Number of answers
				2 + //Number of authority records
				2 + //Number of other records
				r->hostName.size() + 2 + //domain name
				2 + //Question type
				2   //Question class
			;
		
		ASSERT(packetSize <= buf.Size())
		
		ting::u8* p = buf.Begin();
		
		//ID
		ting::Serialize16BE(r->id, p);
		p += 2;
		
		//flags
		ting::Serialize16BE(0x100, p);
		p += 2;
		
		//Number of questions
		ting::Serialize16BE(1, p);
		p += 2;
		
		//Number of answers
		ting::Serialize16BE(0, p);
		p += 2;
		
		//Number of authority records
		ting::Serialize16BE(0, p);
		p += 2;
		
		//Number of other records
		ting::Serialize16BE(0, p);
		p += 2;
		
		//domain name
		for(size_t dotPos = 0; dotPos < r->hostName.size();){
			size_t oldDotPos = dotPos;
			dotPos = r->hostName.find('.', dotPos);
			if(dotPos == std::string::npos){
				dotPos = r->hostName.size();
			}
			
			ASSERT(dotPos <= 0xff)
			size_t labelLength = dotPos - oldDotPos;
			ASSERT(labelLength <= 0xff)
			
			*p = ting::u8(labelLength);//save label length
			++p;
			//copy the label bytes
			memcpy(p, r->hostName.c_str() + oldDotPos, labelLength);
			p += labelLength;
			
			++dotPos;
			
			ASSERT(p <= buf.End());
		}
		
		*p = 0; //terminate labels sequence
		++p;
		
		//Question type (1 means A query)
		ting::Serialize16BE(1, p);
		p += 2;
		
		//Question class (1 means inet)
		ting::Serialize16BE(1, p);
		p += 2;
		
		ASSERT(buf.Begin() <= p && p <= buf.End());
		ASSERT(size_t(p - buf.Begin()) == packetSize);
		
		TRACE(<< "sending DNS request to " << (r->dns.host) << std::endl)
		size_t ret = this->socket.Send(ting::Buffer<ting::u8>(buf.Begin(), packetSize), r->dns);
		
		ASSERT(ret == packetSize || ret == 0)
		
//		TRACE(<< "DNS request sent, packetSize = " << packetSize << std::endl)
//#ifdef DEBUG
//		for(unsigned i = 0; i < packetSize; ++i){
//			TRACE(<< int(buf[i]) << std::endl)
//		}
//#endif
		return ret == packetSize;
	}
	
	
	
	//NOTE: call to this function should be protected by mutex
	inline void CallCallback(dns::Resolver* r, ting::net::HostNameResolver::E_Result result, ting::u32 ip = 0)throw(){
		this->mutex.Unlock();
		r->hnr->OnCompleted_ts(result, ip);
		this->mutex.Lock();
	}
	
	
	
	//NOTE: call to this function should be protected by mutex
	//This function will call the Resolver callback.
	void ParseReplyFromDNS(dns::Resolver* r, const ting::Buffer<ting::u8>& buf){
//		TRACE(<< "dns::Resolver::ParseReplyFromDNS(): enter" << std::endl)
//#ifdef DEBUG
//		for(unsigned i = 0; i < buf.Size(); ++i){
//			TRACE(<< int(buf[i]) << std::endl)
//		}
//#endif
		
		if(buf.Size() <
				2 + //ID
				2 + //flags
				2 + //Number of questions
				2 + //Number of answers
				2 + //Number of authority records
				2   //Number of other records
			)
		{
			this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);
			return;
		}
		
		const ting::u8* p = buf.Begin();
		p += 2;//skip ID
		
		{
			ting::u16 flags = ting::Deserialize16BE(p);
			p += 2;
			
			if((flags & 0x8000) == 0){//we expect it to be a response, not query.
				TRACE(<< "ParseReplyFromDNS(): (flags & 0x8000) = " << (flags & 0x8000) << std::endl)
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);
				return;
			}
			
			//Check response code
			if((flags & 0xf) != 0){//0 means no error condition
				if((flags & 0xf) == 3){//name does not exist
					this->CallCallback(r, ting::net::HostNameResolver::NO_SUCH_HOST);
					return;
				}else{
					TRACE(<< "ParseReplyFromDNS(): (flags & 0xf) = " << (flags & 0xf) << std::endl)
					this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);
					return;
				}
			}
		}
		
		{//check number of questions
			ting::u16 numQuestions = ting::Deserialize16BE(p);
			p += 2;
			
			if(numQuestions != 1){
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);
				return;
			}
		}
		
		ting::u16 numAnswers = ting::Deserialize16BE(p);
		p += 2;
		ASSERT(buf.Begin() <= p)
		ASSERT(p <= (buf.End() - 1) || p == buf.End())
		
		if(numAnswers == 0){
			this->CallCallback(r, ting::net::HostNameResolver::NO_SUCH_HOST);
			return;
		}
		
		{
//			ting::u16 nscount = ting::Deserialize16BE(p);
			p += 2;
		}
		
		{
//			ting::u16 arcount = ting::Deserialize16BE(p);
			p += 2;
		}
		
		//parse host name
		{
			std::string host = dns::ParseHostNameFromDNSPacket(p, buf.End());
//			TRACE(<< "host = " << host << std::endl)
			
			if(r->hostName != host){
//				TRACE(<< "this->hostName = " << this->hostName << std::endl)
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//wrong host name for ID.
				return;
			}
		}
		
		//check query type, we sent question type 1 (A query).
		{
			ting::u16 type = ting::Deserialize16BE(p);
			p += 2;
			
			if(type != 1){
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//wrong question type
				return;
			}
		}
		
		//check query class, we sent question class 1 (inet).
		{
			ting::u16 cls = ting::Deserialize16BE(p);
			p += 2;
			
			if(cls != 1){
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//wrong question class
				return;
			}
		}
		
		ASSERT(buf.Overlaps(p) || p == buf.End())
		
		//loop through the answers
		for(ting::u16 n = 0; n != numAnswers; ++n){
			if(p == buf.End()){
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//unexpected end of packet
				return;
			}
			
			//check if there is a domain name or a reference to the domain name
			if(((*p) >> 6) == 0){ //check if two high bits are set
				//skip possible domain name
				for(; p != buf.End() && *p != 0; ++p){
					ASSERT(buf.Overlaps(p))
				}
				if(p == buf.End()){
					this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//unexpected end of packet
					return;
				}
				++p;
			}else{
				//it is a reference to the domain name.
				//skip it
				p += 2;
			}
			
			if(buf.End() - p < 2){
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//unexpected end of packet
				return;
			}
			ting::u16 type = ting::Deserialize16BE(p);
			p += 2;
			
			if(buf.End() - p < 2){
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//unexpected end of packet
				return;
			}
//			ting::u16 cls = ting::Deserialize16(p);
			p += 2;
			
			if(buf.End() - p < 4){
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//unexpected end of packet
				return;
			}
//			ting::u32 ttl = ting::Deserialize32(p);//time till the returned value can be cached.
			p += 4;
			
			if(buf.End() - p < 2){
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//unexpected end of packet
				return;
			}
			ting::u16 dataLen = ting::Deserialize16BE(p);
			p += 2;
			
			if(buf.End() - p < dataLen){
				this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//unexpected end of packet
				return;
			}
			if(type == 1){//'A' type answer
				if(dataLen < 4){
					this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//unexpected end of packet
					return;
				}
				
				ting::u32 address = ting::Deserialize32BE(p);
				this->CallCallback(r, ting::net::HostNameResolver::OK, address);
				TRACE(<< "host resolved: " << r->hostName << " = " << address << std::endl)
				return;
			}
			p += dataLen;
		}
		
		this->CallCallback(r, ting::net::HostNameResolver::DNS_ERROR);//no answer found
	}
	
	
	
private:
	LookupThread() :
			waitSet(2),
			timeMap1(resolversByTime1),
			timeMap2(resolversByTime2)
	{
		ASSERT_INFO(ting::net::Lib::IsCreated(), "ting::net::Lib is not initialized before doing the DNS request")
		
		//Open socket in constructor instead of Run() to catch any possible error
		//with opening the socket before starting the thread.
		this->socket.Open();
	}
public:
	~LookupThread(){
		ASSERT(this->sendList.size() == 0)
		ASSERT(this->resolversMap.size() == 0)
		ASSERT(this->resolversByTime1.size() == 0)
		ASSERT(this->resolversByTime2.size() == 0)
		ASSERT(this->idMap.size() == 0)		
	}
	
	//returns Ptr owning the removed resolver, returns invalid Ptr if there was
	//no such resolver object found.
	//NOTE: call to this function should be protected by mutex.
	ting::Ptr<dns::Resolver> RemoveResolver(HostNameResolver* resolver){
		ting::Ptr<dns::Resolver> r;
		{
			dns::T_ResolversIter i = this->resolversMap.find(resolver);
			if(i == this->resolversMap.end()){
				return r;
			}
			r = i->second;
			this->resolversMap.erase(i);
		}

		//the request is active, remove it from all the maps

		//if the request was not sent yet
		if(r->sendIter != this->sendList.end()){
			this->sendList.erase(r->sendIter);
		}

		r->timeMap->erase(r->timeMapIter);

		this->idMap.erase(r->idIter);
		
		return r;
	}
	
private:
	//NOTE: call to this function should be protected by dns::mutex
	void RemoveAllResolvers(){
		while(this->resolversMap.size() != 0){
			ting::Ptr<dns::Resolver> r = this->RemoveResolver(this->resolversMap.begin()->first);
			ASSERT(r)

			//Notify about timeout. OnCompleted_ts() does not throw any exceptions, so no worries about that.
			this->CallCallback(r.operator->(), HostNameResolver::ERROR, 0);
		}
	}
	
	
	void InitDNS(){
		try{
#if M_OS == M_OS_WIN32 || M_OS == M_OS_WIN64
			struct WinRegKey{
				HKEY	key;
			
				WinRegKey(){
					if(RegOpenKey(
							HKEY_LOCAL_MACHINE,
							"SYSTEM\\ControlSet001\\Services\\Tcpip\\Parameters\\Interfaces",
							&this->key
						) != ERROR_SUCCESS)
					{
						throw ting::Exc("InitDNS(): RegOpenKey() failed");
					}
				}
				
				~WinRegKey(){
					RegCloseKey(this->key);
				}
			} key;
			
			ting::StaticBuffer<char, 256> subkey;//according to MSDN docs maximum key name length is 255 chars.
			
			for(unsigned i = 0; RegEnumKey(key.key, i, subkey.Begin(), subkey.Size()) == ERROR_SUCCESS; ++i){
				HKEY hSub;
				if(RegOpenKey(key.key, subkey.Begin(), &hSub) != ERROR_SUCCESS){
					continue;
				}
				
				ting::StaticBuffer<BYTE, 1024> value;
				
				DWORD len = value.Size();
				
				if(RegQueryValueEx(hSub, "NameServer", 0, NULL, value.Begin(), &len) != ERROR_SUCCESS){
					TRACE(<< "NameServer reading failed " << std::endl)
				}else{
					try{
						std::string str(reinterpret_cast<char*>(value.Begin()));
						size_t spaceIndex = str.find(' ');

						std::string ip = str.substr(0, spaceIndex);
						TRACE(<< "NameServer ip = " << ip << std::endl)
				
						this->dns = ting::net::IPAddress(ip.c_str(), 53);
						RegCloseKey(hSub);
						return;
					}catch(...){}
				}

				len = value.Size();
				if(RegQueryValueEx(hSub, "DhcpNameServer", 0, NULL, value.Begin(), &len) != ERROR_SUCCESS){
					TRACE(<< "DhcpNameServer reading failed " << std::endl)
					RegCloseKey(hSub);
					continue;
				}

				try{
					std::string str(reinterpret_cast<char*>(value.Begin()));
					size_t spaceIndex = str.find(' ');

					std::string ip = str.substr(0, spaceIndex);
					TRACE(<< "DhcpNameServer ip = " << ip << std::endl)
				
					this->dns = ting::net::IPAddress(ip.c_str(), 53);
					RegCloseKey(hSub);
					return;
				}catch(...){}
				RegCloseKey(hSub);
			}

#elif M_OS == M_OS_LINUX || M_OS == M_OS_MACOSX || M_OS == M_OS_SOLARIS
			ting::FSFile f("/etc/resolv.conf");
			
			ting::Array<ting::u8> buf = f.LoadWholeFileIntoMemory(0xfff);//4kb max
			
			for(ting::u8* p = buf.Begin(); p != buf.End(); ++p){
				ting::u8* start = p;
				
				while(p != buf.End() && *p != '\n'){
					++p;
				}
				
				ASSERT(p >= start)
				std::string line(reinterpret_cast<const char*>(start), size_t(p - start));
				if(p == buf.End()){
					--p;
				}
				
				const std::string ns("nameserver ");
				
				size_t nsStart = line.find(ns);
				if(nsStart != 0){
					continue;
				}
				
				size_t ipStart = nsStart + ns.size();
				
				size_t ipEnd = line.find_first_not_of(".0123456789", ipStart);
				
				std::string ipstr = line.substr(ipStart, ipEnd - ipStart);
				
				TRACE(<< "dns ipstr = " << ipstr << std::endl)
				
				try{
					this->dns = ting::net::IPAddress(ipstr.c_str(), 53);
					return;
				}catch(...){}
			}
#else
			TRACE(<< "InitDNS(): don't know how to get DNS IP on this OS" << std::endl)
#endif
		}catch(...){
		}
		this->dns = ting::net::IPAddress(ting::u32(0), 0);
	}
	
	
	void Run(){
		TRACE(<< "DNS lookup thread started" << std::endl)
		
		//destroy previous thread if necessary
		if(this->prevThread){
			//NOTE, if the thread was not started due to some error during adding its
			//first DNS lookup request it is OK to call Join() on such not
			//started thread.
			this->prevThread->Join();
			this->prevThread.Reset();
			TRACE(<< "Previous thread destroyed" << std::endl)
		}
		
		this->InitDNS();
		
		TRACE(<< "this->dns.host = " << this->dns.host << std::endl)

		this->waitSet.Add(&this->queue, ting::Waitable::READ);
		this->waitSet.Add(&this->socket, ting::Waitable::READ);
		
		while(!this->quitFlag){
			ting::u32 timeout;
			{
				ting::Mutex::Guard mutexGuard(this->mutex);
				
				if(this->socket.ErrorCondition()){
					this->isExiting = true;
					this->RemoveAllResolvers();
					break;//exit thread
				}

				if(this->socket.CanRead()){
					TRACE(<< "can read" << std::endl)
					try{
						ting::StaticBuffer<ting::u8, 512> buf;//RFC 1035 limits DNS request UDP packet size to 512 bytes. So, no need to allocate bigger buffer.
						ting::net::IPAddress address;
						size_t ret = this->socket.Recv(buf, address);
						
						ASSERT(ret != 0)
						ASSERT(ret <= buf.Size())
						if(ret >= 13){//at least there should be standard header and host name, otherwise ignore received UDP packet
							ting::u16 id = ting::Deserialize16BE(buf.Begin());
							
							T_IdIter i = this->idMap.find(id);
							if(i != this->idMap.end()){
								ASSERT(id == i->second->id)
								
								//check by host name also
								const ting::u8* p = buf.Begin() + 12;//start of the host name
								std::string host = dns::ParseHostNameFromDNSPacket(p, buf.End());
								
								if(host == i->second->hostName){
									ting::Ptr<dns::Resolver> r = this->RemoveResolver(i->second->hnr);
									this->ParseReplyFromDNS(r.operator->(), ting::Buffer<ting::u8>(buf.Begin(), ret));//this function will call the callback
								}
							}
						}
					}catch(ting::net::Exc& e){
						this->isExiting = true;
						this->RemoveAllResolvers();
						break;//exit thread
					}
				}

//				TRACE(<< "this->sendList.size() = " << (this->sendList.size()) << std::endl)
//Workaround for strange bug on Win32 (reproduced on WinXP at least).
//For some reason waiting for WRITE on UDP socket does not work. It hangs in the
//Wait() method until timeout is hit. So, just try to send data to the socket without waiting for WRITE.
#if M_OS == M_OS_WIN32 || M_OS == M_OS_WIN64
				if(this->sendList.size() != 0)
#else
				if(this->socket.CanWrite())
#endif
				{
					TRACE(<< "can write" << std::endl)
					//send request
					ASSERT(this->sendList.size() > 0)
					
					try{
						while(this->sendList.size() != 0){
							dns::Resolver* r = this->sendList.front();
							if(r->dns.host == 0){
								r->dns = this->dns;
							}

							if(r->dns.host != 0){
								if(!this->SendRequestToDNS(r)){
									TRACE(<< "request not sent" << std::endl)
									break;//socket is not ready for sending, go out of requests sending loop.
								}
								TRACE(<< "request sent" << std::endl)
								r->sendIter = this->sendList.end();//end() value will indicate that the request has already been sent
								this->sendList.pop_front();
							}else{
								ting::Ptr<dns::Resolver> removedResolver = this->RemoveResolver(r->hnr);
								ASSERT(removedResolver)

								//Notify about error. OnCompleted_ts() does not throw any exceptions, so no worries about that.
								this->CallCallback(removedResolver.operator->(), HostNameResolver::ERROR, 0);
							}
						}
					}catch(ting::net::Exc& e){
						this->isExiting = true;
						this->RemoveAllResolvers();
						break;//exit thread
					}
					
					if(this->sendList.size() == 0){
						//move socket to waiting for READ condition only
						this->waitSet.Change(&this->socket, ting::Waitable::READ);
						TRACE(<< "socket wait mode changed to read only" << std::endl)
					}
				}
				
				ting::u32 curTime = ting::GetTicks();
				{//check if time has warped around and it is necessary to swap time maps
					bool isFirstHalf = curTime < (ting::u32(-1) / 2);
					if(isFirstHalf && !this->lastTicksInFirstHalf){
						//Time warped.
						//Timeout all requests from first time map
						while(this->timeMap1.size() != 0){
							ting::Ptr<dns::Resolver> r = this->RemoveResolver(this->timeMap1.begin()->second->hnr);
							ASSERT(r)

							//Notify about timeout. OnCompleted_ts() does not throw any exceptions, so no worries about that.
							this->CallCallback(r.operator->(), HostNameResolver::TIMEOUT, 0);
						}
						
						ASSERT(this->timeMap1.size() == 0)
						std::swap(this->timeMap1, this->timeMap2);
					}
					this->lastTicksInFirstHalf = isFirstHalf;
				}
				
				while(this->timeMap1.size() != 0){
					if(this->timeMap1.begin()->first > curTime){
						break;
					}
					
					//timeout
					ting::Ptr<dns::Resolver> r = this->RemoveResolver(this->timeMap1.begin()->second->hnr);
					ASSERT(r)
					
					//Notify about timeout. OnCompleted_ts() does not throw any exceptions, so no worries about that.
					this->CallCallback(r.operator->(), HostNameResolver::TIMEOUT, 0);
				}
				
				if(this->resolversMap.size() == 0){
					this->isExiting = true;
					break;//exit thread
				}
				
				ASSERT(this->timeMap1.size() > 0)
				ASSERT(this->timeMap1.begin()->first > curTime)
				
//				TRACE(<< "DNS thread: curTime = " << curTime << std::endl)
//				TRACE(<< "DNS thread: this->timeMap1.begin()->first = " << (this->timeMap1.begin()->first) << std::endl)
				
				timeout = this->timeMap1.begin()->first - curTime;
			}
			
			//Make sure that ting::GetTicks is called at least 4 times per full time warp around cycle.
			ting::ClampTop(timeout, ting::u32(-1) / 4);
			
//Workaround for strange bug on Win32 (reproduced on WinXP at least).
//For some reason waiting for WRITE on UDP socket does not work. It hangs in the
//Wait() method until timeout is hit. So, just check every 100ms if it is OK to write to UDP socket.
#if M_OS == M_OS_WIN32 || M_OS == M_OS_WIN64
			if(this->sendList.size() > 0){
				ting::ClampTop(timeout, ting::u32(100));
			}
#endif
			
			TRACE(<< "DNS thread: waiting with timeout = " << timeout << std::endl)
			if(this->waitSet.WaitWithTimeout(timeout) == 0){
				//no Waitables triggered
//				TRACE(<< "timeout hit" << std::endl)
				continue;
			}
			
			if(this->queue.CanRead()){
				while(ting::Ptr<ting::Message> m = this->queue.PeekMsg()){
					m->Handle();
				}
			}			
		}//~while(!this->quitFlag)
		
		this->waitSet.Remove(&this->socket);
		this->waitSet.Remove(&this->queue);
		TRACE(<< "DNS lookup thread stopped" << std::endl)
	}
	
public:
	class StartSendingMessage : public ting::Message{
		LookupThread* thr;
	public:
		StartSendingMessage(LookupThread* thr) :
				thr(ASS(thr))
		{}
		
		//override
		void Handle(){
			this->thr->waitSet.Change(&this->thr->socket, ting::Waitable::READ_AND_WRITE);
			TRACE(<< "socket wait mode changed to read and write" << std::endl)
		}
	};
	
	static inline ting::Ptr<LookupThread> New(){
		return ting::Ptr<LookupThread>(new LookupThread());
	}
};

//accessing this variable must be protected by dnsMutex
ting::Ptr<LookupThread> thread;



}//~namespace
}//~namespace



HostNameResolver::~HostNameResolver(){
#ifdef DEBUG
	//check that there is no ongoing DNS lookup operation.
	ting::Mutex::Guard mutexGuard(dns::mutex);
	
	if(dns::thread.IsValid()){
		ting::Mutex::Guard mutexGuard(dns::thread->mutex);
		
		dns::T_ResolversIter i = dns::thread->resolversMap.find(this);
		if(i != dns::thread->resolversMap.end()){
			ASSERT_INFO_ALWAYS(false, "trying to destroy the HostNameResolver object while DNS lookup request is in progress, call HostNameResolver::Cancel_ts() first.")
		}
	}
#endif
}



void HostNameResolver::Resolve_ts(const std::string& hostName, ting::u32 timeoutMillis, const ting::net::IPAddress& dnsIP){
//	TRACE(<< "HostNameResolver::Resolve_ts(): enter" << std::endl)
	
	ASSERT(ting::net::Lib::IsCreated())
	
	if(hostName.size() > 253){
		throw DomainNameTooLongExc();
	}
	
	ting::Mutex::Guard mutexGuard(dns::mutex);
	
	bool needStartTheThread = false;
	
	//check if thread is created
	if(dns::thread.IsNotValid()){
		dns::thread = dns::LookupThread::New();
		needStartTheThread = true;
	}else{
		ting::Mutex::Guard mutexGuard(dns::thread->mutex);
		
		//check if already in progress
		if(dns::thread->resolversMap.find(this) != dns::thread->resolversMap.end()){
			throw AlreadyInProgressExc();
		}

		//Thread is created, check if it is running.
		//If there are active requests then the thread must be running.
		if(dns::thread->isExiting == true){
			ting::Ptr<dns::LookupThread> t = dns::LookupThread::New();
			t->prevThread = dns::thread;
			dns::thread = t;
			needStartTheThread = true;
		}
	}
	
	ASSERT(dns::thread.IsValid())
	
	ting::Ptr<dns::Resolver> r(new dns::Resolver());
	r->hnr = this;
	r->hostName = hostName;
	r->dns = dnsIP;
	
	ting::Mutex::Guard mutexGuard2(dns::thread->mutex);
	
	//Find free ID, it will throw TooMuchRequestsExc if there are no free IDs
	{
		r->id = dns::thread->FindFreeId();
		std::pair<dns::T_IdIter, bool> res =
				dns::thread->idMap.insert(std::pair<ting::u16, dns::Resolver*>(r->id, r.operator->()));
		ASSERT(res.second)
		r->idIter = res.first;
	}
	
	//calculate time
	ting::u32 curTime = ting::GetTicks();
	{
		ting::u32 endTime = curTime + timeoutMillis;
//		TRACE(<< "HostNameResolver::Resolve_ts(): curTime = " << curTime << std::endl)
//		TRACE(<< "HostNameResolver::Resolve_ts(): endTime = " << endTime << std::endl)
		if(endTime < curTime){//if warped around
			r->timeMap = &dns::thread->timeMap2;
		}else{
			r->timeMap = &dns::thread->timeMap1;
		}
		try{
			r->timeMapIter = r->timeMap->insert(std::pair<ting::u32, dns::Resolver*>(endTime, r.operator->()));
		}catch(...){
			dns::thread->idMap.erase(r->idIter);
			throw;
		}
	}
	
	//add resolver to send queue
	try{
		dns::thread->sendList.push_back(r.operator->());
	}catch(...){
		r->timeMap->erase(r->timeMapIter);
		dns::thread->idMap.erase(r->idIter);
		throw;
	}
	r->sendIter = --dns::thread->sendList.end();
	
	//insert the resolver to main resolvers map
	try{
		dns::thread->resolversMap[this] = r;
	
		//If there was no send requests in the list, send the message to the thread to switch
		//socket to wait for sending mode.
		if(dns::thread->sendList.size() == 1){
			dns::thread->PushMessage(
					ting::Ptr<dns::LookupThread::StartSendingMessage>(
							new dns::LookupThread::StartSendingMessage(dns::thread.operator->())
						)
				);
		}

		//Start the thread if we created the new one.
		if(needStartTheThread){
			dns::thread->lastTicksInFirstHalf = curTime < (ting::u32(-1) / 2);
			dns::thread->Start();
			dns::thread->isExiting = false;//thread has just started, clear the exiting flag
	//		TRACE(<< "HostNameResolver::Resolve_ts(): thread started" << std::endl)
		}
	}catch(...){
		dns::thread->resolversMap.erase(this);
		dns::thread->sendList.pop_back();
		r->timeMap->erase(r->timeMapIter);
		dns::thread->idMap.erase(r->idIter);
		throw;
	}
}



bool HostNameResolver::Cancel_ts(){
	ting::Mutex::Guard mutexGuard(dns::mutex);
	
	if(dns::thread.IsNotValid()){
		return false;
	}
	
	ting::Mutex::Guard mutexGuard2(dns::thread->mutex);
	
	bool ret = dns::thread->RemoveResolver(this).IsValid();
	
	if(dns::thread->resolversMap.size() == 0 && ret){
		dns::thread->PushQuitMessage();
	}
	
	return ret;
}



Lib::Lib(){
#ifdef WIN32
	WORD versionWanted = MAKEWORD(2,2);
	WSADATA wsaData;
	if(WSAStartup(versionWanted, &wsaData) != 0 ){
		throw net::Exc("SocketLib::SocketLib(): Winsock 2.2 initialization failed");
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



Lib::~Lib(){
	//check that there are no active dns lookups and finish the DNS request thread
	{
		ting::Mutex::Guard mutexGuard(dns::mutex);
		
		if(dns::thread.IsValid()){
			dns::thread->PushPreallocatedQuitMessage();
			dns::thread->Join();
			
			ASSERT_INFO(dns::thread->resolversMap.size() == 0, "There are active DNS requests upon Sockets library de-initialization, all active DNS requests must be canceled before that.")
			
			dns::thread.Reset();
		}
	}
	
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



void Socket::Close() throw(){
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
	if(this == &s){//detect self-assignment
		return *this;
	}

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
	if(!this->IsValid()){
		throw net::Exc("Socket::DisableNaggle(): socket is not valid");
	}

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
	if(!this->IsValid()){
		throw net::Exc("Socket::SetNonBlockingMode(): socket is not valid");
	}

#if defined(__linux__) || defined(__APPLE__)
	{
		int flags = fcntl(this->socket, F_GETFL, 0);
		if(flags == -1){
			throw net::Exc("Socket::SetNonBlockingMode(): fcntl(F_GETFL) failed");
		}
		if(fcntl(this->socket, F_SETFL, flags | O_NONBLOCK) != 0){
			throw net::Exc("Socket::SetNonBlockingMode(): fcntl(F_SETFL) failed");
		}
	}
#elif defined(WIN32)
	{
		u_long mode = 1;
		if(ioctlsocket(this->socket, FIONBIO, &mode) != 0){
			throw net::Exc("Socket::SetNonBlockingMode(): ioctlsocket(FIONBIO) failed");
		}
	}
#else
#error "Unsupported OS"
#endif
}



ting::u16 Socket::GetLocalPort(){
	if(!this->IsValid()){
		throw net::Exc("Socket::GetLocalPort(): socket is not valid");
	}

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
		throw net::Exc("Socket::GetLocalPort(): getsockname() failed");
	}

	return ting::u16(ntohs(addr.sin_port));
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
		throw net::Exc("Socket::CheckSignalled(): WSAEnumNetworkEvents() failed");
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
	//if some event occurred then some of readiness flags should be set
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
		throw net::Exc("Socket::CreateEventForWaitable(): could not create event (Win32) for implementing Waitable");
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
		throw net::Exc("Socket::SetWaitingEventsForWindows(): could not associate event (Win32) with socket");
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



namespace{

//This function modifies the pointer passed as argument (reference to a pointer).
//After successful completion of the function the pointer passed as argument
//points to the character which goes right after the IP address.
ting::u32 ParseIPAddressString(const char*& p){
	ting::u32 h = 0;//parsed host

	for(unsigned t = 0; t < 4; ++t){
		unsigned digits[3];
		unsigned numDgts;
		for(numDgts = 0; numDgts < 3; ++numDgts, ++p){
			if(*p < '0' || '9' < *p){
				if(numDgts == 0){
					throw ting::net::IPAddress::BadIPAddressFormatExc();
				}
				break;
			}
			digits[numDgts] = unsigned(*p) - unsigned('0');
		}

		if(t < 3){
			if(*p != '.'){//unexpected non-delimiter character
				throw ting::net::IPAddress::BadIPAddressFormatExc();
			}
			++p;
		}else{
			ASSERT(t == 3)
		}

		unsigned xxx = 0;
		for(unsigned i = 0; i < numDgts; ++i){
			unsigned ord = 1;
			for(unsigned j = 1; j < numDgts - i; ++j){
			   ord *= 10;
			}
			xxx += digits[i] * ord;
		}
		if(xxx > 0xff){
			throw ting::net::IPAddress::BadIPAddressFormatExc();
		}

		h |= (xxx << (8 * (3 - t)));
	}
	
	return h;
}

}//~namespace



IPAddress::IPAddress(const char* ip, u16 p) :
		host(ParseIPAddressString(ip)),
		port(p)
{}



IPAddress::IPAddress(const char* ip) :
		host(ParseIPAddressString(ip))
{
	if(*ip != ':'){
//		TRACE(<< "no colon, *ip = " << (*ip) << std::endl)
		throw ting::net::IPAddress::BadIPAddressFormatExc();
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



void TCPSocket::Open(const IPAddress& ip, bool disableNaggle){
	if(this->IsValid()){
		throw net::Exc("TCPSocket::Open(): socket already opened");
	}

	//create event for implementing Waitable
#ifdef WIN32
	this->CreateEventForWaitable();
#endif

	this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if(this->socket == DInvalidSocket()){
#ifdef WIN32
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
			throw net::Exc(ss.str());
		}
	}
}



size_t TCPSocket::Send(const ting::Buffer<u8>& buf, size_t offset){
	if(!this->IsValid()){
		throw net::Exc("TCPSocket::Send(): socket is not opened");
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
				throw net::Exc(ss.str());
			}
		}
		break;
	}//~while

	ASSERT(len >= 0)
	return size_t(len);
}



void TCPSocket::SendAll(const ting::Buffer<u8>& buf){
	if(!this->IsValid()){
		throw net::Exc("TCPSocket::Send(): socket is not opened");
	}

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

	if(!this->IsValid()){
		throw net::Exc("TCPSocket::Send(): socket is not opened");
	}

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
				throw net::Exc(ss.str());
			}
		}
		break;
	}//~while

	ASSERT(len >= 0)
	return size_t(len);
}



IPAddress TCPSocket::GetLocalAddress(){
	if(!this->IsValid()){
		throw net::Exc("Socket::GetLocalPort(): socket is not valid");
	}

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
		throw net::Exc("Socket::GetLocalPort(): getsockname() failed");
	}

	return IPAddress(
			u32(ntohl(addr.sin_addr.s_addr)),
			u16(ntohs(addr.sin_port))
		);
}



IPAddress TCPSocket::GetRemoteAddress(){
	if(!this->IsValid()){
		throw net::Exc("TCPSocket::GetRemoteAddress(): socket is not valid");
	}

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
		throw net::Exc("TCPSocket::GetRemoteAddress(): getpeername() failed");
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
	if(this->IsValid()){
		throw net::Exc("TCPServerSocket::Open(): socket already opened");
	}

	this->disableNaggle = disableNaggle;

#ifdef WIN32
	this->CreateEventForWaitable();
#endif

	this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if(this->socket == DInvalidSocket()){
#ifdef WIN32
		this->CloseEventForWaitable();
#endif
		throw net::Exc("TCPServerSocket::Open(): Couldn't create socket");
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
		throw net::Exc("TCPServerSocket::Open(): Couldn't bind to local port");
	}

	if(listen(this->socket, int(queueLength)) == DSocketError()){
		this->Close();
		throw net::Exc("TCPServerSocket::Open(): Couldn't listen to local port");
	}

	this->SetNonBlockingMode();
}



TCPSocket TCPServerSocket::Accept(){
	if(!this->IsValid()){
		throw net::Exc("TCPServerSocket::Accept(): the socket is not opened");
	}

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

	if(sock.socket == DInvalidSocket()){
		return sock;//no connections to be accepted, return invalid socket
	}

#ifdef WIN32
	sock.CreateEventForWaitable();

	//NOTE: accepted socket is associated with the same event object as the listening socket which accepted it.
	//Re-associate the socket with its own event object.
	sock.SetWaitingEvents(0);
#endif

	sock.SetNonBlockingMode();

	if(this->disableNaggle){
		sock.DisableNaggle();
	}

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
	if(this->IsValid()){
		throw net::Exc("UDPSocket::Open(): the socket is already opened");
	}

#ifdef WIN32
	this->CreateEventForWaitable();
#endif

	this->socket = ::socket(AF_INET, SOCK_DGRAM, 0);
	if(this->socket == DInvalidSocket()){
#ifdef WIN32
		this->CloseEventForWaitable();
#endif
		throw net::Exc("UDPSocket::Open(): ::socket() failed");
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
			throw net::Exc("UDPSocket::Open(): could not bind to local port");
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
			throw net::Exc("UDPSocket::Open(): failed setting broadcast option");
		}
	}
#else
#error "Unsupported OS"
#endif

	this->ClearAllReadinessFlags();
}



size_t UDPSocket::Send(const ting::Buffer<u8>& buf, const IPAddress& destinationIP){
	if(!this->IsValid()){
		throw net::Exc("UDPSocket::Send(): socket is not opened");
	}

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



size_t UDPSocket::Recv(ting::Buffer<u8>& buf, IPAddress &out_SenderIP){
	if(!this->IsValid()){
		throw net::Exc("UDPSocket::Recv(): socket is not opened");
	}

	//The "can read" flag shall be cleared even if this function fails.
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
				throw net::Exc(ss.str());
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
