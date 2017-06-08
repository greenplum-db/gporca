//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 EMC Corp.
//
//	@filename:
//		CXformImplementConstTableGetSingletonDistSpec.h
//
//	@doc:
//		Implement logical const table with a physical const table get
//---------------------------------------------------------------------------
#ifndef GPOPT_CXformImplementConstTableGetSingletonDistSpec_H
#define GPOPT_CXformImplementConstTableGetSingletonDistSpec_H

#include "gpos/base.h"
#include "gpopt/xforms/CXformImplementation.h"

namespace gpopt
{
	using namespace gpos;
	
	//---------------------------------------------------------------------------
	//	@class:
	//		CXformImplementConstTableGetSingletonDistSpec
	//
	//	@doc:
	//		Implement const table get
	//
	//---------------------------------------------------------------------------
	class CXformImplementConstTableGetSingletonDistSpec : public CXformImplementation
	{

		private:

			// private copy ctor
			CXformImplementConstTableGetSingletonDistSpec(const CXformImplementConstTableGetSingletonDistSpec &);

		public:
		
			// ctor
			explicit
			CXformImplementConstTableGetSingletonDistSpec(IMemoryPool *);

			// dtor
			virtual 
			~CXformImplementConstTableGetSingletonDistSpec() {}

			// ident accessors
			virtual
			EXformId Exfid() const;

			// return a string for xform name
			virtual 
			const CHAR *SzId() const;

			// compute xform promise for a given expression handle
			virtual
			EXformPromise Exfp(CExpressionHandle & /* exprhdl */) const;

			// actual transform
			void Transform
				(
				CXformContext *pxfctxt,
				CXformResult *pxfres,
				CExpression *pexpr
				) 
				const;
		
	}; // class CXformImplementConstTableGetSingletonDistSpec

}


#endif // !GPOPT_CXformImplementConstTableGetSingletonDistSpec_H

// EOF
