// Author: Ivan Gagis <igagis@gmail.com>
// Version: 1

// Description:
//      Singleton base class (template)

#ifndef M_Singleton_HPP
#define M_Singleton_HPP

#include <ting/gendef.hpp>

namespace igagis{

template <class T> class Singleton{
	
	inline static T*& StaticMemoryBlock(){
		static T* instance = 0;
		return instance;
	};

protected://use only as a base class
	Singleton(){
		if(Singleton::StaticMemoryBlock() != 0)
			throw ting::Exc("Singleton::Singleton(): instance is already created");

		Singleton::StaticMemoryBlock() = static_cast<T*>(this);
	};

public:
	inline static T& Inst(){
		ASSERT(Singleton::StaticMemoryBlock())
		return *(Singleton::StaticMemoryBlock());
	};

	~Singleton(){
		ASSERT(Singleton::StaticMemoryBlock() == static_cast<T*>(this))
		Singleton::StaticMemoryBlock() = 0;
	};
};

};//~namespace igagis
#endif//~once
