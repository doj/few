/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include <cstdio>
static int
debugToFile(const char *filename, const char *fmt, ...)
{
  FILE *f = fopen(filename, "at");
  if (! f)
    return -1;
  va_list ap;
  va_start(ap, fmt);
  vfprintf(f, fmt, ap);
  va_end(ap);
  fclose(f);
  return 0;
}
