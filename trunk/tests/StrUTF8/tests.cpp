#include "../../src/ting/debug.hpp"
#include "../../src/ting/StrUTF8.hpp"
#include "../../src/ting/fs/FSFile.hpp"

#include "tests.hpp"


using namespace ting;

namespace ting_test{
namespace strutf8{

namespace TestSimple{

void Run(){
	ting::fs::FSFile fi("text.txt");
	
	ting::Array<ting::u8> buf = fi.LoadWholeFileIntoMemory();
	
	StrUTF8 str(buf);
	
	ASSERT_ALWAYS(str.Char(0) == 'a')
	ASSERT_INFO_ALWAYS(str.Char(1) == 0x0411, "str.Char(1) = " << str.Char(1)) //capital russian B
	ASSERT_ALWAYS(str.Char(2) == 0x0446) //small russian C
	ASSERT_INFO_ALWAYS(str.Char(3) == 0xfeb6, "str.Char(3) = " << str.Char(3)) //some arabic stuff
	ASSERT_INFO_ALWAYS(str.Char(4) == 0x2000b, "str.Char(4) = " << str.Char(4)) //some compatibility char
	ASSERT_ALWAYS(str.Char(5) == 0)
}

}//~namespace

}//~namespace
}//~namespace
