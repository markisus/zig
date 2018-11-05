#ifndef ZIG_TABLEAU
#define ZIG_TABLEAU

#include <vector>
#include "column.h"

namespace zig {

struct Tableau {
  double objective;
  std::vector<Column> nonbasic_variables;

  // rhs_values and basic_variable_ids
  // are in tandem.
  std::vector<double> rhs_values;
  std::vector<size_t> basic_variable_ids;
};

}

#endif
