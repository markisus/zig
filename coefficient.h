#ifndef ZIG_COEFFICIENT
#define ZIG_COEFFICIENT

#include <cstddef>

namespace zig {

struct Coefficient {
  Coefficient(size_t c, double v) :
      constraint_id(c), value(v) {}
  size_t constraint_id;
  double value;
};

}

#endif
