#include "../../src/ting/debug.hpp"

#include "tests.hpp"


inline void TestTingRef(){
	TestOperatorArrowAndOperatorStar::Run1();
	TestOperatorArrowAndOperatorStar::Run2();

	TestAutomaticDowncasting::Run1();

	TestBoolRelatedStuff::TestConversionToBool();
	TestBoolRelatedStuff::TestOperatorLogicalNot();
	
	TestBasicWeakRefUseCase::Run1();
	TestBasicWeakRefUseCase::Run2();

	TestExceptionThrowingFromRefCountedDerivedClassConstructor::Run();
	
	TestCreatingWeakRefFromRefCounted::Run1();
	TestCreatingWeakRefFromRefCounted::Run2();
	
	TestVirtualInheritedRefCounted::Run1();
	TestVirtualInheritedRefCounted::Run2();
	TestVirtualInheritedRefCounted::Run3();

	TestConstantReferences::Run1();
	
	TestOverloadedOperatorDelete::Run();

	TRACE_ALWAYS(<< "[PASSED]: Ref test" << std::endl)
}
