// { dg-do assemble  }

class A { };

int main()  {
  A a = a; // { dg-warning "" }
  A b(b); // { dg-warning "" }
}
