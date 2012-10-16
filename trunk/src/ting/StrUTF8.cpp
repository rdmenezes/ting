#include <string.h>

#include "StrUTF8.hpp"


using namespace ting;



StrUTF8& StrUTF8::operator=(const char* str){
	size_t len = strlen(str);
	this->Destroy();
	this->Init(str, len);
	return *this;
}



void StrUTF8::InitInternal(const char* str, size_t len){
	if(len == 0){
		this->s = 0;
		return;
	}
	
	this->s = new ting::u8[len];
	memcpy(this->s, str, len);
}



