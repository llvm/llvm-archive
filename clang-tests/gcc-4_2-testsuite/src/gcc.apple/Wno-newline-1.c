/* Test no newline at eof warning.  */
/* { dg-do compile } */
int main() { return 0; } /* { dg-error "no newline at end of file" } */