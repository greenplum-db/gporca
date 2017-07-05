//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal Software, Inc.
//
//	@filename:
//		CDXLLogicalValuesGet.h
//
//	@doc:
//		Class for representing DXL logical Value get
//		
//	@owner: 
//		
//
//	@test:
//
//---------------------------------------------------------------------------
#ifndef GPDXL_CDXLLogicalValuesGet_H
#define GPDXL_CDXLLogicalValuesGet_H

#include <gpos/assert.h>
#include <gpos/common/CDynamicPtrArray.h>
#include <gpos/types.h>
#include <naucrates/dxl/operators/CDXLColDescr.h>
#include <naucrates/dxl/operators/CDXLLogical.h>
#include <naucrates/dxl/operators/CDXLOperator.h>

namespace gpdxl
{

	// Class for representing DXL logical Values get
	class CDXLLogicalValuesGet : public CDXLLogical
	{
		private:

			// private copy ctor
			CDXLLogicalValuesGet(CDXLLogicalValuesGet&);

			// list of output column descriptors
			DrgPdxlcd *m_pdrgpdxlcd;

			// array of array of Const datums
			DrgPdrgPdxldatum *m_pdrgpdrgpdxldatumValuesList;
		
		public:
			// ctor
			CDXLLogicalValuesGet
				(
				IMemoryPool *pmp,
				DrgPdxlcd *pdrgdxlcd,
				DrgPdrgPdxldatum *pdrgpdrgpdxldatum
				);

			// dtor
			virtual
			~CDXLLogicalValuesGet();

			// operator id
			Edxlopid Edxlop() const;

			// operator name
			const CWStringConst *PstrOpName() const;

			// array of output columns
			const DrgPdxlcd *Pdrgpdxlcd() const
			{
				return m_pdrgpdxlcd;
			}

			// number of output columns
			ULONG UlArity() const
			{
				return m_pdrgpdxlcd->UlLength();
			}

			// output column descriptor at a given position
			const CDXLColDescr *Pdxlcd
				(
				ULONG ulPos
				)
				const
			{
				return (*m_pdrgpdxlcd)[ulPos];
			}

			// serialize operator in DXL format
			virtual
			void SerializeToDXL(CXMLSerializer *pxmlser, const CDXLNode *pdxln) const;

			// check if given column is defined by operator
			virtual
			BOOL FDefinesColumn(ULONG ulColId) const;

			// conversion function
			static
			CDXLLogicalValuesGet *PdxlopConvert(CDXLOperator *pdxlop);
			
#ifdef GPOS_DEBUG
			// checks whether the operator has valid structure, i.e. number and
			// types of child nodes
			void AssertValid(const CDXLNode *, BOOL fValidateChildren) const;
#endif // GPOS_DEBUG

	};
}
#endif // !GPDXL_CDXLLogicalValuesGet_H

// EOF
