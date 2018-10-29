//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp.
//
//	@filename:
//		CJoinOrder.cpp
//
//	@doc:
//		Implementation of join order logic
//---------------------------------------------------------------------------

#include "gpos/base.h"

#include "gpos/io/COstreamString.h"
#include "gpos/string/CWStringDynamic.h"

#include "gpos/common/clibwrapper.h"
#include "gpos/common/CBitSet.h"

#include "gpopt/base/CDrvdPropScalar.h"
#include "gpopt/base/CColRefSetIter.h"
#include "gpopt/operators/ops.h"
#include "gpopt/operators/CPredicateUtils.h"
#include "gpopt/xforms/CJoinOrder.h"

#include "gpos/common/CAutoRef.h"


using namespace gpopt;

			
//---------------------------------------------------------------------------
//	@function:
//		ICmpEdgesByLength
//
//	@doc:
//		Comparison function for simple join ordering: sort edges by length
//		only to guaranteed that single-table predicates don't end up above 
//		joins;
//
//---------------------------------------------------------------------------
INT ICmpEdgesByLength
	(
	const void *pvOne,
	const void *pvTwo
	)
{
	CJoinOrder::SEdge *pedgeOne = *(CJoinOrder::SEdge**)pvOne;
	CJoinOrder::SEdge *pedgeTwo = *(CJoinOrder::SEdge**)pvTwo;

	
	INT iDiff = (pedgeOne->m_pbs->Size() - pedgeTwo->m_pbs->Size());
	if (0 == iDiff)
	{
		return (INT)pedgeOne->m_pbs->HashValue() - (INT)pedgeTwo->m_pbs->HashValue();
	}
		
	return iDiff;
}

	
//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::SComponent::SComponent
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CJoinOrder::SComponent::SComponent
	(
	IMemoryPool *mp,
	CExpression *pexpr
	)
	:
	m_pbs(NULL),
	m_edge_set(NULL),
	m_pexpr(pexpr),
	m_fUsed(false),
	m_outerchild_index(0),
	m_innerchild_index(0)
{	
	m_pbs = GPOS_NEW(mp) CBitSet(mp);
	m_edge_set = GPOS_NEW(mp) CBitSet(mp);
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::SComponent::SComponent
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CJoinOrder::SComponent::SComponent
	(
	CExpression *pexpr,
	CBitSet *pbs,
	CBitSet *edge_set
	)
	:
	m_pbs(pbs),
	m_edge_set(edge_set),
	m_pexpr(pexpr),
	m_fUsed(false),
	m_outerchild_index(0),
	m_innerchild_index(0)
{
	GPOS_ASSERT(NULL != pbs);
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::SComponent::~SComponent
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CJoinOrder::SComponent::~SComponent()
{	
	m_pbs->Release();
	m_edge_set->Release();
	CRefCount::SafeRelease(m_pexpr);
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::SComponent::OsPrint
//
//	@doc:
//		Debug print
//
//---------------------------------------------------------------------------
IOstream &
CJoinOrder::SComponent::OsPrint
	(
	IOstream &os
	)
const
{
	CBitSet *pbs = m_pbs;
	os 
		<< "Component: ";
	os
		<< (*pbs) << std::endl;
	os
		<< *m_pexpr << std::endl;
	os
		<< "Outerchild index: ";
	os
		<<  m_outerchild_index << std::endl;
	os
		<< "Innerchild index: ";
	os
		<<  m_innerchild_index << std::endl;

	return os;
}

void
CJoinOrder::SComponent::SetOuterChildIndex
	(
	INT index
	)
{
	GPOS_ASSERT(0 == m_outerchild_index);
	m_outerchild_index = index;
}

void
CJoinOrder::SComponent::SetInnerChildIndex
	(
	INT index
	)
{
	GPOS_ASSERT(0 == m_innerchild_index);
	m_innerchild_index = index;
}

//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::SEdge::SEdge
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CJoinOrder::SEdge::SEdge
	(
	IMemoryPool *mp,
	CExpression *pexpr
	)
	:
	m_pbs(NULL),
	m_pexpr(pexpr),
	m_fUsed(false)
{	
	m_pbs = GPOS_NEW(mp) CBitSet(mp);
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::SEdge::~SEdge
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CJoinOrder::SEdge::~SEdge()
{	
	m_pbs->Release();
	m_pexpr->Release();
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::SEdge::OsPrint
//
//	@doc:
//		Debug print
//
//---------------------------------------------------------------------------
IOstream &
CJoinOrder::SEdge::OsPrint
	(
	IOstream &os
	)
	const
{
	return os 
		<< "Edge : " 
		<< (*m_pbs) << std::endl 
		<< *m_pexpr << std::endl;
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::CJoinOrder
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CJoinOrder::CJoinOrder
	(
	IMemoryPool *mp,
	CExpressionArray *pdrgpexpr,
	CExpressionArray *pdrgpexprConj,
	BOOL include_loj_rels
	)
	:
	m_mp(mp),
	m_rgpedge(NULL),
	m_ulEdges(0),
	m_rgpcomp(NULL),
	m_ulComps(0),
	m_include_loj_rels(include_loj_rels)
{
	typedef SComponent* Pcomp;
	typedef SEdge* Pedge;
	
	const ULONG num_of_nary_children = pdrgpexpr->Size();
	INT num_of_loj = 0;

	// Since we are using a static array, we need to know size of the array before hand
	// e.g.
	// +--CLogicalNAryJoin
	// |--CLogicalGet "t1"
	// |--CLogicalLeftOuterJoin
	// |  |--CLogicalGet "t5"
	// |  |--CLogicalGet "t4"
	// |  +--CScalarCmp (=)
	// |
	// +--CScalarCmp (=)
	//
	// In above case the pdrgpexpr comes with two elements in it:
	//  - CLogicalGet "t1"
	//  - CLogicalLeftOuterJoin
	// We need to create components out of "t1", "t4", "t5" and store them
	// in m_rgcomp.
	// total number of components = size of pdrgpexpr + no. of LOJs in it


	if (m_include_loj_rels)
	{
		for (ULONG ul = 0; ul < num_of_nary_children; ul++)
		{
			CExpression *pexprComp = (*pdrgpexpr)[ul];
			if (COperator::EopLogicalLeftOuterJoin == pexprComp->Pop()->Eopid())
			{
				// we handle only one level of LOJ
				num_of_loj++;
			}
		}
	}

	m_ulComps = num_of_nary_children + num_of_loj;
	m_rgpcomp = GPOS_NEW_ARRAY(mp, Pcomp, m_ulComps);

	INT loj_index = 0;
	INT component = 0;

	for (ULONG ul = 0; ul < num_of_nary_children; ul++, component++)
	{
		CExpression *pexprComp = (*pdrgpexpr)[ul];
		if (m_include_loj_rels &&
			COperator::EopLogicalLeftOuterJoin == pexprComp->Pop()->Eopid())
		{
			// counter for number of loj available in tree
			++loj_index;

			CExpression *outer_child = (*pexprComp)[0];
			CExpression *inner_child = (*pexprComp)[1];

			outer_child->AddRef();
			SComponent *sc_outer = GPOS_NEW(mp) SComponent(mp, outer_child);
			sc_outer->SetOuterChildIndex(loj_index);
			m_rgpcomp[component] = sc_outer;

			// component always covers itself
			(void) m_rgpcomp[component]->m_pbs->ExchangeSet(component);

			component++;
			inner_child->AddRef();
			SComponent *sc_inner = GPOS_NEW(mp) SComponent(mp, inner_child);
			sc_inner->SetInnerChildIndex(loj_index);
			m_rgpcomp[component] = sc_inner;

			// add scalar
			CExpression *scalar = (*pexprComp)[2];
			scalar->AddRef();
			pdrgpexprConj->Append(scalar);
		}
		else
		{
			pexprComp->AddRef();
			m_rgpcomp[component] = GPOS_NEW(mp) SComponent(mp, pexprComp);
		}
		
		// component always covers itself
		(void) m_rgpcomp[component]->m_pbs->ExchangeSet(component);
	}

	m_ulEdges = pdrgpexprConj->Size();
	m_rgpedge = GPOS_NEW_ARRAY(mp, Pedge, m_ulEdges);
	
	for (ULONG ul = 0; ul < m_ulEdges; ul++)
	{
		CExpression *pexprEdge = (*pdrgpexprConj)[ul];
		pexprEdge->AddRef();
		m_rgpedge[ul] = GPOS_NEW(mp) SEdge(mp, pexprEdge);
	}
	
	pdrgpexpr->Release();
	pdrgpexprConj->Release();
	
	ComputeEdgeCover();
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::~CJoinOrder
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CJoinOrder::~CJoinOrder()
{
	for (ULONG ul = 0; ul < m_ulComps; ul++)
	{
		m_rgpcomp[ul]->Release();
	}
	GPOS_DELETE_ARRAY(m_rgpcomp);

	for (ULONG ul = 0; ul < m_ulEdges; ul++)
	{
		m_rgpedge[ul]->Release();
	}
	GPOS_DELETE_ARRAY(m_rgpedge);
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::ComputeEdgeCover
//
//	@doc:
//		Compute cover for each edge and also the index of edges associated
//		with each component
//
//---------------------------------------------------------------------------
void
CJoinOrder::ComputeEdgeCover()
{
	for (ULONG ulEdge = 0; ulEdge < m_ulEdges; ulEdge++)
	{
		CExpression *pexpr = m_rgpedge[ulEdge]->m_pexpr;
		CColRefSet *pcrsUsed = CDrvdPropScalar::GetDrvdScalarProps(pexpr->PdpDerive())->PcrsUsed();

		for (ULONG ulComp = 0; ulComp < m_ulComps; ulComp++)
		{
			CExpression *pexprComp = m_rgpcomp[ulComp]->m_pexpr;
			CColRefSet *pcrsOutput = CDrvdPropRelational::GetRelationalProperties(pexprComp->PdpDerive())->PcrsOutput();


			if (!pcrsUsed->IsDisjoint(pcrsOutput))
			{
				(void) m_rgpcomp[ulComp]->m_edge_set->ExchangeSet(ulEdge);
				(void) m_rgpedge[ulEdge]->m_pbs->ExchangeSet(ulComp);
			}
		}
	}
}

//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::PcompCombine
//
//	@doc:
//		Combine the two given components using applicable edges
//
//
//---------------------------------------------------------------------------
CJoinOrder::SComponent *
CJoinOrder::PcompCombine
	(
	SComponent *pcompOuter,
	SComponent *pcompInner
	)
{
	CBitSet *pbs = GPOS_NEW(m_mp) CBitSet(m_mp);
	CBitSet *edge_set = GPOS_NEW(m_mp) CBitSet(m_mp);

	pbs->Union(pcompOuter->m_pbs);
	pbs->Union(pcompInner->m_pbs);

	// edges connecting with the current component
	edge_set->Union(pcompOuter->m_edge_set);
	edge_set->Union(pcompInner->m_edge_set);

	CExpressionArray *pdrgpexpr = GPOS_NEW(m_mp) CExpressionArray(m_mp);
	for (ULONG ul = 0; ul < m_ulEdges; ul++)
	{
		SEdge *pedge = m_rgpedge[ul];
		if (pedge->m_fUsed)
		{
			// edge is already used in result component
			continue;
		}

		if (pbs->ContainsAll(pedge->m_pbs))
		{
			// edge is subsumed by the cover of the combined component
			CExpression *pexpr = pedge->m_pexpr;
			pexpr->AddRef();
			pdrgpexpr->Append(pexpr);
		}
	}

	CExpression *pexprOuter = pcompOuter->m_pexpr;
	CExpression *pexprInner = pcompInner->m_pexpr;
	CExpression *pexprScalar = CPredicateUtils::PexprConjunction(m_mp, pdrgpexpr);

	CExpression *pexpr = NULL;
	INT component_outerchild_index = 0;
	INT component_innerchild_index = 0;

	if (NULL == pexprOuter)
	{
		// first call to this function, we create a Select node
		component_outerchild_index = pcompInner->GetOuterChildIndex();
		component_innerchild_index = pcompInner->GetInnerChildIndex();
		pexpr = CUtils::PexprCollapseSelect(m_mp, pexprInner, pexprScalar);
		pexprScalar->Release();
	}
	else
	{
		pexprInner->AddRef();
		pexprOuter->AddRef();

		if (IsChildOfSameLOJ(pcompOuter, pcompInner))
		{
			pexpr = CUtils::PexprLogicalJoin<CLogicalLeftOuterJoin>(m_mp, pexprOuter, pexprInner, pexprScalar);
		}
		else if (IsChildOfSameLOJ(pcompInner, pcompOuter))
		{
			pexpr = CUtils::PexprLogicalJoin<CLogicalLeftOuterJoin>(m_mp, pexprInner, pexprOuter, pexprScalar);
		}
		else
		{
			if (pcompOuter->GetOuterChildIndex() > 0)
			{
				component_outerchild_index = pcompOuter->GetOuterChildIndex();
			}
			else if (pcompInner->GetOuterChildIndex() > 0)
			{
				component_outerchild_index = pcompInner->GetOuterChildIndex();
			}

			// not first call, we create an Inner Join
			pexpr = CUtils::PexprLogicalJoin<CLogicalInnerJoin>(m_mp, pexprOuter, pexprInner, pexprScalar);
		}
	}

	SComponent *combinedComponent = GPOS_NEW(m_mp) SComponent(pexpr, pbs, edge_set);
	combinedComponent->SetOuterChildIndex(component_outerchild_index);
	combinedComponent->SetInnerChildIndex(component_innerchild_index);

	return combinedComponent;
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::DeriveStats
//
//	@doc:
//		Helper function to derive stats on a given component
//
//---------------------------------------------------------------------------
void
CJoinOrder::DeriveStats
	(
	CExpression *pexpr
	)
{
	GPOS_ASSERT(NULL != pexpr);

	if (NULL != pexpr->Pstats())
	{
		// stats have been already derived
		return;
	}

	CExpressionHandle exprhdl(m_mp);
	exprhdl.Attach(pexpr);
	exprhdl.DeriveStats(m_mp, m_mp, NULL /*prprel*/, NULL /*pdrgpstatCtxt*/);
}


//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::OsPrint
//
//	@doc:
//		Helper function to print a join order class
//
//---------------------------------------------------------------------------
IOstream &
CJoinOrder::OsPrint
	(
	IOstream &os
	)
	const
{
	os	
		<< "Join Order: " << std::endl
		<< "Edges: " << m_ulEdges << std::endl;
		
	for (ULONG ul = 0; ul < m_ulEdges; ul++)
	{
		m_rgpedge[ul]->OsPrint(os);
		os << std::endl;
	}

	os << "Components: " << m_ulComps << std::endl;
	for (ULONG ul = 0; ul < m_ulComps; ul++)
	{
		os << (void*)m_rgpcomp[ul] << " - " << std::endl;
		m_rgpcomp[ul]->OsPrint(os);
	}
	
	return os;
}

BOOL
CJoinOrder::IsValidLOJCombination
	(
		SComponent *component_1,
		SComponent *component_2
	)
const
{
	// if both the participating component are inner children of an LOJ, this is an invalid combination
	if (component_1->GetInnerChildIndex() > 0 && component_2->GetInnerChildIndex() > 0)
	{
		return false;
	}

	// if the outerchild index and innerchild index do not match, this is an
	// invalid combination. In below tree, t1 and t4 should not combine. Also
	// we are banning combining t1 and t3
	//
	// NARY
	//  |_
	//  |  LOJ
	//  |   |_
	//  |   |  t1
	//  |   |_
	//  |      t2
	//  |_
	//     LOJ
	//  |   |_
	//  |   |  t3
	//  |   |_
	//         t4

	if ((component_1->GetOuterChildIndex() > 0 && component_2->GetOuterChildIndex() > 0) &&
		(component_1->GetOuterChildIndex() != component_2->GetOuterChildIndex()))
	{
		return false;
	}

	if(component_1->GetOuterChildIndex() > 0 && component_2->GetInnerChildIndex() > 0)
	{
		if (component_1->GetOuterChildIndex() != component_2->GetInnerChildIndex())
			return false;
	}

	if (component_2->GetOuterChildIndex() > 0 && component_1->GetInnerChildIndex() > 0)
	{
		if (component_2->GetOuterChildIndex() != component_1->GetInnerChildIndex())
			return false;
	}


	// check if both the components are a part of an LOJ (not necessary the same LOJ), else
	// we do not want to continue with this combination
	if ((component_1->GetOuterChildIndex() == 0 && component_2->GetInnerChildIndex() > 0) ||
		(component_2->GetOuterChildIndex() == 0 && component_1->GetInnerChildIndex() > 0))
	{
		return false;
	}

	return true;
}

BOOL
CJoinOrder::IsChildOfSameLOJ
	(
		SComponent *outer_component,
		SComponent *inner_component
	)
const
{
	// check if these components are inner and outer children of a same join
	return ((outer_component->GetOuterChildIndex() > 0 && inner_component->GetInnerChildIndex() > 0) &&
			outer_component->GetOuterChildIndex() == inner_component->GetInnerChildIndex());
}

//---------------------------------------------------------------------------
//	@function:
//		CJoinOrder::MarkUsedEdges
//
//	@doc:
//		Mark edges used by result component
//
//---------------------------------------------------------------------------
void
CJoinOrder::MarkUsedEdges
	(
	CExpression *pexpr
	)
{
	GPOS_ASSERT(NULL != pexpr);

	COperator::EOperatorId eopid = pexpr->Pop()->Eopid();
	if (0 == pexpr->Arity() ||
		(COperator::EopLogicalSelect != eopid &&
		 COperator::EopLogicalInnerJoin != eopid &&
		 COperator::EopLogicalLeftOuterJoin != eopid))
	{
		// result component does not have a scalar child, e.g. a Get node
		return;
	}

	CExpression *pexprScalar = (*pexpr) [pexpr->Arity() - 1];
	CExpressionArray *pdrgpexprScalar = CPredicateUtils::PdrgpexprConjuncts(m_mp, pexprScalar);
	const ULONG ulSizeScalar = pdrgpexprScalar->Size();

	// Find the correct edge to mark as used.  All the conjucts of the edge expr
	// must match some conjuct of the scalar expr of m_compResults for that edge
	// to be marked as used. This way edges that contain multiple conjucts are
	// also matched correctly. e.g. sometimes we have an hyper-edge like below
	//
	// +--CScalarBoolOp (EboolopAnd)
	//	|--CScalarCmp (=)
	//	|  |--CScalarIdent "l_orderkey" (87)
	//	|  +--CScalarIdent "l_orderkey" (41)
	//	+--CScalarCmp (<>)
	//	|--CScalarIdent "l_suppkey" (89)
	//	+--CScalarIdent "l_suppkey" (43)

	for (ULONG ulEdge = 0; ulEdge < m_ulEdges; ulEdge++)
	{
		SEdge *pedge = m_rgpedge[ulEdge];
		if (pedge->m_fUsed)
		{
			continue;
		}

		CExpressionArray *pdrgpexprEdge = CPredicateUtils::PdrgpexprConjuncts(m_mp, pedge->m_pexpr);
		const ULONG ulSizeEdge = pdrgpexprEdge->Size();

#ifdef GPOS_DEBUG
		CAutoRef<CBitSet> pbsScalarConjuctsMatched(GPOS_NEW(m_mp) CBitSet(m_mp));
#endif
		ULONG ulMatchCount = 0; // Count of edge predicate conjucts matched
		// For each conjuct of the edge predicate
		for (ULONG ulEdgePred = 0; ulEdgePred < ulSizeEdge; ++ulEdgePred)
		{
			// For each conjuct of the scalar predicate
			for (ULONG ulScalarPred = 0; ulScalarPred < ulSizeScalar; ulScalarPred++)
			{
				if ((*pdrgpexprScalar)[ulScalarPred] == (*pdrgpexprEdge)[ulEdgePred])
				{
					// Count the number of edge predicate conjucts matched
					ulMatchCount++;
#ifdef GPOS_DEBUG
					// Make sure each match is unique ie. each scalar conjuct matches
					// only one edge conjunct
					GPOS_ASSERT(!pbsScalarConjuctsMatched->Get(ulScalarPred));
					pbsScalarConjuctsMatched->ExchangeSet(ulScalarPred);
#endif
					break;
				}
			}
		}

		if (ulMatchCount == ulSizeEdge)
		{
			// All the predicates of the edge was matched -> Mark it as used.
			pedge->m_fUsed = true;
		}
		pdrgpexprEdge->Release();
	}
	pdrgpexprScalar->Release();
}
// EOF
