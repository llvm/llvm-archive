// RUN: test.sh -e -t %t %s
// XFAIL: darwin
#include <string.h>

// memcpy() called with too short a source.

int main(int argc)
{
  char src[] = "aaaaaaa";
  char dst[100];
  memcpy(dst, src, 11);
  return dst[argc];
}
