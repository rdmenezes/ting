#include "../../src/ting/debug.hpp"
#include "../../src/ting/utf8.hpp"
#include "../../src/ting/fs/FSFile.hpp"

#include "tests.hpp"


using namespace ting;

namespace ting_test{
namespace strutf8{

namespace TestSimple{

void Run(){
	ting::fs::FSFile fi("text.txt");
	
	std::vector<std::uint8_t> buf = fi.LoadWholeFileIntoMemory();
	
//	TRACE_ALWAYS(<< "buf.Size() = " << buf.Size() << std::endl)
	
	std::vector<std::uint8_t> str(buf.size() + 1);
	memcpy(&*str.begin(), &*buf.begin(), buf.size());
	str[buf.size()] = 0; //null-terminate
	
	utf8::Iterator i(reinterpret_cast<char*>(&*str.begin()));
	
	ASSERT_ALWAYS(i.Char() == 'a')
	++i;
	ASSERT_INFO_ALWAYS(i.Char() == 0x0411, "i.Char() = " << i.Char()) //capital russian B
	++i;
	ASSERT_ALWAYS(i.Char() == 0x0446) //small russian C
	++i;
	ASSERT_INFO_ALWAYS(i.Char() == 0xfeb6, "i.Char() = " << i.Char()) //some arabic stuff
	++i;
	ASSERT_INFO_ALWAYS(i.Char() == 0x2000b, "i.Char() = " << i.Char()) //some compatibility char
	++i;
	ASSERT_INFO_ALWAYS(i.Char() == 0, "i.Char() = " << i.Char())
	ASSERT_ALWAYS(i.IsEnd())
	ASSERT_ALWAYS(!i.IsNotEnd())
}

}//~namespace

}//~namespace
}//~namespace
