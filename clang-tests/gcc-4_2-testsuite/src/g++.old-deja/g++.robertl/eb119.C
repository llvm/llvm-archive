// { dg-do assemble  }
template<bool B>
void f()  // { dg-error "" }
{
}

int main()
{
  f<bool>(); // { dg-error "" } .*
}

