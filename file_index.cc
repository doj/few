#include "file_index.h"
#include <stdexcept>

file_index::file_index(const file_t& f) :
  file_(f)
{
  if (f.empty()) {
    return;
  }

  // we start counting lines with number 1. So add an invalid pointer to index 0.
  line_.push_back(nullptr);

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

/// \todo remove this function once we have a better compiler!
namespace std {
  std::string to_string(const unsigned u)
  {
    return "std::to_string() not implemented!";
  }
}

line_t file_index::line(const unsigned idx)
{
  if (idx > lines()) {
    throw std::runtime_error("file_index::line(" + std::to_string(idx) + "): index too large, " + std::to_string(lines()));
  }
  if (idx == lines() - 1) {
    return line_t(line_[idx], file_.end());
  }
  return line_t(line_[idx], line_[idx + 1]);
}
