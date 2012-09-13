// { dg-do assemble }

struct test_box
    {
     void print(void);
    };

void test<class BOX> (test_box *);   // { dg-error "" } illegal code

class test_square
    {
      friend void test<class BOX> (test_box *); // { dg-error "" } does not match
    }  // { dg-error "" } semicolon missing



template <class BOX> void test(BOX *the_box)   // { dg-error "" }
    {x   // { dg-error "" }
    the_box->print();   // { dg-error "" }
    };   // { dg-error "" }

template void test<> (test_box *); // { dg-error "" }
