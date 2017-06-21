//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 EMC Corp.
//
//	@filename:
//		CXformImplementConstTableGetSingletonDistSpec.cpp
//
//	@doc:
//		Implementation of transform
//---------------------------------------------------------------------------

#include "gpos/base.h"
#include "gpopt/xforms/CXformImplementConstTableGetSingletonDistSpec.h"

#include "gpopt/operators/ops.h"

using namespace gpopt;


//---------------------------------------------------------------------------
//	@function:
//		CXformImplementConstTableGetSingletonDistSpec::CXformImplementConstTableGetSingletonDistSpec
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CXformImplementConstTableGetSingletonDistSpec::CXformImplementConstTableGetSingletonDistSpec
	(
	IMemoryPool *pmp
	)
	:
	CXformImplementation
		(
		 // pattern
		GPOS_NEW(pmp) CExpression
				(
				pmp,
				GPOS_NEW(pmp) CLogicalConstTableGetSingletonDistSpec(pmp)
				)
		)
{}


//---------------------------------------------------------------------------
//	@function:
//		CXformImplementConstTableGetSingletonDistSpec::Transform
//
//	@doc:
//		Actual transformation
//
//---------------------------------------------------------------------------
void
CXformImplementConstTableGetSingletonDistSpec::Transform
	(
	CXformContext *pxfctxt,
	CXformResult *pxfres,
	CExpression *pexpr
	)
	const
{
	GPOS_ASSERT(NULL != pxfctxt);
	GPOS_ASSERT(FPromising(pxfctxt->Pmp(), this, pexpr));
	GPOS_ASSERT(FCheckPattern(pexpr));

	CLogicalConstTableGetSingletonDistSpec *popConstTableGetSingletonDistSpec = CLogicalConstTableGetSingletonDistSpec::PopConvert(pexpr->Pop());
	IMemoryPool *pmp = pxfctxt->Pmp();

	// create/extract components for alternative
	DrgPcoldesc *pdrgpcoldesc = popConstTableGetSingletonDistSpec->Pdrgpcoldesc();
	pdrgpcoldesc->AddRef();
	
	DrgPdrgPdatum *pdrgpdrgpdatum = popConstTableGetSingletonDistSpec->Pdrgpdrgpdatum();
	pdrgpdrgpdatum->AddRef();
	
	DrgPcr *pdrgpcrOutput = popConstTableGetSingletonDistSpec->PdrgpcrOutput();
	pdrgpcrOutput->AddRef();
		
	// create alternative expression
	CExpression *pexprAlt = 
		GPOS_NEW(pmp) CExpression
			(
			pmp,
			GPOS_NEW(pmp) CPhysicalConstTableGetSingletonDistSpec(pmp, pdrgpcoldesc, pdrgpdrgpdatum, pdrgpcrOutput)
			);
	
	// add alternative to transformation result
	pxfres->Add(pexprAlt);
}


// compute xform promise for a given expression handle
CXform::EXformPromise
CXformImplementConstTableGetSingletonDistSpec::Exfp
	(
	CExpressionHandle & // exprhdl
	)
	const
{
	return CXform::ExfpHigh;
}


// return a string for xform name
const CHAR *
CXformImplementConstTableGetSingletonDistSpec::SzId() const
{
	return "CXformImplementConstTableGetSingletonDistSpec";
}

// ident accessors
CXform::EXformId
CXformImplementConstTableGetSingletonDistSpec::Exfid() const
{
	return ExfImplementConstTableGetSingletonDistSpec;
}

// EOF

