//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal Software, Inc.
//
//	@filename:
//		CParseHandlerLogicalValuesGet.h
//
//	@doc:
//		Parse handler for parsing a logical value scan
//
//	@owner: 
//		
//
//	@test:
//
//---------------------------------------------------------------------------
#ifndef GPDXL_CParseHandlerLogicalValuesGet_H
#define GPDXL_CParseHandlerLogicalValuesGet_H

#include "gpos/base.h"
#include "naucrates/dxl/parser/CParseHandlerLogicalOp.h"

namespace gpdxl
{
	using namespace gpos;

	XERCES_CPP_NAMESPACE_USE

	// Parse handler for parsing a logical value scan
	class CParseHandlerLogicalValuesGet : public CParseHandlerLogicalOp
	{
		private:

			// array of datum arrays
			DrgPdrgPdxldatum *m_pdrgpdrgpdxldatum;

			// array of datums
			DrgPdxldatum *m_pdrgpdxldatum;

			// private copy ctor
			CParseHandlerLogicalValuesGet(const CParseHandlerLogicalValuesGet &);

			// process the start of an element
			void StartElement
				(
					const XMLCh* const xmlszUri, 		// URI of element's namespace
 					const XMLCh* const xmlszLocalname,	// local part of element's name
					const XMLCh* const xmlszQname,		// element's qname
					const Attributes& attr				// element's attributes
				);

			// process the end of an element
			void EndElement
				(
					const XMLCh* const xmlszUri, 		// URI of element's namespace
					const XMLCh* const xmlszLocalname,	// local part of element's name
					const XMLCh* const xmlszQname		// element's qname
				);

		public:
			// ctor
			CParseHandlerLogicalValuesGet
				(
				IMemoryPool *pmp,
				CParseHandlerManager *pphm,
				CParseHandlerBase *pphRoot
				);

			// dtor
			~CParseHandlerLogicalValuesGet();
	};
}

#endif // !GPDXL_CParseHandlerLogicalValuesGet_H

// EOF
