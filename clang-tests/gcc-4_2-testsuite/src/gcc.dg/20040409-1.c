/* This tests to make sure that the attribute noreturn 
   can be used on function pointers. */

void (*temp) (void) __attribute__((noreturn));
