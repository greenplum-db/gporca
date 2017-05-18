//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal Inc.
//
//	@filename:
//		CMDIdIndexType.h
//
//	@doc:
//		class for representing metadata objects ids and aggregate index
//---------------------------------------------------------------------------



#ifndef GPMD_CMDIdIndexType_H
#define GPMD_CMDIdIndexType_H

#include "naucrates/md/IMDId.h"

namespace gpmd
{
	using namespace gpos;

	//  class for representing metadata objects ids and aggregate index
	class CMDIdIndexType : public IMDId
	{

		public:

			//ctor
			CMDIdIndexType(){};

			// dtor
			virtual
			~CMDIdIndexType(){};

			// type of mdid
			virtual
			EMDIdType Emdidt() const;

			// string representation of mdid
			virtual
			const WCHAR *Wsz() const;

			// system id
			virtual
			CSystemId Sysid() const;

			// equality check
			virtual
			BOOL FEquals(const IMDId *pmdid) const;

			// computes the hash value for the metadata id
			virtual
			ULONG UlHash() const;

			// return true if calling object's destructor is allowed
			virtual
			BOOL FDeletable() const;

			enum EIndexType {
				EAggregate,
				ENonAggregate
			};

			// partitioned / non partioned
			EIndexType m_indexType;

			// return partioned / non partitioned
			EIndexType FIndexType();

			// set partition / non partition type table
			void FSetIndexType(EIndexType m_indexType);

			// CMDIdIndexType converter
			static
			CMDIdIndexType *PmdidConvert(IMDId *pmdid)
			{
				GPOS_ASSERT(NULL != pmdid && IMDId::EmdidGPDB == pmdid->Emdidt());
				
				return static_cast<CMDIdIndexType *>(pmdid);
			}
			
			// IMDId converter
			static
			IMDId *PmdidConvert(CMDIdIndexType *pmdidIndexType)
			{
				GPOS_ASSERT(NULL != pmdidIndexType && IMDId::EmdidGPDB == pmdidIndexType->Emdidt());
				
				return dynamic_cast<IMDId *>(pmdidIndexType);
			}
	};
}



#endif // !GPMD_CMDIdIndexType_H

// EOF
