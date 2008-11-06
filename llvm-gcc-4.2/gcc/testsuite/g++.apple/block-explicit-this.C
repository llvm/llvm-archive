/* APPLE LOCAL file radar 6275956 */
/* Use of explicit "this" inside a block. */
/* { dg-options "-mmacosx-version-min=10.5" { target *-*-darwin* } } */
/* { dg-do run } */
extern "C" void abort (void);

struct S {

	int ifield;

	int mem ()
	{
	    int (^p) (void) = ^ { return this->ifield; }; 
	    if (p() != ifield)
	     abort ();
	    return 0;
        }
	S (int val) { ifield = val; }

};

int main()
{
	S s(123);

	return s.mem();
}

	
