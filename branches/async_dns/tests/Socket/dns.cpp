#include "../../src/ting/Socket.hpp"

#include "dns.hpp"



namespace TestSimpleDNSLookup{

class Resolver : public ting::net::HostNameResolver{
	
public:
	
	ting::u32 ip;
	
	ting::Semaphore* sema;
	
	E_Result result;
	
	//override
	void OnCompleted_ts(E_Result result, ting::u32 ip)throw(){
//		ASSERT_INFO_ALWAYS(result == ting::net::HostNameResolver::OK, "result = " << result)
		this->result = result;
		
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
	
	ASSERT_INFO_ALWAYS(r.result == ting::net::HostNameResolver::OK, "r.result = " << r.result)
	
	ting::net::IPAddress ip(r.ip, 0);
	
	TRACE(<< "ip = " << ip.host << std::endl)
	
	//TODO:
}
}