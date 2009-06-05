/* The MIT License:

Copyright (c) 2009 Ivan Gagis

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

#ifndef M_Fixed_hpp
#define M_Fixed_hpp

#include <ting/types.hpp>
#include <ting/debug.hpp>
#include <ting/math.hpp>

namespace ting{

template <ting::uint Mantissa> class Fixed{
	ting::s32 n;

	inline Fixed& Set(ting::s32 v){
		this->n = v;
		return *this;
	}
public:

	inline Fixed(){};

	//copy constructor
	inline Fixed(const Fixed& v) :
			n(v.n)
	{}

	inline Fixed(int value) :
			n(value << Mantissa)
	{}

	inline Fixed(float value){
		n = ting::s32(value * (1 << Mantissa));
	}

	inline Fixed(double value){
		n = ting::s32(value * (1 << Mantissa));
	}

	inline Fixed operator-()const{
		return Fixed().Set(-this->n);
	}

	inline Fixed& operator+=(const Fixed& v){
		this->n += v.n;
		return *this;
	}

	inline Fixed operator+(const Fixed& v)const{
		return (Fixed(*this) += v);
	}

	inline Fixed& operator-=(const Fixed& v){
		this->n -= v.n;
		return *this;
	}

	inline Fixed operator-(const Fixed& v)const{
		return (Fixed(*this) -= v);
	}

	inline Fixed& operator*=(const Fixed& v){
		this->n = ting::s32(
				(ting::s64(this->n) * ting::s64(v.n)) >> Mantissa
			);
		return *this;
	}

	inline Fixed operator*(const Fixed& v)const{
		return (Fixed(*this) *= v);
	}

	inline Fixed& operator/=(const Fixed& v){
		this->n = ting::s32(
				(ting::s64(this->n) << Mantissa) / ting::s64(v.n)
			);
		return *this;
	}

	inline Fixed operator/(const Fixed& v)const{
		return (Fixed(*this) /= v);
	}

	inline Fixed operator%(const Fixed& v)const{
		return Fixed().Set(this->n % v.n);
	}

	//monster implementation of division operator :-)
	//but using 64 bit integers should be faster
//	Fixed operator/(const Fixed& v){
//		ting::s32 a = this->n;
//		ting::s32 b = v.n;
//		ting::s32 c = a / b;
//		ting::s32 r = a % b;
//
//		int sign = ((a < 0 && b > 0) || (a > 0 && b < 0)) ? (-1) : 1;
//
//		if(sign < 0 && c == 0)
//			c = ~c;
//		else if(sign < 0)
//			c -= 1;
//
//		c <<= Mantissa;
//
//		r = (r >= 0) ? r : (-r);
//		b = (b >= 0) ? b : (-b);
//
//		ting::s32 m = 0;
//		for(ting::uint i = 1; i <= Mantissa; ++i){
//			r <<= 1;
//			if(r / b != 0){
//				m |= 1;
//				r %= b;
//			}
//			m <<= 1;
//		}
//		m >>= 1;
//
//		m = (sign > 0) ? m :((-m) & ((1 << Mantissa) - 1));
//		//NOTE: ((1 << Mantissa) - 1) = 0xfff for Mantissa = 12
//
//		return Fixed().Set(c | m);
//	}

	inline Fixed& operator=(const Fixed& v){
		this->n = v.n;
		return *this;
	}

	inline bool operator<(const Fixed& v)const{
		return this->n < v.n;
	}
	
	inline bool operator<=(const Fixed& v)const{
		return this->n <= v.n;
	}

	inline bool operator>(const Fixed& v)const{
		return this->n > v.n;
	}

	inline bool operator>=(const Fixed& v)const{
		return this->n >= v.n;
	}

	inline operator float()const{
		return float(this->n) / float(1 << Mantissa);
	}

	inline operator double()const{
		return double(float(this->n) / float(1 << Mantissa));
	}

	inline operator ting::s32()const{
		return this->n >> Mantissa;
	}

private:
	static inline Fixed SinTeylor7Degree(Fixed v){
		return v - ting::Pow3(v) / Fixed(6)
				+ ting::Pow5(v) / Fixed(120)
				- (ting::Pow5(v) / Fixed(5040)) * ting::Pow2(v);
	}
public:
	Fixed SinTeylor()const{
		//Good Teylor approximation of sine in [-pi:pi] is 7th degree polynomial

		//move to [-2pi:2pi]
		Fixed v = this->operator%(Fixed(ting::D2Pi<float>()));

		//move v to [-pi:pi]
		if(v < Fixed(-ting::DPi<float>())){
			v += Fixed(ting::D2Pi<float>());
		}else if(v > Fixed(ting::DPi<float>())){
			v -= Fixed(ting::D2Pi<float>());
		}

		//Looks like precision of ~13 mantissa bits is not enough
		//to calculate sine teylor in ranges [-pi:-pi/2) and (pi/2:pi].
		//So, move calculation to [-pi/2:pi/2].
		if(v < Fixed(-ting::DPi<float>() / 2)){
			v += Fixed(ting::DPi<float>());
			return -SinTeylor7Degree(v);
		}else if(v > Fixed(ting::DPi<float>() / 2)){
			v -= Fixed(ting::DPi<float>());
			return -SinTeylor7Degree(v);
		}

		return SinTeylor7Degree(v);
	}

private:

// 3.14159 dec = 11.001001 bin (8 bits). 11001001 bin = 201 dec
#define M_SIN_SMALL_TABLE_LENGTH 202 //202 = including 0
	//Small table contains values of sin for x from [0:pi]
	static Fixed* FillSmallTableInternal(){
		//NOTE: (M_SIN_SMALL_TABLE_LENGTH + 1) to include pi
		static Fixed table[M_SIN_SMALL_TABLE_LENGTH + 1];

//		TRACE(<< "creating table" << std::endl)

		for(ting::uint i = 0; i < M_SIN_SMALL_TABLE_LENGTH + 1; ++i){
			table[i] = Fixed(
					float(sin(
							Fixed().Set(i << (Mantissa - 6)).operator float()
						))
				);
//			TRACE(<< "table[i] = " << table[i] << std::endl)
		}

//		TRACE(<< "table created" << std::endl)

		return &table[0];
	}

	static inline Fixed SinSmallTableInterpolated(Fixed v){
		static Fixed *sinTable = FillSmallTableInternal();

		ASSERT(Fixed(0.0f) <= v && v < ting::DPi<Fixed>())

		ASSERT(Mantissa >= 6)
		ting::uint index = ((v.n) >> (Mantissa - 6)); //take 1 bit from integer part and 7 from mantissa

		ASSERT(index < M_SIN_SMALL_TABLE_LENGTH)

		Fixed k = Fixed().Set( (v.n << 6) & ((1 << Mantissa) - 1) );

		ASSERT(k < Fixed(1.0f))

//		TRACE(<< "index = " << index << std::endl)
//		TRACE(<< "sinTable[index] = " << sinTable[index] << std::endl)

		return sinTable[index] * (Fixed(1) - k) + sinTable[index + 1] * k;
	}
#undef M_SIN_SMALL_TABLE_LENGTH

public:
	/**
	@brief Calculate sine of angle using table lookup with linear interpolation.
	Small table is the table which stores whole sine period [0:pi].
	It is more accurate than big table lookup (big table stores [0:2pi] ).
	*/
	Fixed SinSmallTableSmooth()const{
//		TRACE(<< "SinSmallTable(): enter" << (*this) << std::endl)

		//move to [-2pi:2pi]
		Fixed v = this->operator%(ting::D2Pi<Fixed>());

//		TRACE(<< "vv = " << v << std::endl)

		//move to [0:2pi]
		if(v < Fixed(0)){
			v += ting::D2Pi<Fixed>();
		}

//		TRACE(<< "vvv = " << v << std::endl)

		//move to [0:pi]
		if(v >= ting::DPi<Fixed>()){
			v -= ting::DPi<Fixed>();
//			TRACE(<< "2v = " << v << std::endl)
			return -SinSmallTableInterpolated(v);
		}else{
//			TRACE(<< "1v = " << v << std::endl)
			return SinSmallTableInterpolated(v);
		}
	}


private:

// 6.28318 dec = 110.01001 bin (8 bits). 11001001 bin = 201 dec
#define M_SIN_BIG_TABLE_LENGTH 202 //202 = including 0
	//Big table contains values of sin for x from [0:2pi]
	static Fixed* FillBigTableInternal(){
		//NOTE: (M_SIN_SMALL_TABLE_LENGTH + 1) to include 2pi
		static Fixed table[M_SIN_BIG_TABLE_LENGTH + 1];

		for(ting::uint i = 0; i < M_SIN_BIG_TABLE_LENGTH + 1; ++i){
			table[i] = Fixed(
					sin(
							Fixed().Set(i << (Mantissa - 5)).operator float()
						)
				);
		}

		return &table[0];
	}

	static inline Fixed SinBigTableInterpolated(Fixed v){
		static Fixed *sinTable = FillBigTableInternal();

		ASSERT(Fixed(0) <= v && v < ting::D2Pi<Fixed>())

		ASSERT(Mantissa >= 5)
		ting::uint index = ((v.n) >> (Mantissa - 5)); //take 3 bits from integer part and 5 from mantissa

		ASSERT_INFO(index < M_SIN_BIG_TABLE_LENGTH, "v = " << float(v) << " index = " << index << " 2pi = " << float(ting::D2Pi<Fixed>()))

		Fixed k = Fixed().Set( (v.n << 5) & ((1 << Mantissa) - 1) );

		ASSERT(k < Fixed(1.0f))

//		TRACE(<< "index = " << index << std::endl)
//		TRACE(<< "sinTable[index] = " << sinTable[index] << std::endl)

		return sinTable[index] * (Fixed(1) - k) + sinTable[index + 1] * k;
	}


	static Fixed* FillVeryBigTableInternal(){
		//NOTE: (M_SIN_SMALL_TABLE_LENGTH + 1) to include 2pi
		static Fixed table[M_SIN_BIG_TABLE_LENGTH * 16 + 1];

		for(ting::uint i = 0; i < M_SIN_BIG_TABLE_LENGTH * 16 + 1; ++i){
			table[i] = Fixed(
					sin(
							Fixed().Set(i << (Mantissa - 9)).operator float()
						)
				);
		}

		return &table[0];
	}

	static inline Fixed SinBigTableNotInterpolated(Fixed v){
		static Fixed *sinTable = FillVeryBigTableInternal();

		ASSERT(Fixed(0) <= v && v < ting::D2Pi<Fixed>())

		ASSERT(Mantissa >= 9)
		ting::uint index = ((v.n) >> (Mantissa - 9)); //take 3 bits from integer part and 9 from mantissa

		ASSERT_INFO(index < M_SIN_BIG_TABLE_LENGTH * 16, "v = " << float(v) << " index = " << index << " 2pi = " << float(ting::D2Pi<Fixed>()))

//		TRACE(<< "index = " << index << std::endl)
//		TRACE(<< "sinTable[index] = " << sinTable[index] << std::endl)

		return sinTable[index];
	}

#undef M_SIN_BIG_TABLE_LENGTH

public:
	/**
	@brief Calculate sine of angle using table lookup with linear interpolation.
	Big table is the table which stores whole sine period [0: 2pi].
	It is less accurate than small table lookup (small table stores [0:pi] ).
	*/
	Fixed SinBigTableSmooth()const{
		//move to [-2pi:2pi]
		Fixed v = this->operator%(ting::D2Pi<Fixed>());

		//move to [0:2pi]
		if(v < Fixed(0)){
			v += ting::D2Pi<Fixed>();
		}

		return SinBigTableInterpolated(v);
	}

	/**
	@brief Calculate sine of angle using table lookup without interpolation.
	Big table is the table which stores whole sine period [0: 2pi].
	It is less accurate than small table lookup (small table stores [0:pi] ).
	*/
	Fixed SinBigTable()const{
		//move to [-2pi:2pi]
		Fixed v = this->operator%(ting::D2Pi<Fixed>());

		//move to [0:2pi]
		if(v < Fixed(0)){
			v += ting::D2Pi<Fixed>();
		}

		return SinBigTableNotInterpolated(v);
	}

	
	inline Fixed Sin()const{
		return this->SinBigTable();
	}


#ifdef DEBUG
	friend std::ostream& operator<<(std::ostream& s, const Fixed<Mantissa>& n){
		s << float(n);
		return s;
	};
#endif
};

}//~namespace
#endif//~once
