#ifndef ZIG_WORK_BUFFERS
#define ZIG_WORK_BUFFERS

#include <vector>
#include "coefficient.h"

namespace zig {

struct WorkBuffers {
  std::vector<Coefficient> incoming_variable_coefficients;
  std::vector<Coefficient> merge_buffer;
  std::vector<Coefficient> scale_buffer;
};

}

#endif
