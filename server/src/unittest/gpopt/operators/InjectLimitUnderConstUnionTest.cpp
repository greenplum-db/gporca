//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 EMC Corp.
//
//	@filename:
//		InjectLimitUnderConstUnionTest.cpp
//
//	@doc:
//		Test for expression preprocessing
//---------------------------------------------------------------------------
#include <string.h>

#include "gpos/io/COstreamString.h"
#include "gpos/string/CWStringDynamic.h"
#include "gpos/task/CAutoTraceFlag.h"
#include "gpos/common/CAutoRef.h"

#include "gpopt/base/CUtils.h"
#include "gpopt/eval/CConstExprEvaluatorDefault.h"
#include "gpopt/mdcache/CAutoMDAccessor.h"
#include "gpopt/operators/CPredicateUtils.h"
#include "gpopt/operators/CScalarProjectElement.h"
#include "gpopt/operators/CLogicalNAryJoin.h"
#include "gpopt/operators/CLogicalInnerJoin.h"
#include "gpopt/operators/CLogicalLeftOuterJoin.h"
#include "gpopt/operators/CExpressionUtils.h"
#include "gpopt/optimizer/COptimizerConfig.h"
#include "gpopt/xforms/CXformUtils.h"

#include "unittest/base.h"
#include "unittest/gpopt/operators/InjectLimitUnderConstUnionTest.h"
#include "unittest/gpopt/CTestUtils.h"

#include "naucrates/md/CMDProviderMemory.h"
#include "naucrates/md/CMDIdGPDB.h"

using namespace gpopt;
using namespace gpmd;
//---------------------------------------------------------------------------
//	@function:
//		InjectLimitUnderConstUnionTest::EresUnittest
//
//	@doc:
//		Unittest for InjectLimitUnderConstUnion pre-processing step
//
//---------------------------------------------------------------------------
GPOS_RESULT
InjectLimitUnderConstUnionTest::EresUnittest()
{

	CUnittest rgut[] =
			{

					GPOS_UNITTEST_FUNC(EresUnittest_InjectLimitUnderConstUnionTest),
			};

	return CUnittest::EresExecute(rgut, GPOS_ARRAY_SIZE(rgut));
}

CExpression *CreateLogicalGet(IMemoryPool *pmp)
{
	CWStringConst strName(GPOS_WSZ_LIT("foo"));
	CMDIdGPDB *pmdid = GPOS_NEW(pmp) CMDIdGPDB(GPOPT_MDCACHE_TEST_OID, 1, 1);
	CTableDescriptor *ptabdesc = CTestUtils::PtabdescCreate(pmp, 2, pmdid, CName(&strName));
	CWStringConst strAlias = CWStringConst(GPOS_WSZ_LIT("fooAlias"));
	CExpression *logicalGet = CTestUtils::PexprLogicalGet(pmp, ptabdesc, &strAlias);
	return logicalGet;
}

CExpression *CreateLogicalProject(IMemoryPool *pmp, CExpression *pexprFirstChild, CMDAccessor *cmdAccessor)
{
	CExpression *pexprScalarConst = CUtils::PexprScalarConstInt4(pmp, 5);
	IMDId *pmdidType = CScalarConst::PopConvert(pexprScalarConst->Pop())->PmdidType();

	const IMDType *pmdtype = cmdAccessor->Pmdtype(pmdidType); // copy from a cast
	CColumnFactory *pcf = COptCtxt:: PoctxtFromTLS()->Pcf(); // copy from reinterpret cast
	CColRef *pcrComputed = pcf->PcrCreate(pmdtype, -1); // destructor does nothing

	CExpression *pexprPrjElem = CUtils::PexprScalarProjectElement(pmp, pcrComputed, pexprScalarConst);
	CExpression *pexprPrjList = GPOS_NEW(pmp) CExpression(pmp, GPOS_NEW(pmp) CScalarProjectList(pmp), pexprPrjElem);
	CExpression *logicalProject = CUtils::PexprLogicalProject(pmp, pexprFirstChild, pexprPrjList, true);

	return logicalProject;
}

CColRefSet *CreateDerivedCColRefSet(IMemoryPool *pmp, CExpression *pexprLogical)
{
	CExpressionHandle exprhdl(pmp);
	exprhdl.Attach(pexprLogical);
	return CLogical::PopConvert(pexprLogical->Pop())->PcrsDeriveOutput(pmp, exprhdl); // doesn't take ownership of logicalproject
}

// Returns the first descendent of pexpr with OperatorId eopid from a depth first search.
// Returns NULL if no such descendent exists.
CExpression *FindExprWithOperatorId(CExpression *pexpr, COperator::EOperatorId eopid)
{
	for (ULONG ul = 0; ul < pexpr->UlArity(); ul++)
	{
		CExpression *pexprChild = (*pexpr)[ul];
		if (pexprChild->Pop()->Eopid() == eopid)
		{
			return pexprChild;
		}
		else
		{
			CExpression *pexprResult = FindExprWithOperatorId(pexprChild, eopid);
			if (pexprResult != NULL)
			{
				return pexprResult;
			}
		}
	}

	return NULL;
}

// Produces a CLogicalUnion where the first child exclusively projects CScalarConsts and the second child projects
// exclusively CScalarIdents. pexprLogicalProject must have operator CLogicalProject and a descendent with operator
// CLogicalGet. pexprLogicalGet must have operator CLogicalGet.
// pexprLogicalProject will be used to construct the first child of the CLogicalUnion
// pexprLogicalGet will be used to construct the second child of the CLogicalUnion
//
// Produces output that resembles either:
// +--CLogicalUnion Output: ("ColRef_0004" (4)), Input: [("ColRef_0004" (4)), ("column_0000" (2))]
// |--CLogicalProject
// |  |--CLogicalGet "fooAlias" ("foo"), Columns: ["column_0000" (0), "column_0001" (1)] Key sets: {[0]}
// |  +--CScalarProjectList
// |     +--CScalarProjectElement "ColRef_0004" (4)
// |        +--CScalarConst (5)
// +--CLogicalGet "fooAlias" ("foo"), Columns: ["column_0000" (2), "column_0001" (3)] Key sets: {[0]}
//
// Or:
// +--CLogicalUnion Output: ("ColRef_0004" (4)), Input: [("ColRef_0004" (4)), ("column_0000" (0))]
// |--CLogicalProject
// |  |--CLogicalLimit <empty> global
// |  |  |--CLogicalGet "fooAlias" ("foo"), Columns: ["column_0000" (2), "column_0001" (3)] Key sets: {[0]}
// |  |  |--CScalarConst (0)
// |  |  +--CScalarConst (1)
// |  +--CScalarProjectList
// |     +--CScalarProjectElement "ColRef_0004" (4)
// |        +--CScalarConst (5)
// +--CLogicalGet "fooAlias" ("foo"), Columns: ["column_0000" (0), "column_0001" (1)] Key sets: {[0]}

CExpression *CreateLogicalUnion(IMemoryPool *pmp, CExpression *pexprLogicalProject, CExpression *pexprLogicalGet)
{
	CColRefSet *pcrsProject = CreateDerivedCColRefSet(pmp, pexprLogicalProject);

	// CLogicalUnion needs an input array of CColRefSet and an output CColRefSet
	// The input will include CColRefSet for both of its children
	// The output will be only the CColRefSet projected by its first child
	// Because the first child of the CLogicalUnion, in this case, is projecting only constants
	// The CColRefSet is obtained, as a matter of convenience, by excluding the CColRefSet from the CLogicalGet
	// from that of the CLogicalProject of which it is a descendent
	CExpression *pexprGetUnderProject = FindExprWithOperatorId(pexprLogicalProject, COperator::EopLogicalGet);
	GPOS_ASSERT(pexprGetUnderProject);
	CColRefSet *pcrsGetUnderProject = CreateDerivedCColRefSet(pmp, pexprGetUnderProject);
	pcrsProject->Exclude(pcrsGetUnderProject);
	pcrsGetUnderProject->Release();

	// The CLogicalGet, which is the second child to the CLogicalUnion, because we have collapsed its CScalarProjectLIst
	// In this very contrived test case, we assume that we are using exactly the first CColRef in the CLogicalGet's CColRefSet
	CColRefSet *pcrsGet = CreateDerivedCColRefSet(pmp, pexprLogicalGet);
	DrgPcr *pdrgpcrGet = GPOS_NEW(pmp) DrgPcr(pmp);
	pdrgpcrGet->Append(pcrsGet->PcrFirst());
	pcrsGet->Release();

	// Construct the input array of arrays of column reference sets for the CLogicalUnion
	DrgDrgPcr *pdrgpdrgpcrInput = GPOS_NEW(pmp) DrgDrgPcr(pmp);
	pdrgpdrgpcrInput->Append(pcrsProject->Pdrgpcr(pmp));
	pdrgpdrgpcrInput->Append(pdrgpcrGet);

	// Construct the output array of column reference set for the CLogicalUnion
	// This is the same as the CColRefSet of the first child of the CLogicalUnion
	DrgPcr *pdrgpcrOutput = pcrsProject->Pdrgpcr(pmp);
	pcrsProject->Release();

	CExpression *pexprLogicalUnion = GPOS_NEW(pmp) CExpression(pmp, GPOS_NEW(pmp) CLogicalUnion(pmp,
																								pdrgpcrOutput,
																								pdrgpdrgpcrInput), pexprLogicalProject, pexprLogicalGet);
	return pexprLogicalUnion;
}

GPOS_RESULT
InjectLimitUnderConstUnionTest::EresUnittest_InjectLimitUnderConstUnionTest()
{
	// TODO: add negative test case
	CAutoMemoryPool amp;
	IMemoryPool *pmp = amp.Pmp();
	// reset metadata cache
	CMDCache::Reset();
	// setup a file-based provider
	CMDProviderMemory *pmdp = CTestUtils::m_pmdpf;
	pmdp->AddRef();
	CMDAccessor mda(pmp, CMDCache::Pcache(), CTestUtils::m_sysidDefault, pmdp);
	CAutoOptCtxt aoc(pmp, &mda, NULL, CTestUtils::Pcm(pmp));

	// RHS is shared by input and output
	CExpression *pexprLogicalGetRHS = CreateLogicalGet(pmp);

	// Construct Input
	CExpression *pexprLogicalGetLHS = CreateLogicalGet(pmp);

	pexprLogicalGetLHS->AddRef();
	CExpression *pexprLogicalProjectInput = CreateLogicalProject(pmp, pexprLogicalGetLHS, &mda); // takes ownership of pexprLogicalGetLHS

	pexprLogicalProjectInput->AddRef();
	pexprLogicalGetRHS->AddRef();
	CExpression *pexprLogicalUnionInput = CreateLogicalUnion(pmp, pexprLogicalProjectInput, pexprLogicalGetRHS);

	// Construct expected output by remaking logicalproject but with same scalar project list
	CExpression *pexprScalarProjectList = (*pexprLogicalProjectInput)[1];
	// same logical get left hand side but now we want to put the limit on top of it
	pexprLogicalGetLHS->AddRef();
	CExpression *pexprLogicalLimit = CUtils::PexprLimit(pmp, pexprLogicalGetLHS, 0, 1);

	pexprLogicalLimit->AddRef();
	pexprScalarProjectList->AddRef();
	CExpression *pexprLogicalProjectOutput = CUtils::PexprLogicalProject(pmp, pexprLogicalLimit, pexprScalarProjectList, false);

	pexprLogicalProjectOutput->AddRef();
	pexprLogicalGetRHS->AddRef();
	CExpression *pexprLogicalUnionExpectedOutput = CreateLogicalUnion(pmp, pexprLogicalProjectOutput, pexprLogicalGetRHS);

	pexprLogicalUnionInput->AddRef();
	CExpression *pexprPreprocessed = CExpressionPreprocessor::PexprPreprocess(pmp, pexprLogicalUnionInput);
	pexprLogicalUnionInput->Release();

	GPOS_RTL_ASSERT(pexprLogicalUnionExpectedOutput->FMatch(pexprPreprocessed));
	pexprPreprocessed->Release();

	pexprLogicalUnionExpectedOutput->Release();

	pexprLogicalLimit->Release();
	pexprLogicalProjectOutput->Release();
	pexprLogicalUnionInput->Release();
	pexprLogicalProjectInput->Release();
	pexprLogicalGetLHS->Release();
	pexprLogicalGetRHS->Release();

	return GPOS_OK;
}
// EOF
