//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal, Inc.
//
//	@filename:
//		CPruneColumnsTest.cpp
//
//	@doc:
//		Test for optimizing queries where intermediate columns are pruned
//
//	@owner:
//
//
//	@test:
//
//
//---------------------------------------------------------------------------

#include "unittest/gpopt/minidump/CPruneColumnsTest.h"
#include "gpos/base.h"
#include "gpos/memory/CAutoMemoryPool.h"
#include "gpos/task/CAutoTraceFlag.h"
#include "gpos/test/CUnittest.h"

#include "gpopt/exception.h"
#include "gpopt/minidump/CMinidumperUtils.h"

#include "unittest/gpopt/CTestUtils.h"


using namespace gpopt;

ULONG CPruneColumnsTest::m_ulPruneColumnsTestCounter = 0;  // start from first test

// minidump files
const CHAR *rgszPruneColumnsFileNames[] =
{
	"../data/dxl/minidump/PruneIntermediateUnusedColumns.mdp", // prune all unused columns
	"../data/dxl/minidump/AggTopOfSingleSetRetFuncs.mdp", // no pruning done
	"../data/dxl/minidump/AggTopOfSetRetFuncsAndUnusedScalar.mdp", // partial pruning
	"../data/dxl/minidump/AggTopOfMultipleSetRetFuncs.mdp", // no pruning done
	"../data/dxl/minidump/AggTopOfMultipleSetRetFuncsAndUnusedScalar.mdp", // partial prune
	"../data/dxl/minidump/AggTopOfSetRefFuncsOnTopTbl.mdp", // no pruning done
	"../data/dxl/minidump/AllColsUsed.mdp", // no pruning done
	"../data/dxl/minidump/UsedSetRetFuncAndUnusedScalarFunc.mdp", // partial pruning
	"../data/dxl/minidump/UnusedSetRetFuncAndUsedScalarFunc.mdp", // partial pruning
	"../data/dxl/minidump/MultiLevelSubqueryWithSetRetFuncs.mdp",	 // expect error in optimizer and planner: "Expected no more than one row to be returned by expression"
	"../data/dxl/minidump/MultiLevelSubqueryWithSetRetFuncsAndScalarFuncs.mdp", // if pruning subquery is handled then we should prune some columns
};


//---------------------------------------------------------------------------
//	@function:
//		CPruneColumnsTest::EresUnittest
//
//	@doc:
//		Unittest for expressions
//
//---------------------------------------------------------------------------
GPOS_RESULT
CPruneColumnsTest::EresUnittest()
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

//---------------------------------------------------------------------------
//	@function:
//		CPruneColumnsTest::EresUnittest_RunTests
//
//	@doc:
//		Run all Minidump-based tests with plan matching
//
//---------------------------------------------------------------------------
GPOS_RESULT
CPruneColumnsTest::EresUnittest_RunTests()
{
	return CTestUtils::EresUnittest_RunTests
						(
						rgszPruneColumnsFileNames,
						&m_ulPruneColumnsTestCounter,
						GPOS_ARRAY_SIZE(rgszPruneColumnsFileNames)
						);
}

// EOF
