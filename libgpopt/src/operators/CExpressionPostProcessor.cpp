//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2018 Pivotal Inc.
//
//	@filename:
//		CExpressionPostProcessor.cpp
//
//	@doc:
//		Expression plan tree post processing routines
//---------------------------------------------------------------------------

#include "gpopt/operators/CExpressionPostProcessor.h"
#include "gpos/memory/IMemoryPool.h"
#include "gpos/common/CAutoTimer.h"
#include "gpopt/operators/CPhysicalDML.h"
#include "gpopt/base/CDistributionSpecRandom.h"
#include "gpopt/operators/CPhysicalMotionRandom.h"


using namespace gpopt;

CExpressionPostProcessor::CExpressionPostProcessor()
{}

// add a random motion before dml insert /ctas if the input tree does not have
// random motion ensuring randomness
CExpression *
CExpressionPostProcessor::AddRandomMotionBeforeDMLInsertOrCTAS
	(
	IMemoryPool *mp,
	CExpression *expr
	)
{
	// applicable only for dml insert operation on randomly distributed table
	if (expr->Pop()->Eopid() == COperator::EopPhysicalDML)
	{
		CPhysicalDML *phy_dml_op = CPhysicalDML::PopConvert(expr->Pop());
		if (CLogicalDML::EdmlInsert == phy_dml_op->Edmlop() &&
			IMDRelation::EreldistrRandom == phy_dml_op->Ptabdesc()->Ereldistribution() &&
			IsRandomMotionRequired(expr))
		{
			// create a new spec and a random motion operator
			CDistributionSpecRandom *random_spec_new = GPOS_NEW(mp) CDistributionSpecRandom();
			CPhysicalMotionRandom *new_random_motion_op = GPOS_NEW(mp) CPhysicalMotionRandom(mp, random_spec_new);

			// create a new random motion expression with the child of dml expression
			CExpression *sub_tree_expr = (*expr)[0]; // child of dml expression
			sub_tree_expr->AddRef();
			CExpression *random_motion_expr_new = GPOS_NEW(mp) CExpression(mp, new_random_motion_op, sub_tree_expr);
			random_motion_expr_new->CopyExpressionProp(sub_tree_expr);

			// create a new dml expression with the new random motion expression
			phy_dml_op->AddRef();
			CExpression *phy_dml_expr_new = GPOS_NEW(mp) CExpression(mp, phy_dml_op, random_motion_expr_new);
			phy_dml_expr_new->CopyExpressionProp(expr);

			return phy_dml_expr_new;
		}
	}

	// return the unmodified tree
	expr->AddRef();
	return expr;
}

BOOL
CExpressionPostProcessor::IsRandomMotionRequired
	(
	CExpression *expr
	)
{
	// protect against stack overflow during recursion
	GPOS_CHECK_STACK_SIZE;
	GPOS_ASSERT(NULL != expr);

	// if a random motion exists, need not further traverse down the tree.
	// if the random motion presents a duplicate hazard, this random motion
	// will be converted into a result node with a hash filter and
	// additional redistribute is required for random distribution.
	// Otherwise it will be converted into redistribute motion node and there is
	// no need for additional redistribute
	if (COperator::EopPhysicalMotionRandom == expr->Pop()->Eopid())
	{
		return CUtils::FDuplicateHazardMotion(expr);
	}

	// Consider a dml insert/ctas on a randomly distributed table, and the child
	// of the DML node provides a random distribution spec. In this scenario,
	// we are guaranteed that there no random motion in the subtree at this stage
	// and there is a possibility that the tuples are skewed when the plan is
	// executed. To avoid this, we should enforce a random motion to redistribute
	// the tuples on all segments.
	ULONG num_of_child = expr->UlArity();
	if (num_of_child == 0 && CDistributionSpec::EdtRandom == CDrvdPropPlan::Pdpplan(expr->PdpDerive())->Pds()->Edt())
	{
		return true;
	}

	BOOL is_random_motion_required = false;
	for (ULONG idx = 0; idx < num_of_child; idx++)
	{
		CExpression *expr_child = (*expr)[idx];
		// scalar child does not impact the distribution spec, so skip scalar childs
		if (expr_child->Pop()->FScalar())
		{
			continue;
		}
		is_random_motion_required = (is_random_motion_required || IsRandomMotionRequired(expr_child));
	}
	return is_random_motion_required;
}

// main driver, post-processing of input plan expression
CExpression *
CExpressionPostProcessor::PostProcessPlan
	(
	IMemoryPool *mp,
	CExpression *expr_plan
	)
{
	GPOS_ASSERT(NULL != expr_plan);

	CExpression *random_motion_processed_plan_expr = AddRandomMotionBeforeDMLInsertOrCTAS(mp, expr_plan);
	GPOS_CHECK_ABORT;

	return random_motion_processed_plan_expr;
}

// EOF
