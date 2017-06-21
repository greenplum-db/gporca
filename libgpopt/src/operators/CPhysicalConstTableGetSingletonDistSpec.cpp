//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 EMC Corp.
//
//	@filename:
//		CPhysicalConstTableGetSingletonDistSpec.cpp
//
//	@doc:
//		Implementation of physical const table get operator
//---------------------------------------------------------------------------

#include "gpos/base.h"

#include "gpopt/base/CUtils.h"
#include "gpopt/base/CCTEMap.h"
#include "gpopt/base/COptCtxt.h"
#include "gpopt/operators/CPhysicalConstTableGetSingletonDistSpec.h"
#include "gpopt/operators/CExpressionHandle.h"


using namespace gpopt;


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalConstTableGetSingletonDistSpec::CPhysicalConstTableGetSingletonDistSpec
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CPhysicalConstTableGetSingletonDistSpec::CPhysicalConstTableGetSingletonDistSpec
	(
	IMemoryPool *pmp,
	DrgPcoldesc *pdrgpcoldesc,
	DrgPdrgPdatum *pdrgpdrgpdatum,
	DrgPcr *pdrgpcrOutput
	)
	:
	CPhysicalConstTableGet(pmp, pdrgpcoldesc, pdrgpdrgpdatum, pdrgpcrOutput)
{}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalConstTableGetSingletonDistSpec::~CPhysicalConstTableGetSingletonDistSpec
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CPhysicalConstTableGetSingletonDistSpec::~CPhysicalConstTableGetSingletonDistSpec()
{}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalConstTableGetSingletonDistSpec::FMatch
//
//	@doc:
//		Match operators
//
//---------------------------------------------------------------------------
BOOL
CPhysicalConstTableGetSingletonDistSpec::FMatch
	(
	COperator *pop
	)
	const
{
	if (Eopid() == pop->Eopid())
	{
		CPhysicalConstTableGetSingletonDistSpec *popCTG = CPhysicalConstTableGetSingletonDistSpec::PopConvert(pop);
		return m_pdrgpcoldesc == popCTG->Pdrgpcoldesc() && 
				m_pdrgpdrgpdatum == popCTG->Pdrgpdrgpdatum() && 
				m_pdrgpcrOutput == popCTG->PdrgpcrOutput();
	}
	
	return false;
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalConstTableGetSingletonDistSpec::PdsDerive
//
//	@doc:
//		Derive distribution
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalConstTableGetSingletonDistSpec::PdsDerive
	(
	IMemoryPool *pmp,
	CExpressionHandle & // exprhdl
	)
	const
{
	return GPOS_NEW(pmp) CDistributionSpecSingleton(CDistributionSpecSingleton::EstMaster);
}

// conversion function
CPhysicalConstTableGetSingletonDistSpec *
CPhysicalConstTableGetSingletonDistSpec::PopConvert
(
 COperator *pop
 )
{
	GPOS_ASSERT(NULL != pop);
	GPOS_ASSERT(EopPhysicalConstTableGet == pop->Eopid() ||
				EopPhysicalConstTableGetSingletonDistSpec == pop->Eopid());

	return dynamic_cast<CPhysicalConstTableGetSingletonDistSpec*>(pop);
}

const CHAR *
CPhysicalConstTableGetSingletonDistSpec::SzId() const
{
	return "CPhysicalConstTableGetSingletonDistSpec";
}

// ident accessors
COperator::EOperatorId
CPhysicalConstTableGetSingletonDistSpec::Eopid() const
{
	return COperator::EopPhysicalConstTableGetSingletonDistSpec;
}
// EOF

