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
//	Math utilities


#ifndef M_math_hpp
#define M_math_hpp

#include <math.h>
#include <limits> //this is for std::numeric_limits<float>::quiet_NaN(); etc.

namespace ting{

//======================
//Utility math functions
//======================
template <typename T_Type> inline T_Type Sign(T_Type n){
	return n > 0 ? (1) : (-1);
};

template <typename T_Type> inline T_Type Abs(T_Type n){
	return n > 0 ? n : (-n);
};

template <typename T_Type> inline T_Type Min(T_Type a, T_Type b){
	return a < b ? a : b;
};

template <typename T_Type> inline T_Type Max(T_Type a, T_Type b){
	return a > b ? a : b;
};

template <typename T> inline T DEps(){
	return T(1e-6f);
};

template <typename T> inline T DPi(){
	return T(3.14159f);
};

template <typename T> inline T D2Pi(){
	return T(2 * DPi<T>());
};

/**
@return natural logarithm of 2
*/
template <typename T> inline T DLnOf2(){
	return T(0.693147181);
};

template <typename T> inline T DNaN(){
	return std::numeric_limits<T>::quiet_NaN();
};

template <typename T> inline T DInf(){
	return std::numeric_limits<T>::infinity();
};

//Power functions
template <typename T_Type> inline T_Type Pow2(T_Type x){
	return x * x;
};

template <typename T_Type> inline T_Type Pow3(T_Type x){
	return Pow2(x) * x;
};

template <typename T_Type> inline T_Type Pow4(T_Type x){
	return Pow2(Pow2(x));
};

//Cubic root function
template <typename T_Type> inline T_Type CubicRoot(T_Type x){
	if(x > 0)
		return exp(::log(x) / 3);
	else if(x == 0)
		return 0;
	else
		return -exp(::log(-x) / 3);
};

//argument of a complex number
template <typename T_Type> inline T_Type Arg(T_Type x, T_Type y){
	T_Type a;
	if(x == 0)
		a = DPi<T_Type>() / 2;
	else if(x > 0)
		a = T_Type(atan(Abs(y / x)));
	else
		a = DPi<T_Type>() - T_Type(atan(Abs(y / x)));

	if(y >= 0)
		return a;
	else
		return -a;
};

}//~namespace ting
#endif//~once
