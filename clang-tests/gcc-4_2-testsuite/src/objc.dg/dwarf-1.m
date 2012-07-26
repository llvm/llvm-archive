/* { dg-options "-gdwarf-2 -dA" } */
/* { dg-final { scan-assembler "\"id.0\".*DW_AT_name" } } */
/* { dg-skip-if "No Dwarf" { { *-*-aix* hppa*-*-hpux* *-*-solaris2.[56]* } && { ! hppa*64*-*-* } } { "*" } { "" } } */
__attribute__((objc_root_class)) @interface foo
  id x;
@end
