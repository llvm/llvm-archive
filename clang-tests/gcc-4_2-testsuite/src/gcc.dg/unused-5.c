/* { dg-do compile } */
/* { dg-options "-Wunused" } */
/* { dg-final { scan-assembler "string_to_look_for" } } */

/* 'volatile' variables get output and don't produce a warning about being
   unused.  */
static volatile char string[]  __attribute__((used))
  = "string_to_look_for"; 
