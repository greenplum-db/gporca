//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2018 Pivotal Inc.
//
//	@filename:
//		CExpressionPostProcessor.h
//
//	@doc:
//		Expression plan tree post processing routines
//
//---------------------------------------------------------------------------
#ifndef GPOPT_CExpressionPostProcessor_H
#define GPOPT_CExpressionPostProcessor_H

#include "gpos/base.h"
#include "gpopt/operators/CExpression.h"
#include "gpos/memory/IMemoryPool.h"

namespace gpopt
{

	using namespace gpos;

	//---------------------------------------------------------------------------
	//	@class:
	//		CExpressionPostProcessor
	//---------------------------------------------------------------------------
	class CExpressionPostProcessor
	{

		public:

			// ctor
			CExpressionPostProcessor();

			// dtor
			virtual
			~CExpressionPostProcessor(){};

			// copy ctor
			CExpressionPostProcessor(const CExpressionPostProcessor &);

			// main driver
			static
			CExpression *PostProcessPlan(IMemoryPool *mp, CExpression *expression_plan);

			static
			CExpression *AddRandomMotionBeforeDMLInsertOrCTAS(IMemoryPool *mp, CExpression *pexpr);

			static
			BOOL IsRandomMotionRequired(CExpression *pexpr);

	}; // class CExpressionPostProcessor
}


#endif // !GPOPT_CExpressionPostProcessor_H

// EOF
