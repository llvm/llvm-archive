// { dg-do compile }

int main()  // { dg-error "note" }
{
  return 0;
}


int main(int, const char**) // { dg-error "conflicting types" }
{
  return 0;
}
