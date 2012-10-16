#include <string.h>

#include "StrUTF8.hpp"


using namespace ting;



StrUTF8::StrUTF8(const char* str){
	size_t len = strlen(str);
	this->InitInternal(reinterpret_cast<const ting::u8*>(str), len);
}



StrUTF8& StrUTF8::operator=(const char* str){
	size_t len = strlen(str);
	this->Destroy();
	this->InitInternal(reinterpret_cast<const ting::u8*>(str), len);
	return *this;
}



void StrUTF8::InitInternal(const ting::u8* str, size_t len){
	if(len == 0){
		this->s = 0;
		return;
	}
	
	this->s = new ting::u8[len + 1];
	memcpy(this->s, str, len);
	this->s[len] = 0;//null-terminate
}



