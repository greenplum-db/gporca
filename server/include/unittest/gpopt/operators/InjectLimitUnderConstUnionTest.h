//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 EMC Corp.
//
//	@filename:
//		InjectLimitUnderConstUnionTest.h
//
//	@doc:
//		Test for expression preprocessing
//---------------------------------------------------------------------------
#ifndef GPOPT_InjectLimitUnderConstUnionTest_H
#define GPOPT_InjectLimitUnderConstUnionTest_H

#include "gpos/base.h"

namespace gpopt
{
	//---------------------------------------------------------------------------
	//	@class:
	//		InjectLimitUnderConstUnionTest
	//
	//	@doc:
	//		Unittests
	//
	//---------------------------------------------------------------------------
	class InjectLimitUnderConstUnionTest 
	{


	public:

		// unittests
		static GPOS_RESULT EresUnittest();
		static GPOS_RESULT EresUnittest_InjectLimitUnderConstUnionTest();


	}; // class InjectLimitUnderConstUnionTest
}

#endif // !GPOPT_InjectLimitUnderConstUnionTest_H

// EOF
