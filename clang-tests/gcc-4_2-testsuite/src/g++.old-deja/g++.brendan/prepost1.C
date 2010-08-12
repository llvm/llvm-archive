// { dg-do assemble  }
// GROUPS passed prefix-postfix
class foo {
public:
      operator ++ (); // { dg-error "" } no type or storage class
};

int main()
{
  foo x;

  // This should fall back to calling operator++(), and be an error with
  // the -pedantic flag.
  x++;  // clang no longer issues 2ndry error because class foo is marked invalid. 
}
