// { dg-do assemble  }
// g++ 1.36.1 bug 900119_01

// g++ allows initializers to be included in the declaration of members
// of classes, structs, unions (even when -pedantic is used).

// This is not allowed by the C++ 2.0 Reference Manual or by Cfront 2.0.

// keywords: member declaration, member initialization

int global_int;

class class0 {
public:
  int class0_member_0 = 99;			/* { dg-warning "extension" }  */
  static int class0_member_1 = 99;		/* { dg-error "out of line" }  */
  int &class0_member_2 = global_int;		/* { dg-warning "extension" }  */

  class0 () : class0_member_2 (global_int) { }
};


struct struct0 {
  int struct0_member_0 = 99;			/* { dg-warning "extension" }  */
  static int struct0_member_1 = 99;		/* { dg-error "out of line" }  */
  int &struct0_member_2 = global_int;		/* { dg-warning "extension" }  */

  struct0 () : struct0_member_2 (global_int) { }
};

// g++ does not allow unions to have more than one member with an initializer

union union0 {
  int union0_member_0 = 99;			/* { dg-warning "extension" }  */
};

union union1 {
  //static int union1_member_0 = 99;		/* definitely illegal (9.5) */
};

union union2 {
  int &union2_member_0 /* { dg-error "reference type" } */
    = global_int;      /* { dg-warning "extension" }  */

  union2 () : union2_member_0 (global_int) { }  
};

int main () { return 0; }
