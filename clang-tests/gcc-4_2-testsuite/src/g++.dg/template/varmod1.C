// { dg-options "-w" }

template<typename T> void foo(T); // { dg-error "note" }
 
void bar()
{
  int i;
  int A[i][i]; 
  foo(A); // { dg-error "cannot initialize a parameter of type" } 
}
