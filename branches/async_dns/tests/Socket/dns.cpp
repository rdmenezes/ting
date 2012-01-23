#include "../../src/ting/Socket.hpp"

#include "dns.hpp"



namespace TestSimpleDNSLookup{

class Resolver : public ting::net::HostNameResolver{
	
public:
	
	ting::u32 ip;
	
	ting::Semaphore* sema;
	
	//override
	void OnCompleted_ts(E_Result result, ting::u32 ip)throw(){
		ASSERT_ALWAYS(result == ting::net::HostNameResolver::OK)
		
		this->ip = ip;
		
		this->sema->Signal();
	}
};

void Run(){
	
	ting::Semaphore sema;
	
	Resolver r;
	r.sema = &sema;
	
	r.Resolve_ts("ya.ru", 10000);
	
	if(!sema.Wait(11000)){
		ASSERT_ALWAYS(false)
	}
	
	ting::net::IPAddress ip(r.ip, 0);
	
	TRACE(<< "ip = " << ip.host << std::endl)
	
	//TODO:
}
}