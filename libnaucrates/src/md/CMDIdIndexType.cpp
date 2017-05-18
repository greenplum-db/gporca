//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2017 Pivotal Inc.
//
//	@filename:
//		IMDIdIndexTblType.cpp
//
//	@doc:
//		class for representing metadata objects ids and aggregate index
//---------------------------------------------------------------------------

#include "naucrates/md/CMDIdIndexType.h"

using namespace gpmd;

CMDIdIndexType::EIndexType
CMDIdIndexType::FIndexType()
{
	return m_indexType;
}

void
CMDIdIndexType::FSetIndexType
	(
	 EIndexType indexType
	)
{
	m_indexType = indexType;
}

// EOF

