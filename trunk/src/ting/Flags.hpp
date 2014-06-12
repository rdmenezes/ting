/* The MIT License:

Copyright (c) 2014 Ivan Gagis <igagis@gmail.com>

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

#include "types.hpp"
#include "debug.hpp"

#include <cstring>


namespace ting{


/**
 * @brief class representing a set of flags.
 * If you define an enumeration according to the following rules:
 * - enumeration is defined inside of a struct/class namespace
 * - enumeration name is 'Type'
 * - there is no direct assignment of values to enumeration items, i.e. values are in strict ascending order
 * - the very last item is ENUM_SIZE
 * 
 * For example:
 * @code
 * struct MyEnum{
 *     enum Type{
 *         MY_ZEROTH_ITEM,
 *         MY_FIRST_ITEM,
 *         MY_SECOND_ITEM,
 *         MY_THIRD_ITEM,
 *         ...
 *         ENUM_SIZE
 *     };
 * };
 * @endcode
 * Then, the Flags can be used as follows:
 * @code
 * ting::Flags<MyEnum> fs;
 * 
 * fs.Set(MyEnum::MY_FIRST_ITEM, true).Set(MyEnum::MY_THIRD_ITEM, true);
 * 
 * if(fs.Get(MyEnum::MY_FIRST_ITEM)){
 *     //MY_FIRST_ITEM flag is set
 * }
 * 
 * if(fs.Get(MyEnum::MY_ZEROTH_ITEM)){
 *     //Will not get here, since MY_ZEROTH_ITEM flag is not set
 * }
 * 
 * @endcode
 */
template <class T_Enum> class Flags{
	ting::u8 flags[T_Enum::ENUM_SIZE / 8 + 1];
	
public:
	typedef typename ting::UnsignedTypeForSize<sizeof(typename T_Enum::Type)>::Type index_t;
	
	/**
	 * @brief Constructor.
	 * Creates a Flags with all flags initialized to a given value.
     * @param initialValueOfAllFlags - value to initialize all flags to.
     */
	Flags(bool initialValueOfAllFlags = false){
		memset(this->flags, initialValueOfAllFlags ? ting::u8(-1) : 0, sizeof(this->flags));
	}
	
	/**
	 * @brief Size of the flag set.
     * @return Number of flags in this flag set.
     */
	index_t Size()const throw(){
		return T_Enum::ENUM_SIZE;
	}
	
	/**
	 * @brief Get value of a given flag.
     * @param flag - flag to get value of.
     * @return true if the flag is set.
	 * @return false otherwise.
     */
	bool Get(enum T_Enum::Type flag)const throw(){
		ASSERT(flag < T_Enum::ENUM_SIZE)
		return (this->flags[flag / 8] & (1 << (flag % 8))) != 0;
	}
	
	/**
	 * @brief Get value for i'th flag.
	 * Returns the value of the flag given by index.
	 * Note, the index must be less than enumeration size,
	 * otherwise the behavior is undefined.
     * @param i - index of the flag to get value of.
     * @return value of the flag given by index.
     */
	bool Get(index_t i)const throw(){
		return this->Get(typename T_Enum::Type(i));
	}
	
	/**
	 * @brief Set value of a given flag.
     * @param flag - flag to set value of.
     * @param value - value to set.
     * @return Reference to this Flags.
     */
	Flags& Set(enum T_Enum::Type flag, bool value)throw(){
		ASSERT(flag < T_Enum::ENUM_SIZE)
		if(value){
			this->flags[flag / 8] |= (1 << (flag % 8));
		}else{
			this->flags[flag / 8] &= (~(1 << (flag % 8)));
		}
		return *this;
	}
	
	/**
	 * @brief Set value of an i'th flag.
	 * Sets the value of the flag given by index.
	 * Note, the index must be less than enumeration size,
	 * otherwise the behavior is undefined.
     * @param i - index of the flag to set value of.
     * @param value - value to set.
     * @return Reference to this Flags.
     */
	Flags& Set(index_t i, bool value)throw(){
		return this->Set(typename T_Enum::Type(i), value);
	}
	
	/**
	 * @brief Check if all flags are cleared.
     * @return true if all flags are cleared.
	 * @return false otherwise.
     */
	bool IsAllClear()const throw(){
		ASSERT(sizeof(this->flags) > 0)
		for(size_t i = 0; i != sizeof(this->flags) - 1; ++i){
			if(this->flags[i] != 0){
				return false;
			}
		}
		for(index_t i = (this->Size() / 8) * 8; i != this->Size(); ++i){
			if(this->Get(i)){
				return false;
			}
		}
		return true;
	}
	
	/**
	 * @brief Check if all flags are set.
     * @return true if all flags are set.
	 * @return false otherwise.
     */
	bool IsAllSet()const throw(){
		ASSERT(sizeof(this->flags) > 0)
		for(size_t i = 0; i != sizeof(this->flags) - 1; ++i){
			if(this->flags[i] != ting::u8(-1)){
				return false;
			}
		}
		for(index_t i = (this->Size() / 8) * 8; i != this->Size(); ++i){
			if(!this->Get(i)){
				return false;
			}
		}
		return true;
	}
	
#ifdef DEBUG
	friend std::ostream& operator<<(std::ostream& s, const Flags& fs){
		s << "(";
		
		for(index_t i = 0; i != fs.Size(); ++i){
			s << (fs.Get(i) ? "1" : "0");
		}
		
		s << ")";
		return s;
	}
#endif
};


}//~namespace
