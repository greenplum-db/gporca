//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 EMC Corp.
//
//	@filename:
//		CPhysicalConstTableGetSingletonDistSpec.h
//
//	@doc:
//		Physical const table get
//---------------------------------------------------------------------------
#ifndef GPOPT_CPhysicalConstTableGetSingletonDistSpec_H
#define GPOPT_CPhysicalConstTableGetSingletonDistSpec_H

#include "gpos/base.h"
#include "gpopt/operators/CPhysicalConstTableGet.h"


namespace gpopt
{
	
	//---------------------------------------------------------------------------
	//	@class:
	//		CPhysicalConstTableGetSingletonDistSpec
	//
	//	@doc:
	//		Physical const table get operator
	//
	//---------------------------------------------------------------------------
	class CPhysicalConstTableGetSingletonDistSpec : public CPhysicalConstTableGet
	{

		private:
			// private copy ctor
			CPhysicalConstTableGetSingletonDistSpec(const CPhysicalConstTableGetSingletonDistSpec &);

		public:
		
			// ctor
			CPhysicalConstTableGetSingletonDistSpec
				(
				IMemoryPool *pmp,
				DrgPcoldesc *pdrgpcoldesc,
				DrgPdrgPdatum *pdrgpdrgpconst,
				DrgPcr *pdrgpcrOutput
				);

			// dtor
			virtual 
			~CPhysicalConstTableGetSingletonDistSpec();

			// ident accessors
			virtual 
			EOperatorId Eopid() const;

			virtual 
			const CHAR *SzId() const;

			// match function
			BOOL FMatch(COperator *) const;

			// derive distribution
			virtual
			CDistributionSpec *PdsDerive(IMemoryPool *pmp, CExpressionHandle &exprhdl) const;

			// conversion function
			static
			CPhysicalConstTableGetSingletonDistSpec *PopConvert(COperator *pop);
		
	}; // class CPhysicalConstTableGetSingletonDistSpec

}

#endif // !GPOPT_CPhysicalConstTableGetSingletonDistSpec_H

// EOF
