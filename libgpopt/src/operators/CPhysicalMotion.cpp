//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp.
//
//	@filename:
//		CPhysicalMotion.cpp
//
//	@doc:
//		Implementation of motion operator
//---------------------------------------------------------------------------

#include "gpos/base.h"

#include "gpopt/base/CUtils.h"
#include "gpopt/base/COptCtxt.h"
#include "gpopt/base/CDistributionSpecAny.h"
#include "gpopt/operators/CExpressionHandle.h"
#include "gpopt/operators/CPhysicalMotion.h"
#include "gpopt/search/CMemo.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalMotion::FValidContext
//
//	@doc:
//		Check if optimization context is valid
//
//---------------------------------------------------------------------------
BOOL
CPhysicalMotion::FValidContext
	(
	IMemoryPool *pmp,
	COptimizationContext *poc,
	DrgPoc *pdrgpocChild
	)
	const
{
	GPOS_ASSERT(NULL != pdrgpocChild);
	GPOS_ASSERT(1 == pdrgpocChild->UlLength());

	COptimizationContext *pocChild = (*pdrgpocChild)[0];
	CCostContext *pccBest = pocChild->PccBest();
	GPOS_ASSERT(NULL != pccBest);

	CDrvdPropPlan *pdpplanChild = pccBest->Pdpplan();
	if (pdpplanChild->Ppim()->FContainsUnresolved())
	{
		return false;
	}

	CExpressionHandle exprhdl(pmp);
	exprhdl.Attach(pccBest);
	exprhdl.DeriveProps(NULL /*CDrvdPropCtxt*/);
	if (exprhdl.FHasOuterRefs())
	{
		// disallow plans with outer references below motion operator
		return false;
	}

	CEnfdDistribution *ped = poc->Prpp()->Ped();
	if (ped->FCompatible(this->Pds()) && ped->FCompatible(pdpplanChild->Pds()))
	{
		// required distribution is compatible with the distribution delivered by Motion and its child plan,
		// in this case, Motion is redundant since child plan delivers the required distribution
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalMotion::PdsRequired
//
//	@doc:
//		Compute required distribution of the n-th child
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalMotion::PdsRequired
	(
	IMemoryPool *pmp,
	CExpressionHandle &, // exprhdl
	CDistributionSpec *, // pdsRequired
	ULONG
#ifdef GPOS_DEBUG
	ulChildIndex
#endif // GPOS_DEBUG
	,
	DrgPdp *, // pdrgpdpCtxt
	ULONG // ulOptReq
	)
	const
{
	GPOS_ASSERT(0 == ulChildIndex);

	// any motion operator is distribution-establishing and does not require
	// child to deliver any specific distribution
	return GPOS_NEW(pmp) CDistributionSpecAny(this->Eopid());
}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalMotion::PrsRequired
//
//	@doc:
//		Compute required rewindability of the n-th child
//
//---------------------------------------------------------------------------
CRewindabilitySpec *
CPhysicalMotion::PrsRequired
	(
	IMemoryPool *pmp,
	CExpressionHandle &, // exprhdl
	CRewindabilitySpec *, // prsRequired
	ULONG
#ifdef GPOS_DEBUG
	ulChildIndex
#endif // GPOS_DEBUG
	,
	DrgPdp *, // pdrgpdpCtxt
	ULONG // ulOptReq
	)
	const
{
	GPOS_ASSERT(0 == ulChildIndex);

	// motion does not preserve rewindability;
	// child does not need to be rewindable
	return GPOS_NEW(pmp) CRewindabilitySpec(CRewindabilitySpec::ErtNone /*ert*/);
}

// Hash function
ULONG
CPhysicalMotion::CPartPropReq::UlHash
	(
	const CPartPropReq *pppr
	)
{
    GPOS_ASSERT(NULL != pppr);
    
    ULONG ulHash = pppr->Ppps()->UlHash();
    ulHash = UlCombineHashes(ulHash , pppr->UlChildIndex());
    return UlCombineHashes(ulHash , pppr->UlOuterChild());
 
}

// Equality function
BOOL
CPhysicalMotion::CPartPropReq::FEqual
	(
	const CPartPropReq *ppprFst,
	const CPartPropReq *ppprSnd
	)
{
    GPOS_ASSERT(NULL != ppprFst);
    GPOS_ASSERT(NULL != ppprSnd);
    
    return
    ppprFst->UlChildIndex() == ppprSnd->UlChildIndex() &&
    ppprFst->UlOuterChild() == ppprSnd->UlOuterChild() &&
    ppprFst->Ppps()->FMatch(ppprSnd->Ppps());
}


// Create partition propagation request
CPhysicalMotion::CPartPropReq *
CPhysicalMotion::PpprCreate
(
	IMemoryPool *pmp,
	CExpressionHandle &exprhdl,
	CPartitionPropagationSpec *pppsRequired,
	ULONG ulChildIndex
	)
{
    GPOS_ASSERT(exprhdl.Pop() == this);
    GPOS_ASSERT(NULL != pppsRequired);
    if (NULL == exprhdl.Pgexpr())
    {
        return NULL;
    }
    
    ULONG ulOuterChild = (*exprhdl.Pgexpr())[0]->UlId();

    
    pppsRequired->AddRef();
    return  GPOS_NEW(pmp) CPartPropReq(pppsRequired, ulChildIndex, ulOuterChild);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalMotion::PppsRequired
//
//	@doc:
//		Compute required partition propagation of the n-th child
//
//---------------------------------------------------------------------------
CPartitionPropagationSpec *
CPhysicalMotion::PppsRequired
	(
	IMemoryPool *pmp,
	CExpressionHandle &exprhdl,
	CPartitionPropagationSpec *pppsRequired,
	ULONG 
	ulChildIndex,
	DrgPdp *, //pdrgpdpCtxt,
	ULONG //ulOptReq
	)
{
	GPOS_ASSERT(0 == ulChildIndex);
	GPOS_ASSERT(NULL != pppsRequired);

	CPartPropReq *pppr = PpprCreate(pmp, exprhdl, pppsRequired, ulChildIndex);
	CPartitionPropagationSpec *ppps = m_phmpp->PtLookup(pppr);
	if (NULL == ppps)
	{
		CPartIndexMap *ppimResult = GPOS_NEW(pmp) CPartIndexMap(pmp);
		CPartFilterMap *ppfmResult = GPOS_NEW(pmp) CPartFilterMap(pmp);
		CPartIndexMap *ppimReqd = pppsRequired->Ppim();
		CPartFilterMap *ppfmReqd = pppsRequired->Ppfm();
		
		DrgPul *pdrgpul = ppimReqd->PdrgpulScanIds(pmp);
		
		/// get derived part consumers
		CPartInfo *ppartinfo = exprhdl.Pdprel(0)->Ppartinfo();
		
		const ULONG ulPartIndexSize = pdrgpul->UlLength();
		
		for (ULONG ul = 0; ul < ulPartIndexSize; ul++)
		{
			ULONG ulPartIndexId = *((*pdrgpul)[ul]);
			
			if (!ppartinfo->FContainsScanId(ulPartIndexId))
			{
				// part index id does not exist in child nodes: do not push it below
				// the motion
				continue;
			}
			
			ppimResult->AddRequiredPartPropagation(ppimReqd, ulPartIndexId, CPartIndexMap::EppraPreservePropagators);
			(void) ppfmResult->FCopyPartFilter(m_pmp, ulPartIndexId, ppfmReqd);
		}
		
		pdrgpul->Release();
		
		ppps = GPOS_NEW(pmp) CPartitionPropagationSpec(ppimResult, ppfmResult);
		m_phmpp->FInsert(pppr, ppps);
	}
	else
	{
		pppr->Release();
	}
	
    ppps->AddRef();
    return ppps;

}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalMotion::PcteRequired
//
//	@doc:
//		Compute required CTE map of the n-th child
//
//---------------------------------------------------------------------------
CCTEReq *
CPhysicalMotion::PcteRequired
	(
	IMemoryPool *, //pmp,
	CExpressionHandle &, //exprhdl,
	CCTEReq *pcter,
	ULONG
#ifdef GPOS_DEBUG
	ulChildIndex
#endif
	,
	DrgPdp *, //pdrgpdpCtxt,
	ULONG //ulOptReq
	)
	const
{
	GPOS_ASSERT(0 == ulChildIndex);
	return PcterPushThru(pcter);
}

//---------------------------------------------------------------------------
//	@function:
//		CPhysicalMotion::PdsDerive
//
//	@doc:
//		Derive distribution
//
//---------------------------------------------------------------------------
CDistributionSpec *
CPhysicalMotion::PdsDerive
	(
	IMemoryPool */*pmp*/,
	CExpressionHandle &/*exprhdl*/
	)
	const
{
	CDistributionSpec *pds = Pds();
	pds->AddRef();

	return pds;
}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalMotion::PrsDerive
//
//	@doc:
//		Derive rewindability
//
//---------------------------------------------------------------------------
CRewindabilitySpec *
CPhysicalMotion::PrsDerive
	(
	IMemoryPool *pmp,
	CExpressionHandle & // exprhdl
	)
	const
{
	// output of motion is non-rewindable
	return GPOS_NEW(pmp) CRewindabilitySpec(CRewindabilitySpec::ErtNone /*ert*/);
}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalMotion::EpetDistribution
//
//	@doc:
//		Return distribution property enforcing type for this operator
//
//---------------------------------------------------------------------------
CEnfdProp::EPropEnforcingType
CPhysicalMotion::EpetDistribution
	(
	CExpressionHandle &, // exprhdl
	const CEnfdDistribution *ped
	)
	const
{
	GPOS_ASSERT(NULL != ped);

	if (ped->FCompatible(Pds()))
	{
		return CEnfdProp::EpetUnnecessary;
	}

	return CEnfdProp::EpetProhibited;
}


//---------------------------------------------------------------------------
//	@function:
//		CPhysicalMotion::EpetRewindability
//
//	@doc:
//		Return rewindability property enforcing type for this operator
//
//---------------------------------------------------------------------------
CEnfdProp::EPropEnforcingType
CPhysicalMotion::EpetRewindability
	(
	CExpressionHandle &exprhdl,
	const CEnfdRewindability * // per
	)
	const
{
	if (exprhdl.FHasOuterRefs())
	{
		// motion has outer references: prohibit this plan 
		// Note: this is a GPDB restriction as Motion operators are push-based
		return CEnfdProp::EpetProhibited;
	}

	// motion does not provide rewindability on its output
	return CEnfdProp::EpetRequired;
}

// EOF
