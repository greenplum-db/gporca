//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal Software, Inc.
//
//	@filename:
//		CLogicalValuesGet.cpp
//
//	@doc:
//		Implementation of values access
//---------------------------------------------------------------------------

#include "gpos/base.h"
#include "gpopt/base/CUtils.h"

#include "gpopt/operators/CExpressionHandle.h"
#include "gpopt/operators/CLogicalValuesGet.h"
#include "gpopt/metadata/CName.h"
#include "gpopt/base/CColRefSet.h"
#include "gpopt/base/CColRefTable.h"

using namespace gpopt;


// ctor - for pattern
CLogicalValuesGet::CLogicalValuesGet
	(
	IMemoryPool *pmp
	)
	:
	CLogical(pmp),
	m_pdrgpcoldesc(NULL),
	m_pdrgpdrgpdatum(NULL),
	m_pdrgpcrOutput(NULL)
{
	m_fPattern = true;
}

// ctor
CLogicalValuesGet::CLogicalValuesGet
	(
	IMemoryPool *pmp,
	DrgPcoldesc *pdrgpcoldesc,
	DrgPdrgPdatum *pdrgpdrgpdatum
	)
	:
	CLogical(pmp),
	m_pdrgpcoldesc(pdrgpcoldesc),
	m_pdrgpdrgpdatum(pdrgpdrgpdatum),
	m_pdrgpcrOutput(NULL)
{
	GPOS_ASSERT(NULL != pdrgpcoldesc);
	GPOS_ASSERT(NULL != pdrgpdrgpdatum);

	// generate a default column set for the list of column descriptors
	m_pdrgpcrOutput = PdrgpcrCreateMapping(pmp, pdrgpcoldesc, UlOpId());
	
#ifdef GPOS_DEBUG
	for (ULONG ul = 0; ul < pdrgpdrgpdatum->UlLength(); ul++)
	{
		DrgPdatum *pdrgpdatum = (*pdrgpdrgpdatum)[ul];
		GPOS_ASSERT(pdrgpdatum->UlLength() == pdrgpcoldesc->UlLength());
	}
#endif
}


// ctor
CLogicalValuesGet::CLogicalValuesGet
	(
	IMemoryPool *pmp,
	DrgPcr *pdrgpcrOutput,
	DrgPdrgPdatum *pdrgpdrgpdatum
	)
	:
	CLogical(pmp),
	m_pdrgpcoldesc(NULL),
	m_pdrgpdrgpdatum(pdrgpdrgpdatum),
	m_pdrgpcrOutput(pdrgpcrOutput)
{
	GPOS_ASSERT(NULL != pdrgpcrOutput);
	GPOS_ASSERT(NULL != pdrgpdrgpdatum);

	// generate column descriptors for the given output columns
	m_pdrgpcoldesc = PdrgpcoldescMapping(pmp, pdrgpcrOutput);

#ifdef GPOS_DEBUG
	for (ULONG ul = 0; ul < pdrgpdrgpdatum->UlLength(); ul++)
	{
		DrgPdatum *pdrgpdatum = (*pdrgpdrgpdatum)[ul];
		GPOS_ASSERT(pdrgpdatum->UlLength() == m_pdrgpcoldesc->UlLength());
	}
#endif
}

// dtor
CLogicalValuesGet::~CLogicalValuesGet()
{
	CRefCount::SafeRelease(m_pdrgpcoldesc);
	CRefCount::SafeRelease(m_pdrgpdrgpdatum);
	CRefCount::SafeRelease(m_pdrgpcrOutput);
}

// Operator specific hash function
ULONG
CLogicalValuesGet::UlHash() const
{
	ULONG ulHash = gpos::UlCombineHashes(COperator::UlHash(),
								gpos::UlCombineHashes(
										gpos::UlHashPtr<DrgPcoldesc>(m_pdrgpcoldesc),
										gpos::UlHashPtr<DrgPdrgPdatum>(m_pdrgpdrgpdatum)));
	ulHash = gpos::UlCombineHashes(ulHash, CUtils::UlHashColArray(m_pdrgpcrOutput));

	return ulHash;
}

// Match function on operator level
BOOL
CLogicalValuesGet::FMatch
	(
	COperator *pop
	)
	const
{
	if (pop->Eopid() != Eopid())
	{
		return false;
	}

	CLogicalValuesGet *popValuesGet = CLogicalValuesGet::PopConvert(pop);

	// match if column descriptors, const values and output columns are identical
	return m_pdrgpcoldesc->FEqual(popValuesGet->Pdrgpcoldesc()) &&
			m_pdrgpdrgpdatum->FEqual(popValuesGet->Pdrgpdrgpdatum()) &&
			m_pdrgpcrOutput->FEqual(popValuesGet->PdrgpcrOutput());
}

// Return a copy of the operator with remapped columns
COperator *
CLogicalValuesGet::PopCopyWithRemappedColumns
	(
	IMemoryPool *pmp,
	HMUlCr *phmulcr,
	BOOL fMustExist
	)
{
	DrgPcr *pdrgpcr = NULL;
	if (fMustExist)
	{
		pdrgpcr = CUtils::PdrgpcrRemapAndCreate(pmp, m_pdrgpcrOutput, phmulcr);
	}
	else
	{
		pdrgpcr = CUtils::PdrgpcrRemap(pmp, m_pdrgpcrOutput, phmulcr, fMustExist);
	}
	m_pdrgpdrgpdatum->AddRef();

	return GPOS_NEW(pmp) CLogicalValuesGet(pmp, pdrgpcr, m_pdrgpdrgpdatum);
}

// Derive output columns
CColRefSet *
CLogicalValuesGet::PcrsDeriveOutput
	(
	IMemoryPool *pmp,
	CExpressionHandle & // exprhdl
	)
{
	CColRefSet *pcrs = GPOS_NEW(pmp) CColRefSet(pmp);
	pcrs->Include(m_pdrgpcrOutput);

	return pcrs;
}


// Derive max card
CMaxCard
CLogicalValuesGet::Maxcard
	(
	IMemoryPool *, // pmp
	CExpressionHandle & // exprhdl
	)
	const
{
	return CMaxCard(m_pdrgpdrgpdatum->UlLength());
}


// Not called for leaf operators
BOOL
CLogicalValuesGet::FInputOrderSensitive() const
{
	GPOS_ASSERT(!"Unexpected function call of FInputOrderSensitive");
	return false;
}

// Get candidate xforms
CXformSet *
CLogicalValuesGet::PxfsCandidates
	(
	IMemoryPool *pmp
	)
	const
{
	CXformSet *pxfs = GPOS_NEW(pmp) CXformSet(pmp);
	(void) pxfs->FExchangeSet(CXform::ExfImplementConstTableGet);
	return pxfs;
}


// Construct column descriptors from column references
DrgPcoldesc *
CLogicalValuesGet::PdrgpcoldescMapping
	(
	IMemoryPool *pmp,
	DrgPcr *pdrgpcr
	)
	const
{
	GPOS_ASSERT(NULL != pdrgpcr);
	DrgPcoldesc *pdrgpcoldesc = GPOS_NEW(pmp) DrgPcoldesc(pmp);

	const ULONG ulLen = pdrgpcr->UlLength();
	for (ULONG ul = 0; ul < ulLen; ul++)
	{
		CColRef *pcr = (*pdrgpcr)[ul];

		ULONG ulLength = ULONG_MAX;
		if (CColRef::EcrtTable == pcr->Ecrt())
		{
			CColRefTable *pcrTable = CColRefTable::PcrConvert(pcr);
			ulLength = pcrTable->UlWidth();
		}

		CColumnDescriptor *pcoldesc = GPOS_NEW(pmp) CColumnDescriptor
													(
													pmp,
													pcr->Pmdtype(),
													pcr->Name(),
													ul + 1, //iAttno
													true, // FNullable
													ulLength
													);
		pdrgpcoldesc->Append(pcoldesc);
	}

	return pdrgpcoldesc;
}


// Derive statistics
IStatistics *
CLogicalValuesGet::PstatsDerive
	(
	IMemoryPool *pmp,
	CExpressionHandle &exprhdl,
	DrgPstat * // not used
	)
	const
{
	GPOS_ASSERT(Esp(exprhdl) > EspNone);
	CReqdPropRelational *prprel = CReqdPropRelational::Prprel(exprhdl.Prp());
	CColRefSet *pcrs = prprel->PcrsStat();
	DrgPul *pdrgpulColIds = GPOS_NEW(pmp) DrgPul(pmp);
	pcrs->ExtractColIds(pmp, pdrgpulColIds);
	DrgPul *pdrgpulColWidth = CUtils::Pdrgpul(pmp, m_pdrgpcrOutput);

	// TODO: Derive actual statistics with the values data instead of passing dummy stats
	IStatistics *pstats = CStatistics::PstatsDummy
										(
										pmp,
										pdrgpulColIds,
										pdrgpulColWidth,
										m_pdrgpdrgpdatum->UlLength()
										);

	// clean up
	pdrgpulColIds->Release();
	pdrgpulColWidth->Release();

	return pstats;
}


// debug print
IOstream &
CLogicalValuesGet::OsPrint
	(
	IOstream &os
	)
	const
{
	if (m_fPattern)
	{
		return COperator::OsPrint(os);
	}
	else
	{
		os << SzId() << " ";
		os << "Columns: [";
		CUtils::OsPrintDrgPcr(os, m_pdrgpcrOutput);
		os << "] ";
		os << "Values: [";
		for (ULONG ulA = 0; ulA < m_pdrgpdrgpdatum->UlLength(); ulA++)
		{
			if (0 < ulA)
			{
				os << "; ";
			}
			os << "(";
			DrgPdatum *pdrgpdatum = (*m_pdrgpdrgpdatum)[ulA];

			const ULONG ulLen = pdrgpdatum->UlLength();
			for (ULONG ulB = 0; ulB < ulLen; ulB++)
			{
				IDatum *pdatum = (*pdrgpdatum)[ulB];
				pdatum->OsPrint(os);

				if (ulB < ulLen-1)
				{
					os << ", ";
				}
			}
			os << ")";
		}
		os << "]";

	}

	return os;
}



// EOF

