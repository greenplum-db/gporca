//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp.
//
//	@filename:
//		CLogicalConstTableGetSingletonDistSpec.h
//
//	@doc:
//		Constant table accessor
//---------------------------------------------------------------------------
#ifndef GPOPT_CLogicalConstTableGetSingletonDistSpec_H
#define GPOPT_CLogicalConstTableGetSingletonDistSpec_H

#include "gpos/base.h"
#include "gpopt/operators/CLogicalConstTableGet.h"

namespace gpopt
{
	// dynamic array of datum arrays -- array owns elements
	typedef CDynamicPtrArray<DrgPdatum, CleanupRelease> DrgPdrgPdatum;

	//---------------------------------------------------------------------------
	//	@class:
	//		CLogicalConstTableGetSingletonDistSpec
	//
	//	@doc:
	//		Constant table accessor
	//
	//---------------------------------------------------------------------------
	class CLogicalConstTableGetSingletonDistSpec : public CLogicalConstTableGet
	{

		private:
			// private copy ctor
			CLogicalConstTableGetSingletonDistSpec(const CLogicalConstTableGetSingletonDistSpec &);

		public:
		
			// ctors
			explicit
			CLogicalConstTableGetSingletonDistSpec(IMemoryPool *pmp);

			CLogicalConstTableGetSingletonDistSpec
				(
				IMemoryPool *pmp,
				DrgPcoldesc *pdrgpcoldesc,
				DrgPdrgPdatum *pdrgpdrgpdatum
				);

			CLogicalConstTableGetSingletonDistSpec
				(
				IMemoryPool *pmp,
				DrgPcr *pdrgpcrOutput,
				DrgPdrgPdatum *pdrgpdrgpdatum
				);

			// dtor
			virtual 
			~CLogicalConstTableGetSingletonDistSpec();

			// ident accessors
			virtual 
			EOperatorId Eopid() const;

			// return a string for operator name
			virtual 
			const CHAR *SzId() const;

			// candidate set of xforms
			virtual
			CXformSet *PxfsCandidates(IMemoryPool *pmp) const;

			// conversion function
			static
			CLogicalConstTableGetSingletonDistSpec *PopConvert(COperator *pop);

			// return a copy of the operator with remapped columns
			virtual
			COperator *PopCopyWithRemappedColumns(IMemoryPool *pmp, HMUlCr *phmulcr, BOOL fMustExist);

	}; // class CLogicalConstTableGetSingletonDistSpec

}


#endif // !GPOPT_CLogicalConstTableGetSingletonDistSpec_H

// EOF
