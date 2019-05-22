
//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2008 - 2010 Greenplum, Inc.
//
//	@filename:
//		CList.h
//
//	@doc:
//		Template-based list class;
//		In order to be useful for system programming the class must be
//		allocation-less, i.e. manage elements without additional allocation,
//		to work in exception or OOM situations
//---------------------------------------------------------------------------
#ifndef GPOS_CLink_H
#define GPOS_CLink_H


namespace gpos
{

	//---------------------------------------------------------------------------
	//	@class:
	//		SLink
	//
	//	@doc:
	//		Generic link to be embedded in all classes before they can use
	//		allocation-less lists, e.g. in synchronized hashtables etc.
	//
	//---------------------------------------------------------------------------
	class SLink
	{

		private:

			// no copy constructor
			SLink(const SLink&);

		public:

			// link forward/backward
			void *m_next;
			void *m_prev;

			// ctor
			SLink()
				:
				m_next(NULL),
				m_prev(NULL)
			{}
	};

}

#endif // !GPOS_CLink_H

// EOF

