//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal Software, Inc.
//
//	@filename:
//		CLogicalValuesGet.h
//
//	@doc:
//		Values accessor
//---------------------------------------------------------------------------
#ifndef GPOPT_CLogicalValuesGet_H
#define GPOPT_CLogicalValuesGet_H

#include "gpos/base.h"
#include "gpopt/operators/CLogical.h"

namespace gpopt
{
	// dynamic array of datum arrays -- array owns elements
	typedef CDynamicPtrArray<DrgPdatum, CleanupRelease> DrgPdrgPdatum;

	//---------------------------------------------------------------------------
	//	@class:
	//		CLogicalValuesGet
	//
	//	@doc:
	//		Values accessor
	//
	//---------------------------------------------------------------------------
	class CLogicalValuesGet : public CLogical
	{

		private:
			// array of column descriptors: the schema of values
			DrgPcoldesc *m_pdrgpcoldesc;

			// array of datum arrays
			DrgPdrgPdatum *m_pdrgpdrgpdatum;

			// output columns
			DrgPcr *m_pdrgpcrOutput;

			// private copy ctor
			CLogicalValuesGet(const CLogicalValuesGet &);

			// construct column descriptors from column references
			DrgPcoldesc *PdrgpcoldescMapping(IMemoryPool *pmp, DrgPcr *pdrgpcr)	const;

		public:

			// ctors
			explicit
			CLogicalValuesGet(IMemoryPool *pmp);

			CLogicalValuesGet
				(
				IMemoryPool *pmp,
				DrgPcoldesc *pdrgpcoldesc,
				DrgPdrgPdatum *pdrgpdrgpdatum
				);

			CLogicalValuesGet
				(
				IMemoryPool *pmp,
				DrgPcr *pdrgpcrOutput,
				DrgPdrgPdatum *pdrgpdrgpdatum
				);

			// dtor
			virtual
			~CLogicalValuesGet();

			// ident accessors
			virtual
			EOperatorId Eopid() const
			{
				return EopLogicalValuesGet;
			}

			// return a string for operator name
			virtual
			const CHAR *SzId() const
			{
				return "CLogicalValuesGet";
			}

			// col descr accessor
			DrgPcoldesc *Pdrgpcoldesc() const
			{
				return m_pdrgpcoldesc;
			}

			// values accessor
			DrgPdrgPdatum *Pdrgpdrgpdatum () const
			{
				return m_pdrgpdrgpdatum;
			}

			// output columns accessors
			DrgPcr *PdrgpcrOutput() const
			{
				return m_pdrgpcrOutput;
			}

			// sensitivity to order of inputs
			BOOL FInputOrderSensitive() const;

			// operator specific hash function
			virtual
			ULONG UlHash() const;

			// match function
			virtual
			BOOL FMatch(COperator *pop) const;

			// return a copy of the operator with remapped columns
			virtual
			COperator *PopCopyWithRemappedColumns(IMemoryPool *pmp, HMUlCr *phmulcr, BOOL fMustExist);

			//-------------------------------------------------------------------------------------
			// Derived Relational Properties
			//-------------------------------------------------------------------------------------

			// derive output columns
			virtual
			CColRefSet *PcrsDeriveOutput(IMemoryPool *, CExpressionHandle &);

			// derive max card
			virtual
			CMaxCard Maxcard(IMemoryPool *pmp, CExpressionHandle &exprhdl) const;

			// derive partition consumer info
			virtual
			CPartInfo *PpartinfoDerive
				(
				IMemoryPool *pmp,
				CExpressionHandle & //exprhdl
				)
				const
			{
				return GPOS_NEW(pmp) CPartInfo(pmp);
			}

			// derive constraint property
			virtual
			CPropConstraint *PpcDeriveConstraint
				(
				IMemoryPool *pmp,
				CExpressionHandle & // exprhdl
				)
				const
			{
				// compute constraints based on the
				// datum values
				return GPOS_NEW(pmp) CPropConstraint(pmp, GPOS_NEW(pmp) DrgPcrs(pmp), NULL /*pcnstr*/);
			}

			//-------------------------------------------------------------------------------------
			// Required Relational Properties
			//-------------------------------------------------------------------------------------

			// compute required stat columns of the n-th child
			virtual
			CColRefSet *PcrsStat
				(
				IMemoryPool *,// pmp
				CExpressionHandle &,// exprhdl
				CColRefSet *,// pcrsInput
				ULONG // ulChildIndex
				)
				const
			{
				GPOS_ASSERT(!"CLogicalValuesGet has no children");
				return NULL;
			}

			// derive statistics
			virtual
			IStatistics *PstatsDerive
						(
						IMemoryPool *pmp,
						CExpressionHandle &exprhdl,
						DrgPstat *pdrgpstatCtxt
						)
						const;

			//-------------------------------------------------------------------------------------
			// Transformations
			//-------------------------------------------------------------------------------------

			// candidate set of xforms
			virtual
			CXformSet *PxfsCandidates(IMemoryPool *pmp) const;

			// stat promise
			virtual
			EStatPromise Esp(CExpressionHandle &) const
			{
				return CLogical::EspLow;
			}

			//-------------------------------------------------------------------------------------
			//-------------------------------------------------------------------------------------
			//-------------------------------------------------------------------------------------

			// conversion function
			static
			CLogicalValuesGet *PopConvert
				(
				COperator *pop
				)
			{
				GPOS_ASSERT(NULL != pop);
				GPOS_ASSERT(EopLogicalValuesGet == pop->Eopid());

				return dynamic_cast<CLogicalValuesGet*>(pop);
			}


			// debug print
			virtual
			IOstream &OsPrint(IOstream &) const;

	}; // class CLogicalValuesGet

}


#endif // !GPOPT_CLogicalValuesGet_H

// EOF
