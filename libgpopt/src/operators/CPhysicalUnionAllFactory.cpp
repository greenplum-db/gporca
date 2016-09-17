//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.


#include "gpopt/operators/CPhysicalUnionAllFactory.h"
#include "gpopt/operators/CPhysicalSerialUnionAll.h"
#include "gpopt/operators/CPhysicalParallelUnionAll.h"
#include "gpopt/xforms/CXformUtils.h"
#include "gpopt/exception.h"
#include "gpos/base.h"

namespace gpopt
{

	CPhysicalUnionAllFactory::CPhysicalUnionAllFactory
		(
			CLogicalUnionAll *popLogicalUnionAll,
			BOOL fParallel
		)
		: m_popLogicalUnionAll(popLogicalUnionAll), m_fParallel(fParallel) { }

	CPhysicalUnionAll *CPhysicalUnionAllFactory::PopPhysicalUnionAll(IMemoryPool *pmp)
	{

		DrgPcr *pdrgpcrOutput = m_popLogicalUnionAll->PdrgpcrOutput();
		DrgDrgPcr *pdrgpdrgpcrInput = m_popLogicalUnionAll->PdrgpdrgpcrInput();

		// TODO:  May 2nd 2012; support compatible types
		if (!CXformUtils::FSameDatatype(pdrgpdrgpcrInput))
		{
			GPOS_RAISE(gpopt::ExmaGPOPT, gpopt::ExmiUnsupportedOp, GPOS_WSZ_LIT("Union of non-identical types"));
		}

		pdrgpcrOutput->AddRef();
		pdrgpdrgpcrInput->AddRef();

		GPOS_ASSERT(1 < pdrgpdrgpcrInput->UlLength());
		DrgPcr *pdrgpcr = (*pdrgpdrgpcrInput)[0];
		ULONG ulCols = pdrgpcr->UlLength();
		BOOL fCanParallel = false;

		// If parallel union is enable and there is at least 1 hashable column,
		// we can do parallel union. Otherwise, we do serial union regardless of
		// whether the parallel union is enabled or not.
		//
		// We only need to check the columns of the first relation child of union,
		// because the number of columns and data type for each column of each
		// relation child should match every other children of union operator.
		// Otherwise, we will not reach here.
		for (ULONG ul = 0; m_fParallel && ul < ulCols; ul++)
		{
			CColRef *pcr = (*pdrgpcr)[ul];
			if (pcr->Pmdtype()->FHashable())
			{
				fCanParallel = true;
				break;
			}
		}

		if (m_fParallel && fCanParallel)
		{
			return GPOS_NEW(pmp) CPhysicalParallelUnionAll
				(
					pmp,
					pdrgpcrOutput,
					pdrgpdrgpcrInput,
					m_popLogicalUnionAll->UlScanIdPartialIndex()
				);
		}
		else
		{
			return GPOS_NEW(pmp) CPhysicalSerialUnionAll
				(
					pmp,
					pdrgpcrOutput,
					pdrgpdrgpcrInput,
					m_popLogicalUnionAll->UlScanIdPartialIndex()
				);

		}

	}

}
