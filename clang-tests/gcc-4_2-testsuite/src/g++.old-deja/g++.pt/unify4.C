// { dg-do assemble  }
template <class T> void f (T); // { dg-warning "" } candidate template ignored

void g ();
void g (int);

int
main ()
{
  f (g);			// { dg-error "" } no matching function
  return 0;
}
