/* APPLE LOCAL file mainline candidate */
/* { dg-do compile } */
static char _my_endbss[1];
char *my_endbss_static = _my_endbss;

/* { dg-final { scan-assembler ".zerofill __DATA,__bss,__my_endbss" } } */
