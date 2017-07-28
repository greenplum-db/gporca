//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal, Inc.
//
//	@filename:
//		CQuantifiedSubqueryTest.h
//
//	@doc:
//		Test for exists and not exists subquery optimization
//---------------------------------------------------------------------------
#ifndef GPOPT_CQuantifiedSubqueryTest_H
#define GPOPT_CQuantifiedSubqueryTest_H

#include "gpos/base.h"

namespace gpopt
{
	class CQuantifiedSubqueryTest
	{
		private:

			// counter used to mark last successful test
			static
			gpos::ULONG m_ulQuantifiedSubQueryTestCounter;

		public:

			// unittests
			static
			gpos::GPOS_RESULT EresUnittest();

			static
			gpos::GPOS_RESULT EresUnittest_RunTests();

	}; // class CQuantifiedSubqueryTest
}

#endif // !GPOPT_CQuantifiedSubqueryTest_H

// EOF

