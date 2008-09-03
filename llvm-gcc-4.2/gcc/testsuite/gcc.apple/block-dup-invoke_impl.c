/* APPLE LOCAL file radar 5939894 */
/* Check that if a definition of struct invoke_impl already exist, then use it. */
/* { dg-options "-fblocks" } */
/* { dg-do compile } */

struct invoke_impl {
 void   *isa;
 int Flags;
 int Size;
 void *FuncPtr;
};

enum numbers
{
    zero, one, two, three, four
};

typedef enum numbers (^myblock)(enum numbers);


double test(myblock I) {
  return I(three);
}

int main() {
  enum numbers x = one;
  enum numbers y = two;

  myblock CL = ^(enum numbers z)
		{|y| y = z; /* { dg-warning "has been deprecated in blocks" } */
		 test (
		 ^ (enum numbers z) { |x| /* { dg-warning "has been deprecated in blocks" } */
		   x = z;
		   return (enum numbers) four;
		  }
		  );
		  return x;
		};

  enum numbers res = test(CL);

  return 0;
}
