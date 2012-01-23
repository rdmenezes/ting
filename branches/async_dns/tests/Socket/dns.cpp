#include "../../src/ting/Socket.hpp"

#include "dns.hpp"



namespace TestSimpleDNSLookup{

class Resolver : public ting::net::HostNameResolver{
	
public:
	
	Resolver(ting::Semaphore& sema) :
			sema(sema)
	{}
	
	ting::u32 ip;
	
	ting::Semaphore& sema;
	
	E_Result result;
	
	//override
	void OnCompleted_ts(E_Result result, ting::u32 ip)throw(){
//		ASSERT_INFO_ALWAYS(result == ting::net::HostNameResolver::OK, "result = " << result)
		this->result = result;
		
		this->ip = ip;
		
		this->sema.Signal();
	}
};

void Run(){
	{//test one resolve at a time
		ting::Semaphore sema;

		Resolver r(sema);

		r.Resolve_ts("ya.ru", 10000);

		if(!sema.Wait(11000)){
			ASSERT_ALWAYS(false)
		}

		ASSERT_INFO_ALWAYS(r.result == ting::net::HostNameResolver::OK, "r.result = " << r.result)

		ASSERT_INFO_ALWAYS(r.ip == 0x4D581503, "r.ip = " << r.ip)

	//	TRACE(<< "ip = " << ip.host << std::endl)
	}
	
	{//test several resolves at a time
		ting::Semaphore sema;

		typedef std::vector<ting::Ptr<Resolver> > T_ResolverList;
		typedef T_ResolverList::iterator T_ResolverIter;
		T_ResolverList r;

		for(unsigned i = 0; i < 100; ++i){
			r.push_back(ting::Ptr<Resolver>(
					new Resolver(sema)
				));
		}
		
//		TRACE(<< "starting resolutions" << std::endl)
		
		for(T_ResolverIter i = r.begin(); i != r.end(); ++i){
			(*i)->Resolve_ts("ya.ru", 5000);
		}
		
		for(unsigned i = 0; i < r.size(); ++i){
			if(!sema.Wait(6000)){
				ASSERT_ALWAYS(false)
			}
		}
//		TRACE(<< "resolutions done" << std::endl)
		
		for(T_ResolverIter i = r.begin(); i != r.end(); ++i){
			ASSERT_ALWAYS((*i)->result == ting::net::HostNameResolver::OK)
			ASSERT_ALWAYS((*i)->ip == 0x4D581503)
		}
	}
}

}