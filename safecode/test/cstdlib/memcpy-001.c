// RUN: test.sh -e -t %t %s
#include <string.h>

// memcpy() called with too short a destination.

int main()
{
  char src[] = "aaaaaaaaaab";
  char dst[10];
  memcpy(dst, src, 11);
  return 0;
}
