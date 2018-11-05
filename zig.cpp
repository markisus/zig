#include <iostream>

#include "work_buffers.h"
#include "zig.h"
#include "zig_helpers.h"

namespace zig {

void Maximize(Tableau* tableau) {
  // Init the work buffers.
  WorkBuffers work_buffers;
  size_t incoming_variable_id =
      SelectIncomingVariable(*tableau);
  while (incoming_variable_id != NULL_VARIABLE_ID) {
    Pivot(tableau, incoming_variable_id, &work_buffers);
    incoming_variable_id = SelectIncomingVariable(*tableau);
  }
}

}
