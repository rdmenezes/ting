/* The MIT License:

Copyright (c) 2008 Ivan Gagis

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

// ting 0.3
// Homepage: http://code.google.com/p/ting
// Author: Ivan Gagis <igagis@gmail.com>

// File description:
//	Templates to implement signal-slot callback technique

#pragma once

#include <vector>
#include "codegen.h"
#include "Ref.hpp"

#define M_MAX_NUM_SIG_PARAMETERS 10

//output ", class T_Pn", the comma is written only if n==0
#define M_TEMPLATE_PARAM(n, a) M_COMMA_IF_NOT_0(n) class T_P##n

//output "class T_P0, ... class T_Pn"
#define M_TEMPLATE_PARAMS(n) M_REPEAT1(n, M_TEMPLATE_PARAM, )

//output "template <class T_P0, ... class T_Pn>"
#define M_TEMPLATE(n) M_IF_NOT_0(n, template <, ) M_TEMPLATE_PARAMS(n) M_IF_NOT_0(n, >, )



//output ", TPn pn", the comma is written only if n==0
#define M_FUNC_PARAM_FULL(n, a) M_COMMA_IF_NOT_0(n) T_P##n p##n

//output "T_P0 p0, ... T_Pn pn"
#define M_FUNC_PARAMS_FULL(n) M_REPEAT1(n, M_FUNC_PARAM_FULL, )


#define M_FUNC_PARAM_TYPE(n, a) M_COMMA_IF_NOT_0(n) T_P##n

//output "T_P0, ... T_Pn"
#define M_FUNC_PARAM_TYPES(n) M_REPEAT1(n, M_FUNC_PARAM_TYPE, )
//#define M_FUNC_PARAM_TYPES(n) M_FUNC_PARAM_TYPES_I(n)

//output ", pn", the comma is written only if n==0
#define M_FUNC_PARAM_NAME(n, a) M_COMMA_IF_NOT_0(n) p##n

//output "p0, ... pn"
#define M_FUNC_PARAM_NAMES(n) M_REPEAT1(n, M_FUNC_PARAM_NAME, )


//output the template for method slot with n parameters
#define M_METHOD_SLOT(num_meth_params, num_sig_params) \
template <class T_Ob, class T_Ret> class C_MethodSlot##num_meth_params : public C_SlotLink{ \
	T_Ob* o; \
	T_Ret(T_Ob::*m)(M_FUNC_PARAM_TYPES(num_meth_params)); \
  public: \
	C_MethodSlot##num_meth_params(T_Ob* object, T_Ret(T_Ob::*method)(M_FUNC_PARAM_TYPES(num_meth_params))) : \
			o(object), \
			m(method) \
	{} \
	void Execute(M_FUNC_PARAMS_FULL(num_sig_params)){ \
		(this->o->*(this->m))(M_FUNC_PARAM_NAMES(num_meth_params)); \
	} \
};
//#define M_METHOD_SLOT(num_meth_params, num_sig_params) M_METHOD_SLOT_I(num_meth_params, num_sig_params)

#define M_FUNC_SLOT(num_func_params, num_sig_params) \
template <class T_Ret> class C_FuncSlot##num_func_params : public C_SlotLink{ \
	T_Ret(*f)(M_FUNC_PARAM_TYPES(num_func_params)); \
  public: \
	C_FuncSlot##num_func_params(T_Ret(*function)(M_FUNC_PARAM_TYPES(num_func_params))) : \
			f(function) \
	{} \
	void Execute(M_FUNC_PARAMS_FULL(num_sig_params)){ \
		(*this->f)(M_FUNC_PARAM_NAMES(num_func_params)); \
	} \
};

//output the template for method slot for C_WeakRef object with n parameters
#define M_METHOD_WEAKREF_SLOT(num_meth_params, num_sig_params) \
template <class T_Ob, class T_Ret> class C_WeakRefMethodSlot##num_meth_params : public C_SlotLink{ \
	WeakRef<T_Ob> o; \
	T_Ret(T_Ob::*m)(M_FUNC_PARAM_TYPES(num_meth_params)); \
  public: \
	C_WeakRefMethodSlot##num_meth_params(WeakRef<T_Ob>& object, T_Ret(T_Ob::*method)(M_FUNC_PARAM_TYPES(num_meth_params))) : \
			o(object), \
			m(method) \
	{} \
	void Execute(M_FUNC_PARAMS_FULL(num_sig_params)){ \
		if(Ref<T_Ob> r = this->o) \
			(r.operator->()->*(this->m))(M_FUNC_PARAM_NAMES(num_meth_params)); \
	} \
};

//method and pointer to object Connect
#define M_CONNECT_METH(num_meth_params, unused) \
template <class T_Ob, class T_Ret> void Connect(T_Ob* o, T_Ret(T_Ob::*m)(M_FUNC_PARAM_TYPES(num_meth_params))){ \
	Ptr<C_SlotLink> sl( \
			static_cast<C_SlotLink*>(new C_MethodSlot##num_meth_params<T_Ob, T_Ret>(o, m)) \
		); \
	this->slotLink.push_back(sl); \
}

//func Connect
#define M_CONNECT_FUNC(num_func_params, unused) \
template <class T_Ret> void Connect(T_Ret(*f)(M_FUNC_PARAM_TYPES(num_func_params))){ \
	Ptr<C_SlotLink> sl( \
			static_cast<C_SlotLink*>( new C_FuncSlot##num_func_params<T_Ret>(f)) \
		); \
	this->slotLink.push_back(sl); \
}

//Weak ref and method Connect
#define M_CONNECT_METH_WEAKREF(num_meth_params, unused) \
template <class T_Ob, class T_Ret> void Connect(WeakRef<T_Ob>& o, T_Ret(T_Ob::*m)(M_FUNC_PARAM_TYPES(num_meth_params))){ \
	Ptr<C_SlotLink> sl( \
			static_cast<C_SlotLink*>(new C_WeakRefMethodSlot##num_meth_params<T_Ob, T_Ret>(o, m)) \
		); \
	this->slotLink.push_back(sl); \
}

//
// M   M        SSSS IIIII  GGG  N   N  AAA  L
// MM MM       S       I   G     NN  N A   A L
// M M M        SSS    I   G  GG N N N A   A L
// M   M           S   I   G   G N  NN AAAAA L
// M   M _____ SSSS  IIIII  GGG  N   N A   A LLLLL
//
//output a template for a signal with n parameters
#define M_SIGNAL(n, unused) \
M_TEMPLATE(n) class Signal##n{ \
	class C_SlotLink{ \
	  public: \
		virtual ~C_SlotLink(){}; \
		virtual void Execute(M_FUNC_PARAMS_FULL(n)) = 0; \
	}; \
\
	M_REPEAT2(M_INCREMENT(n), M_METHOD_SLOT, n)\
	M_REPEAT2(M_INCREMENT(n), M_FUNC_SLOT, n) \
	M_REPEAT2(M_INCREMENT(n), M_METHOD_WEAKREF_SLOT, n) \
\
	typedef std::vector<Ptr<C_SlotLink> > C_SlotLinkVector; \
	C_SlotLinkVector slotLink; \
\
  public: \
	void Emit(M_FUNC_PARAMS_FULL(n)){ \
		for(uint i = 0; i < this->slotLink.size(); ++i) \
			this->slotLink[i]->Execute(M_FUNC_PARAM_NAMES(n)); \
	} \
	M_REPEAT2(M_INCREMENT(n), M_CONNECT_METH, ) \
	M_REPEAT2(M_INCREMENT(n), M_CONNECT_FUNC, ) \
	M_REPEAT2(M_INCREMENT(n), M_CONNECT_METH_WEAKREF, ) \
\
	void DisconnectAll(){ \
		this->slotLink.clear(); \
	} \
};


namespace ting{

M_REPEAT3(M_MAX_NUM_SIG_PARAMETERS, M_SIGNAL, )
//M_SIGNAL(0)

//M_SIGNAL(1)
//signal with 1 parameter
//template <class T_P0> class C_Signal1{
//M_TEMPLATE(1) class C_Signal1{
//    class C_SlotLink{
//      public:
//        virtual void Execute(M_FUNC_PARAMS_FULL(1))=0;
//        virtual ~C_SlotLink(){};
//    };
//    
//    M_REPEAT2(M_INCREMENT(1), M_METHOD_SLOT, 1)
//    //M_METHOD_SLOT(0, 1)
//    //M_METHOD_SLOT(1, 1)
////    template <class T_Ob, class T_Ret> class C_MethodSlot0 : public C_SlotLink{
////        T_Ob* o;
////        T_Ret(T_Ob::*m)();
////      public:
////        C_MethodSlot0(T_Ob* object, T_Ret(T_Ob::*method)()) : o(object), m(method) {};
////
////        //override
////        void Execute(){ (o->*m)(); };
////    };
//    
////    template <class T_Ob, class T_Ret> class C_MethodSlot1 : public C_SlotLink{
////        T_Ob* o;
////        T_Ret(T_Ob::*m)(T_P0);
////      public:
////        C_MethodSlot1(T_Ob* object, T_Ret(T_Ob::*method)(T_P0)) : o(object), m(method) {};
////
////        //override
////        void Execute(T_P0 p0){ (o->*m)(p0); };
////    };
//
//    //M_FUNC_SLOT(0, 1)
//    //M_FUNC_SLOT(1, 1)
//    M_REPEAT2(2, M_FUNC_SLOT, 1)
//    
////    template <class T_Ret> class C_FuncSlot0 : public C_SlotLink{
////        T_Ret(*f)();
////      public:
////        C_FuncSlot0(T_Ret(*function)()) : f(function) {};
////
////        //override
////        void Execute(T_P0 p0){ (*f)(); };
////    };
////    
////    template <class T_Ret> class C_FuncSlot1 : public C_SlotLink{
////        T_Ret(*f)(T_P0);
////      public:
////        C_FuncSlot1(T_Ret(*function)(T_P0)) : f(function) {};
////
////        //override
////        void Execute(T_P0 p0){ (*f)(p0); };
////    };
//
//    C_VecCntnr<C_SlotLink*> slotLink;
//
//  public:
//    //TODO: move to .cpp
//    void Emit(M_FUNC_PARAMS_FULL(1)){
//        for(uint i=0; i<slotLink.Size(); ++i)
//            slotLink[i]->execute(M_FUNC_PARAM_NAMES(1));
//    };
//
//    M_REPEAT2(2, M_CONNECT_METH, )
//    //M_CONNECT_METH(0)
//    //M_CONNECT_METH(1)
////    template <class T_Ob, class T_Ret> void Connect(T_Ob& o, T_Ret(T_Ob::*m)()){
////        slotLink.PushBack(new C_MethodSlot0<T_Ob, T_Ret>(&o, m));
////    };
////    
////    template <class T_Ob, class T_Ret> void Connect(T_Ob& o, T_Ret(T_Ob::*m)(T_P0)){
////        slotLink.PushBack(new C_MethodSlot1<T_Ob, T_Ret>(&o, m));
////    };
//
//    M_CONNECT_FUNC(0, )
//    M_CONNECT_FUNC(1, )
//    
////    template <class T_Ret> void Connect(T_Ret(*f)()){
////        slotLink.PushBack(new C_FuncSlot0<T_Ret>(f));
////    };
////    
////    template <class T_Ret> void Connect(T_Ret(*f)(T_P0)){
////        slotLink.PushBack(new C_FuncSlot1<T_Ret>(f));
////    };
//};

}//~namespace ting
