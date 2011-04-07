// { dg-do assemble  }
// prms-id: 9068

struct ostream {
  void operator<< (int);	// { dg-error "candidate function not viable" }
};

class C {
public:
  static int& i (); // { dg-error "candidate function" }
  static int& i (int signatureDummy); // { dg-error "candidate function" }
};

void foo (ostream& lhs, const C& rhs)
{
  lhs << rhs.i;		// { dg-error "cannot resolve overloaded function" }
}

int& C::i () {
  static int _i = 4711;
  return _i;
}
