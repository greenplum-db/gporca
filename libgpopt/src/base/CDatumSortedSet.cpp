//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.

#include "gpopt/operators/COperator.h"
#include "gpopt/base/CDatumSortedSet.h"
#include "gpopt/operators/CScalarConst.h"
#include "gpopt/operators/CScalarConstArray.h"
#include "gpopt/base/CUtils.h"
#include "gpos/common/CAutoRef.h"

using namespace gpopt;

CDatumSortedSet::CDatumSortedSet
	(
	IMemoryPool *pmp,
	CExpression *pexprArray,
	const IComparator *pcomp
	)
	:
	DrgPdatum(pmp),
	m_fIncludesNull(false)
{
	GPOS_ASSERT(CUtils::FScalarArray(pexprArray) ||
			CUtils::FScalarConstArray(pexprArray) ||
			CUtils::FScalarArrayCoerce(pexprArray));

	if (CUtils::FScalarArrayCoerce(pexprArray))
	{
		pexprArray = (*pexprArray)[0];
	}

	ULONG ulArrayExprArity = pexprArray->UlArity();
	CScalarConstArray *popScArray = NULL;
	if (CUtils::FScalarConstArray(pexprArray))
	{
		popScArray = CScalarConstArray::PopConvert(pexprArray->Pop());
		ulArrayExprArity = popScArray->UlSize();
	}

	GPOS_ASSERT(0 < ulArrayExprArity);

	gpos::CAutoRef<DrgPdatum> aprngdatum(GPOS_NEW(pmp) DrgPdatum(pmp));
	for (ULONG ul = 0; ul < ulArrayExprArity; ul++)
	{
		CScalarConst *popScConst = NULL;
		if (NULL == popScArray)
		{
			popScConst = CScalarConst::PopConvert((*pexprArray)[ul]->Pop());
		}
		else
		{
			popScConst = popScArray->PopConstAt(ul);
		}

		IDatum *pdatum = popScConst->Pdatum();
		if (pdatum->FNull())
		{
			m_fIncludesNull = true;
		}
		else
		{
			pdatum->AddRef();
			aprngdatum->Append(pdatum);
		}
	}
	aprngdatum->Sort(&CUtils::IDatumCmp);

	// de-duplicate
	const ULONG ulRangeArrayArity = aprngdatum->UlLength();
	IDatum *pdatumPrev = (*aprngdatum)[0];
	pdatumPrev->AddRef();
	Append(pdatumPrev);
	for (ULONG ul = 1; ul < ulRangeArrayArity; ul++)
	{
		if (!pcomp->FEqual((*aprngdatum)[ul], pdatumPrev))
		{
			pdatumPrev = (*aprngdatum)[ul];
			pdatumPrev->AddRef();
			Append(pdatumPrev);
		}
	}
}

BOOL CDatumSortedSet::FIncludesNull() const
{
	return m_fIncludesNull;
}
