// { dg-do assemble  }

void x()
{
 int* p = 1==0; // { dg-warning "" } initialization 
}
