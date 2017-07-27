//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal, Inc.
//
//	@filename:
//		CQuantifiedSubqueryTest.cpp
//
//	@doc:
//		Test for exists and not exists subquery optimization
//---------------------------------------------------------------------------

#include "unittest/gpopt/minidump/CQuantifiedSubqueryTest.h"
#include "gpos/base.h"
#include "gpos/memory/CAutoMemoryPool.h"
#include "gpos/task/CAutoTraceFlag.h"
#include "gpos/test/CUnittest.h"

#include "gpopt/exception.h"
#include "gpopt/minidump/CMinidumperUtils.h"

#include "unittest/gpopt/CTestUtils.h"


using namespace gpopt;

ULONG CQuantifiedSubqueryTest::m_ulQuantifiedSubQueryTestCounter = 0;  // start from first test

// minidump files
const CHAR *rgszQuantifiedFileNames[] =
	{
		"../data/dxl/minidump/NULLComparisonInProject.mdp",
		"../data/dxl/minidump/NULLComparisonInWhereClause.mdp",
		"../data/dxl/minidump/OrderByOuterRef.mdp",
		"../data/dxl/minidump/SubqAll-To-ScalarSubq.mdp",
		"../data/dxl/minidump/SubqAll-Limit1.mdp",
		"../data/dxl/minidump/Subq-On-OuterRef.mdp",
		"../data/dxl/minidump/Subq-With-OuterRefCol.mdp",
		"../data/dxl/minidump/SemiJoin2Select-EnforceSubplan.mdp",
		"../data/dxl/minidump/Join-With-Subq-Preds-1.mdp",
		"../data/dxl/minidump/MultiLevel-CorrelatedExec.mdp",
		"../data/dxl/minidump/OneLevel-CorrelatedExec.mdp",
		"../data/dxl/minidump/MultiLevel-IN-Subquery.mdp",
		"../data/dxl/minidump/Correlated-LASJ-With-Outer-Const.mdp",
		"../data/dxl/minidump/Correlated-LASJ-With-Outer-Col.mdp",
		"../data/dxl/minidump/Correlated-LASJ-With-Outer-Expr.mdp",
		"../data/dxl/minidump/CorrelatedIN-LeftSemiJoin-True.mdp",
		"../data/dxl/minidump/CorrelatedIN-LeftSemiNotIn-True.mdp",
		"../data/dxl/minidump/AddPredsInSubqueries.mdp",
		"../data/dxl/minidump/AntiSemiJoin2Select-1.mdp",
		"../data/dxl/minidump/AntiSemiJoin2Select-2.mdp",
	};


// unittest for expressions
GPOS_RESULT
CQuantifiedSubqueryTest::EresUnittest()
{

#ifdef GPOS_DEBUG
	// disable extended asserts before running test
	fEnableExtendedAsserts = false;
#endif // GPOS_DEBUG

	CUnittest rgut[] =
		{
		GPOS_UNITTEST_FUNC(EresUnittest_RunTests),
		};

	GPOS_RESULT eres = CUnittest::EresExecute(rgut, GPOS_ARRAY_SIZE(rgut));

#ifdef GPOS_DEBUG
	// enable extended asserts after running test
	fEnableExtendedAsserts = true;
#endif // GPOS_DEBUG

	// reset metadata cache
	CMDCache::Reset();

	return eres;
}

// run all Minidump-based tests with plan matching
GPOS_RESULT
CQuantifiedSubqueryTest::EresUnittest_RunTests()
{
	return CTestUtils::EresUnittest_RunTests
						(
						rgszQuantifiedFileNames,
						&m_ulQuantifiedSubQueryTestCounter,
						GPOS_ARRAY_SIZE(rgszQuantifiedFileNames)
						);
}

// EOF
