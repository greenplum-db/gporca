//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal Software, Inc.
//
//	@filename:
//		CScalarConstArray.h
//
//	@doc:
//		Class for scalar array of constant values.
//		This class is used to reduce the optimization time for large IN list.
//---------------------------------------------------------------------------
#ifndef GPOPT_CScalarConstArray_H
#define GPOPT_CScalarConstArray_H

#include "gpos/base.h"
#include "gpos/common/CRefCount.h"
#include "gpos/common/CDynamicPtrArray.h"
#include "naucrates/md/IMDId.h"
#include "gpopt/operators/CScalarConst.h"
#include "gpopt/operators/CScalarArray.h"

namespace gpopt
{

	using namespace gpos;
	using namespace gpmd;

	typedef CDynamicPtrArray<CScalarConst, CleanupRelease> DrgPconst;

	//---------------------------------------------------------------------------
	//	@class:
	//		CScalarConstArray
	//
	//	@doc:
	//		Scalar array for constants
	//
	//---------------------------------------------------------------------------
	class CScalarConstArray : public CScalarArray
	{
		private:
			// constant values
			DrgPconst *m_consts;

			// private copy ctor
			CScalarConstArray(const CScalarConstArray &);

		public:

			// ctor
			CScalarConstArray(IMemoryPool *pmp, IMDId *pmdidElem, IMDId *pmdidArray, BOOL fMultiDimensional, DrgPconst *pConsts);

			// dtor
			virtual 
			~CScalarConstArray();

			// ident accessors
			virtual 
			EOperatorId Eopid() const;

			// return a string for aggregate function
			virtual 
			const CHAR *SzId() const;

			// constant values
			DrgPconst *PConsts() const;

			// operator specific hash function
			ULONG UlHash() const;

			// match function
			BOOL FMatch(COperator *pop) const;

			// return the number of elements in the const array
			ULONG UlSize() const;

			// return the const element at the given position in the const array
			CScalarConst *PopConstAt(ULONG ul);

			// sensitivity to order of inputs
			BOOL FInputOrderSensitive() const;

			// return a copy of the operator with remapped columns
			virtual
			COperator *PopCopyWithRemappedColumns(
						IMemoryPool *, //pmp,
						HMUlCr *, //phmulcr,
						BOOL //fMustExist
						);

			// conversion function
			static
			CScalarConstArray *PopConvert(COperator *pop);

			// print
			IOstream &OsPrint(IOstream &os) const;

	}; // class CScalarConstArray

}


#endif // !GPOPT_CScalarConstArray_H

// EOF
