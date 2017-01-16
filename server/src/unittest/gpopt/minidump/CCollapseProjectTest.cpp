//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal, Inc.
//
//	@filename:
//		CCollapseProjectTest.cpp
//
//	@doc:
//		Test for optimizing queries with multiple project nodes
//---------------------------------------------------------------------------

#include "unittest/gpopt/minidump/CCollapseProjectTest.h"
#include "gpos/base.h"
#include "gpos/memory/CAutoMemoryPool.h"
#include "gpos/task/CAutoTraceFlag.h"
#include "gpos/test/CUnittest.h"

#include "gpopt/exception.h"
#include "gpopt/minidump/CMinidumperUtils.h"

#include "unittest/gpopt/CTestUtils.h"


using namespace gpopt;

ULONG CCollapseProjectTest::m_ulCollapseProjectTestCounter = 0;  // start from first test

// minidump files
const CHAR *rgszCollapseProjectFileNames[] =
{
		"../data/dxl/minidump/DMLCollapseProject.mdp",
		"../data/dxl/minidump/CollapseCascadeProjects2of2.mdp",
		"../data/dxl/minidump/CollapseCascadeProjects2of3.mdp",
		"../data/dxl/minidump/CannotCollapseCascadeProjects.mdp",
		"../data/dxl/minidump/CollapseProject-SetReturning.mdp",
		"../data/dxl/minidump/CollapseProject-SetReturning-CTE.mdp",
		"../data/dxl/minidump/MutlipleSetReturningFunction.mdp",
	};


//---------------------------------------------------------------------------
//	@function:
//		CCollapseProjectTest::EresUnittest
//
//	@doc:
//		Unittest for expressions
//
//---------------------------------------------------------------------------
GPOS_RESULT
CCollapseProjectTest::EresUnittest()
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
//		CCollapseProjectTest::EresUnittest_RunTests
//
//	@doc:
//		Run all Minidump-based tests with plan matching
//
//---------------------------------------------------------------------------
GPOS_RESULT
CCollapseProjectTest::EresUnittest_RunTests()
{
	return CTestUtils::EresUnittest_RunTests
						(
						rgszCollapseProjectFileNames,
						&m_ulCollapseProjectTestCounter,
						GPOS_ARRAY_SIZE(rgszCollapseProjectFileNames)
						);
}

// EOF
