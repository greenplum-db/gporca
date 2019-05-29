//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp.
//
//	@filename:
//		CPhysicalJoin.h
//
//	@doc:
//		Physical join base class
//---------------------------------------------------------------------------
#ifndef GPOPT_CPhysicalJoin_H
#define GPOPT_CPhysicalJoin_H

#include "gpos/base.h"

#include "gpopt/base/CDistributionSpec.h"
#include "gpopt/operators/CPhysical.h"

namespace gpopt
{
	
	// fwd declarations
	class CDistributionSpecSingleton;

	//---------------------------------------------------------------------------
	//	@class:
	//		CPhysicalJoin
	//
	//	@doc:
	//		Inner nested-loops join operator
	//
	//---------------------------------------------------------------------------
	class CPhysicalJoin : public CPhysical
	{

		private:

			// private copy ctor
			CPhysicalJoin(const CPhysicalJoin &);

			// check whether the child being processed is the child that has the part consumer
			static
			BOOL FProcessingChildWithPartConsumer
				(
				BOOL fOuterPartConsumerTest,
				ULONG ulChildIndexToTestFirst,
				ULONG ulChildIndexToTestSecond,
				ULONG child_index
				);

		protected:
		
			// Partition propagation request generated by a join
			class CPartPropReq : public CRefCount
			{
				private:

					// required part propagation
					CPartitionPropagationSpec *m_pppsRequired;

					// child index to push requirements to
					ULONG m_ulChildIndex;

					// id of outer child
					ULONG m_ulOuterChild;

					// id of inner child
					ULONG m_ulInnerChild;

					// id of scalar child
					ULONG m_ulScalarChild;

					// private copy ctor
					CPartPropReq(const CPartPropReq&);

				public:

					// ctor
					CPartPropReq
						(
						CPartitionPropagationSpec *pppsRequired,
						ULONG child_index,
						ULONG ulOuterChild,
						ULONG ulInnerChild,
						ULONG ulScalarChild
						)
						:
						m_pppsRequired(pppsRequired),
						m_ulChildIndex(child_index),
						m_ulOuterChild(ulOuterChild),
						m_ulInnerChild(ulInnerChild),
						m_ulScalarChild(ulScalarChild)
					{
						GPOS_ASSERT(NULL != pppsRequired);
					}

					// dtor
					virtual
					~CPartPropReq()
					{
						m_pppsRequired->Release();
					}

					// partition propgataion spec
					CPartitionPropagationSpec *Ppps() const
					{
						return m_pppsRequired;
					}

					// child index to push requirements to
					ULONG UlChildIndex() const
					{
						return m_ulChildIndex;
					}

					// outer child id
					ULONG UlOuterChild() const
					{
						return m_ulOuterChild;
					}

					// inner child id
					ULONG UlInnerChild() const
					{
						return m_ulInnerChild;
					}

					// scalar child id
					ULONG UlScalarChild() const
					{
						return m_ulScalarChild;
					}

					// hash function
					static
					ULONG HashValue(const CPartPropReq *pppr);

					// equality function
					static
					BOOL Equals(const CPartPropReq *ppprFst, const CPartPropReq *ppprSnd);

			}; // class CPartPropReq

			// map partition propagation request to partition propagation spec
			typedef CHashMap<CPartPropReq, CPartitionPropagationSpec, CPartPropReq::HashValue, CPartPropReq::Equals,
						CleanupRelease<CPartPropReq>, CleanupRelease<CPartitionPropagationSpec> > PartPropReqToPartPropSpecMap;

			// partition propagation request map
			PartPropReqToPartPropSpecMap *m_phmpp;

			// ctor
			explicit
			CPhysicalJoin(CMemoryPool *mp);

			// dtor
			virtual 
			~CPhysicalJoin();

			// helper to check if given child index correspond to first child to be optimized
			BOOL FFirstChildToOptimize(ULONG child_index) const;

			// helper to compute required distribution of correlated join's children
			CDistributionSpec *PdsRequiredCorrelatedJoin
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CDistributionSpec *pdsRequired,
				ULONG child_index,
				CDrvdProp2dArray *pdrgpdpCtxt,
				ULONG  ulOptReq
				)
				const;

			// helper to compute required rewindability of correlated join's children
			CRewindabilitySpec *PrsRequiredCorrelatedJoin
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CRewindabilitySpec *prsRequired,
				ULONG child_index,
				CDrvdProp2dArray *pdrgpdpCtxt,
				ULONG ulOptReq
				)
				const;

			// create partition propagation request
			CPartPropReq *PpprCreate
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CPartitionPropagationSpec *pppsRequired,
				ULONG child_index
				);

			// compute required partition propagation of the n-th child
			CPartitionPropagationSpec *PppsRequiredCompute
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CPartitionPropagationSpec *pppsRequired,
				ULONG child_index,
				BOOL fNLJoin
				);

			// helper function for computing the required partition propagation
			// spec for the children of a join
			CPartitionPropagationSpec *PppsRequiredJoinChild
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CPartitionPropagationSpec *pppsRequired,
				ULONG child_index,
				CDrvdProp2dArray *pdrgpdpCtxt,
				BOOL fNLJoin
				);

			// helper for propagating required sort order to outer child
			static
			COrderSpec *PosPropagateToOuter(CMemoryPool *mp, CExpressionHandle &exprhdl, COrderSpec *posRequired);

			// helper for checking if required sort columns come from outer child
			static
			BOOL FSortColsInOuterChild(CMemoryPool *mp, CExpressionHandle &exprhdl, COrderSpec *pos);

			// helper for checking if the outer input of a binary join operator
			// includes the required columns
			static
			BOOL FOuterProvidesReqdCols(CExpressionHandle &exprhdl, CColRefSet *pcrsRequired);

			
			// helper to add filter on part key
			static
			void AddFilterOnPartKey
				(
				CMemoryPool *mp,
				BOOL fNLJoin,
				CExpression *pexprScalar,
				CPartIndexMap *ppimSource,
				CPartFilterMap *ppfmSource,
				ULONG child_index,
				ULONG part_idx_id,
				BOOL fOuterPartConsumer,
				CPartIndexMap *ppimResult,
				CPartFilterMap *ppfmResult,
				CColRefSet *pcrsAllowedRefs
				);

			// helper to find join predicates on part keys. Returns NULL if not found
			static
			CExpression *PexprJoinPredOnPartKeys
				(
				CMemoryPool *mp,
				CExpression *pexprScalar,
				CPartIndexMap *ppimSource,
				ULONG part_idx_id,
				CColRefSet *pcrsAllowedRefs
				);

			// Do each of the given predicate children use columns from a different
			// join child?
			static
			BOOL FPredKeysSeparated(CExpression *pexprOuter, CExpression* pexprInner,
									CExpression *pexprPredOuter, CExpression *pexprPredInner);

		public:

			// match function
			BOOL Matches(COperator *pop) const;

			// sensitivity to order of inputs
			BOOL FInputOrderSensitive() const
			{
				return true;
			}

			//-------------------------------------------------------------------------------------
			// Required Plan Properties
			//-------------------------------------------------------------------------------------

			// compute required output columns of the n-th child
			virtual
			CColRefSet *PcrsRequired
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CColRefSet *pcrsRequired,
				ULONG child_index,
				CDrvdProp2dArray *pdrgpdpCtxt,
				ULONG ulOptReq
				);

			// compute required ctes of the n-th child
			virtual
			CCTEReq *PcteRequired
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CCTEReq *pcter,
				ULONG child_index,
				CDrvdProp2dArray *pdrgpdpCtxt,
				ULONG ulOptReq
				)
				const;

			// compute required distribution of the n-th child
			virtual
			CDistributionSpec *PdsRequired
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CDistributionSpec *pdsRequired,
				ULONG child_index,
				CDrvdProp2dArray *pdrgpdpCtxt,
				ULONG ulOptReq
				)
				const;
			
			// compute required partition propagation of the n-th child
			virtual
			CPartitionPropagationSpec *PppsRequired
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CPartitionPropagationSpec *pppsRequired,
				ULONG child_index,
				CDrvdProp2dArray *pdrgpdpCtxt,
				ULONG ulOptReq
				);

			// check if required columns are included in output columns
			virtual
			BOOL FProvidesReqdCols(CExpressionHandle &exprhdl, CColRefSet *pcrsRequired, ULONG ulOptReq) const;

			// distribution matching type
			virtual
			CEnfdDistribution::EDistributionMatching Edm
				(
				CReqdPropPlan *prppInput,
				ULONG child_index,
				CDrvdProp2dArray *pdrgpdpCtxt,
				ULONG ulOptReq
				);
			
			//-------------------------------------------------------------------------------------
			// Derived Plan Properties
			//-------------------------------------------------------------------------------------

			// derive sort order from outer child
			virtual
			COrderSpec *PosDerive
				(
				CMemoryPool *, // mp
				CExpressionHandle &exprhdl
				)
				const
			{
				return PosDerivePassThruOuter(exprhdl);
			}

			// derive distribution
			virtual
			CDistributionSpec *PdsDerive(CMemoryPool *mp, CExpressionHandle &exprhdl) const;

			// derive rewindability
			virtual
			CRewindabilitySpec *PrsDerive(CMemoryPool *mp, CExpressionHandle &exprhdl) const;

			// derive partition index map
			virtual
			CPartIndexMap *PpimDerive
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl,
				CDrvdPropCtxt * //pdpctxt
				)
				const
			{
				return PpimDeriveCombineRelational(mp, exprhdl);
			}
			
			// derive partition filter map
			virtual
			CPartFilterMap *PpfmDerive
				(
				CMemoryPool *mp,
				CExpressionHandle &exprhdl
				)
				const
			{
				// combine part filter maps from relational children
				return PpfmDeriveCombineRelational(mp, exprhdl);
			}


			//-------------------------------------------------------------------------------------
			// Enforced Properties
			//-------------------------------------------------------------------------------------

			// return rewindability property enforcing type for this operator
			virtual
			CEnfdProp::EPropEnforcingType EpetRewindability
				(
				CExpressionHandle &exprhdl,
				const CEnfdRewindability *per
				)
				const;
				
			// return true if operator passes through stats obtained from children,
			// this is used when computing stats during costing
			virtual
			BOOL FPassThruStats() const
			{
				return false;
			}

			//-------------------------------------------------------------------------------------
			//-------------------------------------------------------------------------------------
			//-------------------------------------------------------------------------------------

			// is given predicate hash-join compatible
			static
			BOOL FHashJoinCompatible(CExpression *pexprPred, CExpression *pexprOuter, CExpression* pexprInner);

			// return number of distribution requests for correlated join
			static
			ULONG UlDistrRequestsForCorrelatedJoin();

			static
			void AlignJoinKeyOuterInner
				(
				CExpression *pexprConjunct,
				CExpression *pexprOuter,
				CExpression *pexprInner,
				CExpression **ppexprKeyOuter,
				CExpression **ppexprKeyInner
				);

			static
			CRewindabilitySpec *PrsRequiredForNLJoinOuterChild(CMemoryPool *pmp, CExpressionHandle &exprhdl, CRewindabilitySpec *prsRequired);

	}; // class CPhysicalJoin

}

#endif // !GPOPT_CPhysicalJoin_H

// EOF
