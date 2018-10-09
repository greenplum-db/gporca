//---------------------------------------------------------------------------

//  Greenplum Database
//  Copyright (C) 2018 Pivotal Inc.
//
//  @filename:
//      CXformEagerAgg.h
//
//  @doc:
//      Eagerly push aggregates below join when there is no primary/foreign keys
//---------------------------------------------------------------------------
#ifndef GPOPT_CXformEagerAgg_H
#define GPOPT_CXformEagerAgg_H

#include "gpos/base.h"
#include "gpopt/xforms/CXformExploration.h"

namespace gpopt
{
    using namespace gpos;

    //---------------------------------------------------------------------------
    //  @class:
    //      CXformEagerAgg
    //
    //  @doc:
    //      Eagerly push aggregates below join when there is no primary/foreign keys
    //
    //---------------------------------------------------------------------------
    class CXformEagerAgg : public CXformExploration
    {

        private:

            // private copy ctor
            CXformEagerAgg(const CXformEagerAgg &);


        public:

            // ctor
            explicit
            CXformEagerAgg(IMemoryPool *mp);

            // ctor
            explicit
            CXformEagerAgg(CExpression *pexprPattern);

            // dtor
            virtual
            ~CXformEagerAgg()
            {}

            // ident accessors
            virtual
            EXformId Exfid() const
            {
                return ExfEagerAgg;
            }

            virtual
            const CHAR *SzId() const
            {
                return "CXformEagerAgg";
            }

            // compatibility function for eager aggregation
            virtual
            BOOL FCompatible(CXform::EXformId exfid)
            {
                return (CXform::ExfEagerAgg != exfid) &&
                            (CXform::ExfSplitGbAgg != exfid);
            }

            // compute xform promise for a given expression handle
            virtual
            EXformPromise Exfp(CExpressionHandle &exprhdl) const;

            // actual transform
            void Transform(CXformContext *pxfctxt, CXformResult *pxfres, CExpression *pexpr) const;

            // check if Transform can be applied
            BOOL FApplicable(CExpression *pexpr) const;

            static
            void PopulateLowerUpperProjectList
            (
             IMemoryPool *mp,               // memory pool
             CExpression *orig_proj_list,   // project list of the original global aggregate
             CExpression **lower_proj_list, // project list of the new lower aggregate
             CExpression **upper_proj_list  // project list of the new upper aggregate
            );

            // return true if xform should be applied only once
            virtual
            BOOL IsApplyOnce(){
                return true;
             };
    }; // class CXformEagerAgg

}

#endif // !GPOPT_CXformEagerAgg_H

// EOF
