#ifndef ZIG_HELPERS
#define ZIG_HELPERS

#include "column.h"
#include "tableau.h"
#include "work_buffers.h"

namespace zig {

struct PivotSelection {
  size_t outgoing_variable_id;
  size_t active_constraint_id;
  double incoming_variable_active_coefficient;
};

void FixRightHandSide(
    const PivotSelection& pivot_selection,
    const Column& incoming_variable_column,
    std::vector<size_t>* basic_variable_ids,
    std::vector<double>* rhs_values);

// incoming_variable_column points into the nonbasic_variables.
// nonbasic_variables is updated with a primal step, and the
// - incoming_variable_column is overwritten to represent the
//   outgoing variable column.
void FixNonbasicVariableCoefficients(
    const PivotSelection& pivot_selection,
    Column* incoming_variable_column,
    std::vector<Column>* nonbasic_variables,
    WorkBuffers* work_buffers);

size_t SelectIncomingVariable(const Tableau& tableau);

PivotSelection SelectPivot(const Tableau& tableau, 
                           const Column& incoming_variable_column);

void Pivot(Tableau* tableau,
           size_t incoming_variable_id,
           WorkBuffers* work_buffers);
}

#endif
