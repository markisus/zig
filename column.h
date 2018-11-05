#ifndef ZIG_COLUMN
#define ZIG_COLUMN

#include <vector>
#include "coefficient.h"

namespace zig {

struct Column {
  size_t variable_id;
  std::vector<Coefficient> coefficients;
  double objective_coefficient;
};

}

#endif
