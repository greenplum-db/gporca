//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal Software, Inc.
//
//	@filename:
//		CDXLLogicalValuesGet.cpp
//
//	@doc:
//		Implementation of DXL logical Value scan
//		
//	@owner: 
//		
//
//	@test:
//
//---------------------------------------------------------------------------

#include "gpos/string/CWStringDynamic.h"

#include "naucrates/dxl/CDXLUtils.h"
#include "naucrates/dxl/operators/CDXLLogicalValuesGet.h"
#include "naucrates/dxl/operators/CDXLNode.h"
#include "naucrates/dxl/xml/CXMLSerializer.h"


using namespace gpos;
using namespace gpdxl;

// Ctor
CDXLLogicalValuesGet::CDXLLogicalValuesGet
	(
	IMemoryPool *pmp,
	DrgPdxlcd *pdrgdxlcd,
	DrgPdrgPdxldatum *pdrgpdrgpdxldatum
	)
	:
	CDXLLogical(pmp),
	m_pdrgpdxlcd(pdrgdxlcd),
	m_pdrgpdrgpdxldatumValuesList(pdrgpdrgpdxldatum)
{
	GPOS_ASSERT(NULL != m_pdrgpdxlcd);
	GPOS_ASSERT(NULL != m_pdrgpdrgpdxldatumValuesList);
	
#ifdef GPOS_DEBUG	
	const ULONG ulCols = m_pdrgpdxlcd->UlLength();
	const ULONG ulLen = m_pdrgpdrgpdxldatumValuesList->UlLength();
	for (ULONG ul = 0; ul < ulLen; ul++)
	{
		DrgPdxldatum *pdrgpdatum = (*m_pdrgpdrgpdxldatumValuesList)[ul];
		GPOS_ASSERT(ulCols == pdrgpdatum->UlLength());
	}

#endif	
}

// Dtor
CDXLLogicalValuesGet::~CDXLLogicalValuesGet()
{
	m_pdrgpdxlcd->Release();
	m_pdrgpdrgpdxldatumValuesList->Release();
}


// Operator type
Edxlopid
CDXLLogicalValuesGet::Edxlop() const
{
	return EdxlopLogicalValuesGet;
}


// Operator name
const CWStringConst *
CDXLLogicalValuesGet::PstrOpName() const
{
		return CDXLTokens::PstrToken(EdxltokenLogicalValuesGet);
}

// Serialize operator in DXL format
void
CDXLLogicalValuesGet::SerializeToDXL
	(
	CXMLSerializer *pxmlser,
	const CDXLNode *// pdxln
	)
	const
{
	const CWStringConst *pstrElemName = PstrOpName();
	pxmlser->OpenElement(CDXLTokens::PstrToken(EdxltokenNamespacePrefix), pstrElemName);

	// serialize output columns
	pxmlser->OpenElement(CDXLTokens::PstrToken(EdxltokenNamespacePrefix), CDXLTokens::PstrToken(EdxltokenColumns));
	GPOS_ASSERT(NULL != m_pdrgpdxlcd);

	const ULONG ulLen = m_pdrgpdxlcd->UlLength();
	for (ULONG ul = 0; ul < ulLen; ul++)
	{
		CDXLColDescr *pdxlcd = (*m_pdrgpdxlcd)[ul];
		pdxlcd->SerializeToDXL(pxmlser);
	}
	pxmlser->CloseElement(CDXLTokens::PstrToken(EdxltokenNamespacePrefix), CDXLTokens::PstrToken(EdxltokenColumns));

	// serialize Values List
	pxmlser->OpenElement(CDXLTokens::PstrToken(EdxltokenNamespacePrefix), CDXLTokens::PstrToken(EdxltokenValuesList));
	GPOS_ASSERT(NULL != m_pdrgpdrgpdxldatumValuesList);

	const ULONG ulTuples = m_pdrgpdrgpdxldatumValuesList->UlLength();
	for (ULONG ulTuplePos = 0; ulTuplePos < ulTuples; ulTuplePos++)
	{
		// serialize a const tuple
		pxmlser->OpenElement(CDXLTokens::PstrToken(EdxltokenNamespacePrefix), CDXLTokens::PstrToken(EdxltokenConstTuple));
		DrgPdxldatum *pdrgpdxldatum = (*m_pdrgpdrgpdxldatumValuesList)[ulTuplePos];

		const ULONG ulCols = pdrgpdxldatum->UlLength();
		for (ULONG ulColPos = 0; ulColPos < ulCols; ulColPos++)
		{
			CDXLDatum *pdxldatum = (*pdrgpdxldatum)[ulColPos];
			pdxldatum->Serialize(pxmlser, CDXLTokens::PstrToken(EdxltokenDatum));
		}

		pxmlser->CloseElement(CDXLTokens::PstrToken(EdxltokenNamespacePrefix), CDXLTokens::PstrToken(EdxltokenConstTuple));
	}

	pxmlser->CloseElement(CDXLTokens::PstrToken(EdxltokenNamespacePrefix), CDXLTokens::PstrToken(EdxltokenValuesList));

	pxmlser->CloseElement(CDXLTokens::PstrToken(EdxltokenNamespacePrefix), pstrElemName);
}

// Check if given column is defined by operator
BOOL
CDXLLogicalValuesGet::FDefinesColumn
	(
	ULONG ulColId
	)
	const
{
	const ULONG ulSize = UlArity();
	for (ULONG ulDescr = 0; ulDescr < ulSize; ulDescr++)
	{
		ULONG ulId = Pdxlcd(ulDescr)->UlID();
		if (ulId == ulColId)
		{
			return true;
		}
	}

	return false;
}

// conversion function
CDXLLogicalValuesGet *
CDXLLogicalValuesGet::PdxlopConvert
	(
	CDXLOperator *pdxlop
	)
{
	GPOS_ASSERT(NULL != pdxlop);
	GPOS_ASSERT(EdxlopLogicalValuesGet == pdxlop->Edxlop());
	
	return dynamic_cast<CDXLLogicalValuesGet*>(pdxlop);
}

#ifdef GPOS_DEBUG

// Checks whether operator node is well-structured
void
CDXLLogicalValuesGet::AssertValid
	(
	const CDXLNode *pdxln,
	BOOL // fValidateChildren
	) const
{
	// assert validity of col descr
	GPOS_ASSERT(0 < m_pdrgpdxlcd->UlSafeLength());
	GPOS_ASSERT(0 == pdxln->UlArity());
}

#endif // GPOS_DEBUG

// EOF
