//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 Greenplum, Inc.
//
//	@filename:
//		CIdGenerator.cpp
//
//	@doc:
//		Implementing the ULONG Counter  实现 unsigned long 的计数器
//---------------------------------------------------------------------------

#include "naucrates/dxl/CIdGenerator.h"

#include "gpos/base.h"

using namespace gpdxl;
using namespace gpos;

CIdGenerator::CIdGenerator(ULONG start_id) : id(start_id)
{
}

//---------------------------------------------------------------------------
//	@function:
//		CIdGenerator::next_id
//
//	@doc:
//		Returns the next unique id
//
//---------------------------------------------------------------------------
ULONG
CIdGenerator::next_id()
{
	return id++;
}

//---------------------------------------------------------------------------
//	@function:
//		CIdGenerator::current_id
//
//	@doc:
//		Returns the current unique id used
//
//---------------------------------------------------------------------------
ULONG
CIdGenerator::current_id()
{
	return id;
}


// EOF
