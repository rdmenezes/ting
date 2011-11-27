#include "../../src/ting/debug.hpp"
#include "../../src/ting/FSFile.hpp"



using namespace ting;



namespace TestSeekForward{
void Run(){
	ting::FSFile f("test.file.txt");
	ASSERT_ALWAYS(!f.IsDir())
	ASSERT_ALWAYS(!f.IsOpened())
	
	for(unsigned numToSeek = 0; numToSeek < 0x1000; numToSeek += (0x1000 / 4)){
		ting::StaticBuffer<ting::u8, 1> testByte;
		{
			ting::Array<ting::u8> buf(numToSeek);
			
			ting::File::Guard fileGuard(f, ting::File::READ);
			
			unsigned res = f.Read(buf);
			ASSERT_ALWAYS(res == buf.Size())
			
			res = f.Read(testByte);
			ASSERT_ALWAYS(res == testByte.Size())
			
//			TRACE_ALWAYS(<< "testByte = " << unsigned(testByte[0]) << std::endl)
		}
		
		{
			ting::File::Guard fileGuard(f, ting::File::READ);

			f.File::SeekForward(numToSeek);

			ting::StaticBuffer<ting::u8, 1> buf;

			unsigned res = f.Read(buf, 1);
			ASSERT_ALWAYS(res == 1)

			ASSERT_ALWAYS(buf[0] == testByte[0])
		}

		{
			ting::File::Guard fileGuard(f, ting::File::READ);

			f.SeekForward(numToSeek);

			ting::StaticBuffer<ting::u8, 1> buf;

			unsigned res = f.Read(buf, 1);
			ASSERT_ALWAYS(res == 1)

			ASSERT_ALWAYS(buf[0] == testByte[0])
		}
	}//~for
}
}//~namespace



namespace TestListDirContents{
void Run(){
	ting::FSFile curDir("./");
	ting::File& f = curDir;
	
	ting::Array<std::string> r = f.ListDirContents();
	ASSERT_ALWAYS(r.Size() >= 3)
//	TRACE_ALWAYS(<< "list = " << r << std::endl)
	
	ting::Array<std::string> r1 = f.ListDirContents(1);
	ASSERT_ALWAYS(r1.Size() == 1)
	ASSERT_ALWAYS(r[0] == r1[0])
	
	ting::Array<std::string> r2 = f.ListDirContents(2);
	ASSERT_ALWAYS(r2.Size() == 2)
	ASSERT_ALWAYS(r[0] == r2[0])
	ASSERT_ALWAYS(r[1] == r2[1])
}
}//~namespace


int main(int argc, char *argv[]){

	TestSeekForward::Run();
	TestListDirContents::Run();

	TRACE_ALWAYS(<< "[PASSED]" << std::endl)

	return 0;
}
