/* APPLE LOCAL file radar 6305545 */
/* { dg-do run } */
/* { dg-options "-mmacosx-version-min=10.5" { target *-*-darwin* } } */

extern "C" void abort(void);

static int count  = 100;
struct CParallelTest
{
	CParallelTest (void);
	~CParallelTest (void) {
	  void (^p1)() = ^ { count -= 3; };
	  p1();
	}
};

CParallelTest::CParallelTest (void)
{
  void (^p1)() = ^ { count = 1; };
  int (^p2)() = ^ { return ++count; };
  int (^p3)() = ^ { return ++count; };
  p1(); p2(); p3();
}

int main()
{
  {
	CParallelTest t1;
  }
  if (count != 0)
    abort();
  return 0;
}
