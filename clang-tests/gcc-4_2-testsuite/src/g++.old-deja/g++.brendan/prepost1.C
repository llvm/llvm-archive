// { dg-do assemble  }
// GROUPS passed prefix-postfix
class foo {
public:
      operator ++ (); // { dg-error "requires a type specifier for all declarations" }
};

int main()
{
  foo x;

  // This should fall back to calling operator++(), and be an error
  x++;  // { dg-error "cannot increment value of type" } no type or storage class
}
