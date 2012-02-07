/* Make sure Objective-C++ can distinguish ObjC classes from C++ classes.  */
/* Author: Ziemowit Laski  <zlaski@apple.com> */

/* { dg-do compile } */

/* APPLE LOCAL radar 4894756 */
#include "../objc/execute/Object2.h"
#include <iostream>
#include <string>

@interface iostream: Object
@end

int main(void) {
  id i = [std::iostream new];  /* { dg-warning "is not an Objective.C class" } */
  i = [iostream new];
  i = [std::basic_string<char> new];  /* { dg-warning "is not an Objective.C class" } */

  return 0;
}
