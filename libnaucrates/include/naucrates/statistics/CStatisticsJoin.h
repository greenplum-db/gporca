//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 EMC Corp.
//
//	@filename:
//		CStatisticsJoin.h
//
//	@doc:
//		Join on statistics
//---------------------------------------------------------------------------
#ifndef GPNAUCRATES_CStatisticsJoin_H
#define GPNAUCRATES_CStatisticsJoin_H

#include "gpos/base.h"
#include "gpos/common/CRefCount.h"
#include "naucrates/md/IMDType.h"

namespace gpnaucrates
{
	using namespace gpos;
	using namespace gpmd;

	//---------------------------------------------------------------------------
	//	@class:
	//		CStatisticsJoin
	//
	//	@doc:
	//		Join on statistics
	//---------------------------------------------------------------------------
	class CStatisticsJoin : public CRefCount
	{
		public:
			enum EJoinCardAlgorithm
			{
				EjcaDefault, // use histogram as well as total number of distinct values and its frequencies to determine join cardinality
				EjcaNDV, // do not trust histogram boundaries, instead use total number of distinct values and its frequencies
				EjcaGeneric // should be the last in this enum
			};

		private:

			// private copy ctor
			CStatisticsJoin(const CStatisticsJoin &);

			// private assignment operator
			CStatisticsJoin& operator=(CStatisticsJoin &);

			// column id
			ULONG m_ulColId1;

			// comparison type
			CStatsPred::EStatsCmpType m_escmpt;

			// column id
			ULONG m_ulColId2;

			CStatisticsJoin::EJoinCardAlgorithm m_ejca;

		public:

			// c'tor
			CStatisticsJoin
				(
				ULONG ulColId1,
				CStatsPred::EStatsCmpType escmpt,
				ULONG ulColId2,
				CStatisticsJoin::EJoinCardAlgorithm ejca
				)
				:
				m_ulColId1(ulColId1),
				m_escmpt(escmpt),
				m_ulColId2(ulColId2),
				m_ejca(ejca)
			{
				GPOS_ASSERT(EjcaGeneric != m_ejca);
			}

			// accessors
			ULONG UlColId1() const
			{
				return m_ulColId1;
			}

			// comparison type
			CStatsPred::EStatsCmpType Escmpt() const
			{
				return m_escmpt;
			}

			ULONG UlColId2() const
			{
				return m_ulColId2;
			}

			CStatisticsJoin::EJoinCardAlgorithm Ejca() const
			{
				return m_ejca;
			}

			// d'tor
			virtual
			~CStatisticsJoin()
			{}

	}; // class CStatisticsJoin

	// array of filters
	typedef CDynamicPtrArray<CStatisticsJoin, CleanupRelease> DrgPstatsjoin;
}

#endif // !GPNAUCRATES_CStatisticsJoin_H

// EOF
