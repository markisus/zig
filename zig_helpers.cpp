#include <algorithm>
#include <iostream>
#include <limits>
#include <tuple>

#include "zig_helpers.h"
#include "null_variable_id.h"

namespace zig {

void CHECK(bool condition, const char* description) {
  if (!condition) {
    std::cout << "Check failure: " << description << std::endl;
  }
}

bool IsZero(double value) {
  constexpr double kTolerance = 1e-6;
  return std::abs(value) < kTolerance;
}

PivotSelection SelectPivot(const Tableau& tableau, 
                           const Column& incoming_variable_column) {
  // Do the minimum ratio test to find out which variable
  // should leave the basis.
  size_t outgoing_variable_id = NULL_VARIABLE_ID;
  double min_ratio_numerator = std::numeric_limits<double>::infinity();
  double min_ratio_denominator = 1.0;
  size_t active_constraint_id;
  double incoming_variable_active_coefficient;
  for (const auto& coefficient : incoming_variable_column.coefficients) {
    const size_t constraint_id = coefficient.constraint_id;
    const double coefficient_value = coefficient.value;
    if (coefficient_value < 0) {
      continue;
    }
    const double numerator = tableau.rhs_values[constraint_id];
    if (numerator == 0) {
      outgoing_variable_id = tableau.basic_variable_ids[constraint_id];
      active_constraint_id = constraint_id;
      break;
    }
    const double denominator = coefficient_value;
    // numerator/denominator < min_ratio_numerator/min_ratio_denominator
    // <=>
    // numerator*min_ratio_denonimnator < min_ratio_numerator*denominator
    if (numerator * min_ratio_denominator < 
        min_ratio_numerator * denominator) {
      // Ratio test passed. This is the new active constraint.
      active_constraint_id = constraint_id;
      min_ratio_numerator = numerator;
      min_ratio_denominator = denominator;
      incoming_variable_active_coefficient =
          coefficient_value;
      outgoing_variable_id = tableau.basic_variable_ids[constraint_id];
    }
  }
  PivotSelection result;
  result.outgoing_variable_id = outgoing_variable_id;
  result.active_constraint_id = active_constraint_id;
  result.incoming_variable_active_coefficient =
      incoming_variable_active_coefficient;
  return result;
}

void FixNonbasicVariableCoefficients(
    const PivotSelection& pivot_selection,
    Column* incoming_variable_column,
    std::vector<Column>* nonbasic_variables,
    WorkBuffers* work_buffers) {
  const size_t active_constraint_id =
      pivot_selection.active_constraint_id;
  const double incoming_variable_active_coefficient =
      pivot_selection.incoming_variable_active_coefficient;
  const double incoming_variable_objective_coefficient =
      incoming_variable_column->objective_coefficient;
  const size_t outgoing_variable_id =
      pivot_selection.outgoing_variable_id;
  // Copy the incoming variable to the work column.
  work_buffers->incoming_variable_coefficients =
      incoming_variable_column->coefficients;
  // Reuse the memory of the incoming variable to
  // store the outgoing variable.
  Column& outgoing_variable = *incoming_variable_column;
  outgoing_variable.variable_id = outgoing_variable_id;
  outgoing_variable.objective_coefficient = 0;
  outgoing_variable.coefficients.clear();
  outgoing_variable.coefficients.push_back(
      Coefficient(active_constraint_id, 1.0));
  for (auto& current_column : *nonbasic_variables) {
    // Find the coefficient of this variable in
    // the active constraint.
    // todo?: try binary search
    const auto coefficient_it = std::find_if(
        current_column.coefficients.begin(),
        current_column.coefficients.end(),
        [active_constraint_id](
            const Coefficient& coefficient) {
          return coefficient.constraint_id == active_constraint_id;
        });
    if (coefficient_it == current_column.coefficients.end()) {
      // This column does not appear in the active constraint.
      // It does not need to be updated.
      continue;
    }
    const double coefficient_in_active_constraint =
        coefficient_it->value;
    work_buffers->scale_buffer =
        work_buffers->incoming_variable_coefficients;
    for (auto& coefficient : work_buffers->scale_buffer) {
      coefficient.value *= 
          -coefficient_in_active_constraint /
          incoming_variable_active_coefficient;
    }
    // Merge the scale_buffer and the coefficients
    // into the merge_buffer.
    auto scale_buffer_it = work_buffers->scale_buffer.begin();
    auto current_coefficient_it = current_column.coefficients.begin();
    while (scale_buffer_it != work_buffers->scale_buffer.end() &&
           current_coefficient_it != current_column.coefficients.end()) {
      if (scale_buffer_it->constraint_id <
          current_coefficient_it->constraint_id) {
        work_buffers->merge_buffer.push_back(*scale_buffer_it);
        ++scale_buffer_it;
      } else if (scale_buffer_it->constraint_id >
                 current_coefficient_it->constraint_id) {
        work_buffers->merge_buffer.push_back(*current_coefficient_it);
        ++current_coefficient_it;
      } else {
        if (scale_buffer_it->constraint_id == active_constraint_id) {
          // This is the active constraint row.
          // Ignore both work buffers.
          work_buffers->merge_buffer.push_back(
              Coefficient(
                  active_constraint_id,
                  coefficient_in_active_constraint /
                  incoming_variable_active_coefficient));
        } else {
          // Merge the coefficient values by summing.
          scale_buffer_it->value += current_coefficient_it->value;
          if (!IsZero(scale_buffer_it->value)) {
            work_buffers->merge_buffer.push_back(*scale_buffer_it);
          }
        }
        ++scale_buffer_it;
        ++current_coefficient_it;
      }
    }
    std::swap(current_column.coefficients, work_buffers->merge_buffer);
    work_buffers->merge_buffer.clear();
    work_buffers->scale_buffer.clear();
    // Fix objective_coefficient.
    current_column.objective_coefficient -= 
        coefficient_in_active_constraint *
        incoming_variable_objective_coefficient /
        incoming_variable_active_coefficient;
  }
}

void FixRightHandSide(
    const PivotSelection& pivot_selection,
    const Column& incoming_variable_column,
    std::vector<size_t>* basic_variable_ids,
    std::vector<double>* rhs_values) {
  // Fix right hand side values.
  (*basic_variable_ids)[pivot_selection.active_constraint_id] =
      incoming_variable_column.variable_id;
  const double active_rhs_value =
      (*rhs_values)[pivot_selection.active_constraint_id];
  for (const auto& coefficient : incoming_variable_column.coefficients) {
    if (coefficient.constraint_id ==
        pivot_selection.active_constraint_id) {
      (*rhs_values)[pivot_selection.active_constraint_id] /=
          pivot_selection.incoming_variable_active_coefficient;
      continue;
    } else {
      (*rhs_values)[coefficient.constraint_id] -=
          active_rhs_value * coefficient.value / 
          pivot_selection.incoming_variable_active_coefficient;
    }
  }
}

size_t SelectIncomingVariable(const Tableau& tableau) {
  size_t incoming_variable_id = NULL_VARIABLE_ID;
  if (tableau.objective == std::numeric_limits<double>::infinity()) {
    // Optimal already reached.
    return incoming_variable_id;
  }
  double best_objective_coefficient =
      -std::numeric_limits<double>::infinity();
  for (const auto& column : tableau.nonbasic_variables) {
    if(std::make_tuple(-column.objective_coefficient, column.variable_id) <
       std::make_tuple(-best_objective_coefficient, incoming_variable_id)) {
      incoming_variable_id = column.variable_id;
      best_objective_coefficient = column.objective_coefficient;
    }
  }
  if (best_objective_coefficient > 0) {
    return incoming_variable_id;
  } else {
    return NULL_VARIABLE_ID;
  }
}

void Pivot(Tableau* tableau,
           size_t incoming_variable_id,
           WorkBuffers* work_buffers) {
  // If user has not passed in a work_buffers, we have to make one.
  WorkBuffers temp;
  if (work_buffers == nullptr) {
    work_buffers = &temp;
  }
  auto incoming_variable_it =
      std::find_if(tableau->nonbasic_variables.begin(),
                   tableau->nonbasic_variables.end(),
                   [incoming_variable_id](const Column& column){
                     return column.variable_id == incoming_variable_id;
                   });
  CHECK(incoming_variable_it != tableau->nonbasic_variables.end(),
        "incoming variables not found in nonbasic variables");
  const PivotSelection& pivot_selection =
      SelectPivot(*tableau, *incoming_variable_it);
  if (pivot_selection.outgoing_variable_id == NULL_VARIABLE_ID) {
    tableau->objective = std::numeric_limits<double>::infinity();
    return;
  }
  // Fix objective value
  tableau->objective +=
      incoming_variable_it->objective_coefficient *
      tableau->rhs_values[pivot_selection.active_constraint_id] /
      pivot_selection.incoming_variable_active_coefficient;
  FixRightHandSide(
      pivot_selection,
      *incoming_variable_it,
      &(tableau->basic_variable_ids),
      &(tableau->rhs_values));
  FixNonbasicVariableCoefficients(
      pivot_selection,
      &(*incoming_variable_it), // conversion to pointer
      &(tableau->nonbasic_variables),
      work_buffers);
}

}
