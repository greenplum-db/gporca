//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp
//
//	@filename:
//		CLogicalConstTableGetSingletonDistSpec.cpp
//
//	@doc:
//		Implementation of const table access
//---------------------------------------------------------------------------

#include "gpos/base.h"
#include "gpopt/base/CUtils.h"

#include "gpopt/operators/CExpressionHandle.h"
#include "gpopt/operators/CLogicalConstTableGetSingletonDistSpec.h"
#include "gpopt/metadata/CName.h"
#include "gpopt/base/CColRefSet.h"
#include "gpopt/base/CColRefTable.h"

using namespace gpopt;


//---------------------------------------------------------------------------
//	@function:
//		CLogicalConstTableGetSingletonDistSpec::CLogicalConstTableGetSingletonDistSpec
//
//	@doc:
//		ctor - for pattern
//
//---------------------------------------------------------------------------
CLogicalConstTableGetSingletonDistSpec::CLogicalConstTableGetSingletonDistSpec
	(
	IMemoryPool *pmp
	)
	:
	CLogicalConstTableGet(pmp)
{
}


//---------------------------------------------------------------------------
//	@function:
//		CLogicalConstTableGetSingletonDistSpec::CLogicalConstTableGetSingletonDistSpec
//
//	@doc:
//		ctor
//
//---------------------------------------------------------------------------
CLogicalConstTableGetSingletonDistSpec::CLogicalConstTableGetSingletonDistSpec
	(
	IMemoryPool *pmp,
	DrgPcoldesc *pdrgpcoldesc,
	DrgPdrgPdatum *pdrgpdrgpdatum
	)
	:
	CLogicalConstTableGet(pmp, pdrgpcoldesc, pdrgpdrgpdatum)
{
}

//---------------------------------------------------------------------------
//	@function:
//		CLogicalConstTableGetSingletonDistSpec::CLogicalConstTableGetSingletonDistSpec
//
//	@doc:
//		ctor
//
//---------------------------------------------------------------------------
CLogicalConstTableGetSingletonDistSpec::CLogicalConstTableGetSingletonDistSpec
	(
	IMemoryPool *pmp,
	DrgPcr *pdrgpcrOutput,
	DrgPdrgPdatum *pdrgpdrgpdatum
	)
	:
	CLogicalConstTableGet(pmp, pdrgpcrOutput, pdrgpdrgpdatum)
{
}

//---------------------------------------------------------------------------
//	@function:
//		CLogicalConstTableGetSingletonDistSpec::~CLogicalConstTableGetSingletonDistSpec
//
//	@doc:
//		dtor
//
//---------------------------------------------------------------------------
CLogicalConstTableGetSingletonDistSpec::~CLogicalConstTableGetSingletonDistSpec()
{
}

//---------------------------------------------------------------------------
//	@function:
//		CLogicalConstTableGetSingletonDistSpec::PxfsCandidates
//
//	@doc:
//		Get candidate xforms
//
//---------------------------------------------------------------------------
CXformSet *
CLogicalConstTableGetSingletonDistSpec::PxfsCandidates
	(
	IMemoryPool *pmp
	) 
	const
{
	CXformSet *pxfs = GPOS_NEW(pmp) CXformSet(pmp);
	(void) pxfs->FExchangeSet(CXform::ExfImplementConstTableGetSingletonDistSpec);
	return pxfs;
}

//---------------------------------------------------------------------------
//	@function:
//		CLogicalConstTableGetSingletonDistSpec::PopCopyWithRemappedColumns
//
//	@doc:
//		Return a copy of the operator with remapped columns
//
//---------------------------------------------------------------------------
COperator *
CLogicalConstTableGetSingletonDistSpec::PopCopyWithRemappedColumns
(
	IMemoryPool *pmp,
	HMUlCr *phmulcr,
	BOOL fMustExist
	)
{
	DrgPcr *pdrgpcr = NULL;
	if (fMustExist)
	{
		pdrgpcr = CUtils::PdrgpcrRemapAndCreate(pmp, m_pdrgpcrOutput, phmulcr);
	}
	else
	{
		pdrgpcr = CUtils::PdrgpcrRemap(pmp, m_pdrgpcrOutput, phmulcr, fMustExist);
	}
	m_pdrgpdrgpdatum->AddRef();
	
	return GPOS_NEW(pmp) CLogicalConstTableGetSingletonDistSpec(pmp, pdrgpcr, m_pdrgpdrgpdatum);
}

// conversion function
CLogicalConstTableGetSingletonDistSpec *
CLogicalConstTableGetSingletonDistSpec::PopConvert
(
 COperator *pop
 )
{
	GPOS_ASSERT(NULL != pop);
	GPOS_ASSERT(COperator::EopLogicalConstTableGetSingletonDistSpec == pop->Eopid() ||
				COperator::EopLogicalConstTableGet == pop->Eopid());

	return dynamic_cast<CLogicalConstTableGetSingletonDistSpec*>(pop);
}

// return a string for operator name
const CHAR *
CLogicalConstTableGetSingletonDistSpec::SzId() const
{
	return "CLogicalConstTableGetSingletonDistSpec";
}

// ident accessors
COperator::EOperatorId
CLogicalConstTableGetSingletonDistSpec::Eopid() const
{
	return EopLogicalConstTableGetSingletonDistSpec;
}

// EOF

