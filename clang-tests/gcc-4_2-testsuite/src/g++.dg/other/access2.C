// { dg-do compile }
/* { dg-options "-Wno-empty-body" } */
// Origin: Dirk Mueller <dmuell@gmx.net>

// PR c++/2739
// Access to base class private static member.

class Base {
private:
  static int fooprivate;	// { dg-error "note" }
protected:
  static int fooprotected;
public:
  static int foopublic;
};

class Derived : public Base {
public:
  void test();
};

int Base::fooprivate=42;
int Base::fooprotected=42;
int Base::foopublic=42;

void Derived::test() {
  if ( fooprivate );		// { dg-error "'fooprivate' is a private member of 'Base'" }
  if ( fooprotected );
  if ( foopublic );
}

int main()
{
  Derived d;
  d.test();
}
