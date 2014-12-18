#include "file_index.h"

file_index::file_index(const file_t& f) :
  file_(f)
{
  if (f.empty()) {
    return;
  }

  bool push_to_lines = true;
  for(file_t::const_iterator it = f.begin(); it != f.end(); ++it) {
    if (push_to_lines) {
      line_.push_back(it);
      push_to_lines = false;
    }
    if (*it == '\n') {
      push_to_lines = true;
    }
  }

}
