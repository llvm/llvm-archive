// { dg-options "-w" }

template<typename T> void foo(T); // { dg-error "note" }
 
void bar()
{
  int i;
  int A[i][i]; 
  foo(A); // { dg-error "no matching function for call to" } 
}
