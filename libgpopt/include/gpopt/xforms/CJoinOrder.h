//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp.
//
//	@filename:
//		CJoinOrder.h
//
//	@doc:
//		Join Order Generation
//---------------------------------------------------------------------------
#ifndef GPOPT_CJoinOrder_H
#define GPOPT_CJoinOrder_H

#include "gpos/base.h"
#include "gpopt/operators/CExpression.h"
#include "gpos/io/IOstream.h"

namespace gpopt
{
	using namespace gpos;
	
	//---------------------------------------------------------------------------
	//	@class:
	//		CJoinOrder
	//
	//	@doc:
	//		Helper class for creating compact join orders
	//
	//---------------------------------------------------------------------------
	class CJoinOrder
	{
		public:

			//---------------------------------------------------------------------------
			//	@struct:
			//		SEdge
			//
			//	@doc:
			//		Struct to capture edge
			//
			//---------------------------------------------------------------------------
			struct SEdge : public CRefCount
			{
				// cover of edge
				CBitSet *m_pbs;
				
				// associated conjunct
				CExpression *m_pexpr;
				
				// a flag to mark edge as used
				BOOL m_fUsed;

				// ctor
				SEdge(IMemoryPool *mp, CExpression *pexpr);
				
				// dtor
				~SEdge();
				
				// print routine
				IOstream &OsPrint(IOstream &os) const;
			};
			
		
			//---------------------------------------------------------------------------
			//	@struct:
			//		SComponent
			//
			//	@doc:
			//		Struct to capture component
			//
			//---------------------------------------------------------------------------
			struct SComponent : public CRefCount
			{
				// cover
				CBitSet *m_pbs;

				// set of edges associated with this component (stored as indexes into m_rgpedge array)
				CBitSet *m_edge_set;

				// associated expression
				CExpression *m_pexpr;
				
				// a flag to component edge as used
				BOOL m_fUsed;

				INT outerchild_index;

				INT innerchild_index;

				// ctor
				SComponent(IMemoryPool *mp, CExpression *pexpr);
				
				// ctor
				SComponent(CExpression *pexpr, CBitSet *pbs, CBitSet *edge_set);

				// dtor
				~SComponent();

				// print routine
				IOstream &OsPrint(IOstream &os) const;

				// set outer child index
				void SetOuterChildIndex(INT index);

				// set inner child index
				void SetInnerChildIndex(INT index);

				INT GetOuterChildIndex() { return outerchild_index; }

				INT GetInnerChildIndex() { return innerchild_index; }
			};

		protected:
				
			// memory pool
			IMemoryPool *m_mp;
			
			// edges
			SEdge **m_rgpedge;
			
			// number of edges
			ULONG m_ulEdges;
			
			// components
			SComponent **m_rgpcomp;
			
			// number of components
			ULONG m_ulComps;

			// should we optimize outer joins
			BOOL m_include_left_outer_join_rels;
			
			// compute cover of each edge
			void ComputeEdgeCover();

			// combine the two given components using applicable edges
			SComponent *PcompCombine(SComponent *pcompOuter, SComponent *pcompInner);

			// derive stats on a given component
			virtual
			void DeriveStats(CExpression *pexpr);

		private:

			// private copy ctor
			CJoinOrder(const CJoinOrder &);

		public:

			// ctor
			CJoinOrder
				(
				IMemoryPool *mp,
				CExpressionArray *pdrgpexprComponents,
				CExpressionArray *pdrgpexprConjuncts,
				BOOL include_outer_join_rels
				);
		
			// dtor
			virtual
			~CJoinOrder();			
		
			// print function
			virtual
			IOstream &OsPrint(IOstream &) const;

			BOOL IsValidOuterJoinCombination(SComponent *component_1, SComponent *component_2) const;

			BOOL IsChildOfSameLOJ(SComponent *outer_component, SComponent *inner_component) const;

	}; // class CJoinOrder

}

#endif // !GPOPT_CJoinOrder_H

// EOF
