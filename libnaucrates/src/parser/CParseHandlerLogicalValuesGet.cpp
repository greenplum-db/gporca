//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal Software, Inc.
//
//	@filename:
//		CParseHandlerLogicalValuesGet.cpp
//
//	@doc:
//		Implementation of the SAX parse handler class for parsing logical
//		value scan.
//
//	@owner: 
//		
//
//	@test:
//
//---------------------------------------------------------------------------

#include "naucrates/dxl/parser/CParseHandlerLogicalValuesGet.h"
#include "naucrates/dxl/parser/CParseHandlerColDescr.h"
#include "naucrates/dxl/operators/CDXLOperatorFactory.h"
#include "naucrates/dxl/parser/CParseHandlerFactory.h"

using namespace gpdxl;

XERCES_CPP_NAMESPACE_USE

// Ctor
CParseHandlerLogicalValuesGet::CParseHandlerLogicalValuesGet
	(
	IMemoryPool *pmp,
	CParseHandlerManager *pphm,
	CParseHandlerBase *pphRoot
	)
	:
	CParseHandlerLogicalOp(pmp, pphm, pphRoot),
	m_pdrgpdrgpdxldatum(NULL),
	m_pdrgpdxldatum(NULL)
{
}

// Dtor
CParseHandlerLogicalValuesGet::~CParseHandlerLogicalValuesGet()
{
}

// Invoked by Xerces to process an opening tag
void
CParseHandlerLogicalValuesGet::StartElement
	(
	const XMLCh* const , // xmlszUri,
	const XMLCh* const xmlszLocalname,
	const XMLCh* const , // xmlszQname,
	const Attributes& attrs
	)
{
	if (0 == XMLString::compareString(CDXLTokens::XmlstrToken(EdxltokenLogicalValuesGet), xmlszLocalname))
	{
		// start of a const table operator node
		GPOS_ASSERT(0 == this->UlLength());

		// install a parse handler for the columns
		CParseHandlerBase *pphColDescr = CParseHandlerFactory::Pph(m_pmp, CDXLTokens::XmlstrToken(EdxltokenColumns), m_pphm, this);
		m_pphm->ActivateParseHandler(pphColDescr);

		// store parse handler
		this->Append(pphColDescr);
	}
	else if (0 == XMLString::compareString(CDXLTokens::XmlstrToken(EdxltokenValuesList), xmlszLocalname))
	{
		GPOS_ASSERT(NULL == m_pdrgpdrgpdxldatum);

		// initialize the array of const tuples (datum arrays)
		m_pdrgpdrgpdxldatum = GPOS_NEW(m_pmp) DrgPdrgPdxldatum(m_pmp);
	}
	else if (0 == XMLString::compareString(CDXLTokens::XmlstrToken(EdxltokenConstTuple), xmlszLocalname))
	{
		GPOS_ASSERT(NULL != m_pdrgpdrgpdxldatum); // we must have already seen a logical values list
		GPOS_ASSERT(NULL == m_pdrgpdxldatum);

		// initialize the array of datums (const tuple)
		m_pdrgpdxldatum = GPOS_NEW(m_pmp) DrgPdxldatum(m_pmp);
	}
	else if (0 == XMLString::compareString(CDXLTokens::XmlstrToken(EdxltokenDatum), xmlszLocalname))
	{
		// we must have already seen a logical values list and a const tuple
		GPOS_ASSERT(NULL != m_pdrgpdrgpdxldatum);
		GPOS_ASSERT(NULL != m_pdrgpdxldatum);

		// translate the datum and add it to the datum array
		CDXLDatum *pdxldatum = CDXLOperatorFactory::Pdxldatum(m_pphm->Pmm(), attrs, EdxltokenScalarConstValue);
		m_pdrgpdxldatum->Append(pdxldatum);
	}
	else
	{
		CWStringDynamic *pstr = CDXLUtils::PstrFromXMLCh(m_pphm->Pmm(), xmlszLocalname);
		GPOS_RAISE(gpdxl::ExmaDXL, gpdxl::ExmiDXLUnexpectedTag, pstr->Wsz());
	}
}

//---------------------------------------------------------------------------
//	@function:
//		CParseHandlerLogicalValuesGet::EndElement
//
//	@doc:
//		Invoked by Xerces to process a closing tag
//
//---------------------------------------------------------------------------
void
CParseHandlerLogicalValuesGet::EndElement
	(
	const XMLCh* const, // xmlszUri,
	const XMLCh* const xmlszLocalname,
	const XMLCh* const // xmlszQname
	)
{
	if (0 == XMLString::compareString(CDXLTokens::XmlstrToken(EdxltokenLogicalValuesGet), xmlszLocalname))
	{
		GPOS_ASSERT(1 == this->UlLength());

		CParseHandlerColDescr *pphColDescr = dynamic_cast<CParseHandlerColDescr *>((*this)[0]);
		GPOS_ASSERT(NULL != pphColDescr->Pdrgpdxlcd());

		DrgPdxlcd *pdrgpdxlcd = pphColDescr->Pdrgpdxlcd();
		pdrgpdxlcd->AddRef();

		CDXLLogicalValuesGet *pdxlopValuesGet = GPOS_NEW(m_pmp) CDXLLogicalValuesGet(m_pmp, pdrgpdxlcd, m_pdrgpdrgpdxldatum);
		m_pdxln = GPOS_NEW(m_pmp) CDXLNode(m_pmp, pdxlopValuesGet);

#ifdef GPOS_DEBUG
		pdxlopValuesGet->AssertValid(m_pdxln, false /* fValidateChildren */);
#endif // GPOS_DEBUG

		// deactivate handler
	  	m_pphm->DeactivateHandler();
	}
	else if (0 == XMLString::compareString(CDXLTokens::XmlstrToken(EdxltokenValuesList), xmlszLocalname))
	{
		// Do nothing
	}
	else if (0 == XMLString::compareString(CDXLTokens::XmlstrToken(EdxltokenConstTuple), xmlszLocalname))
	{
		GPOS_ASSERT(NULL != m_pdrgpdxldatum);
		m_pdrgpdrgpdxldatum->Append(m_pdrgpdxldatum);

		m_pdrgpdxldatum = NULL; // re-initialize for the parsing the next const tuple (if needed)
	}

	else if (0 == XMLString::compareString(CDXLTokens::XmlstrToken(EdxltokenDatum), xmlszLocalname))
	{
		// Do nothing
	}
	else
	{
		CWStringDynamic *pstr = CDXLUtils::PstrFromXMLCh(m_pphm->Pmm(), xmlszLocalname);
		GPOS_RAISE(gpdxl::ExmaDXL, gpdxl::ExmiDXLUnexpectedTag, pstr->Wsz());
	}
}
// EOF
