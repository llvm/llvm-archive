/* APPLE LOCAL file radar 5732232 - blocks */
/* Testing byref syntax checking. */
/* { dg-do compile } */
/* { dg-options "-fblocks" } */

int main() {
	int x, y;
	^{ };
	^{|x|}; /* { dg-warning "has been deprecated in blocks" } */

	^{ |x,y| int r; return x+y+r; }; /* { dg-warning "has been deprecated in blocks" } */
	^{ |x,y|; int r; return x+y+r; }; /* { dg-warning "has been deprecated in blocks" } */

	^{ |x,y|; int r; return x+y+r; /* { dg-warning "has been deprecated in blocks" } */
           ^{
	   |x,y|; int r; return x+y; /* { dg-warning "has been deprecated in blocks" } */
	    };
         };

	^{ |x,y|; int r; return x+y+r; /* { dg-warning "has been deprecated in blocks" } */
           ^{
              { |x,y|; int r; return x+y; }; /* { dg-error "expected expression before" } */
            };
         };

	^ { int r;
	    |x, y|    /* { dg-error "expected expression before" } */
	  };

	^{| /* { dg-warning "has been deprecated in blocks" } */
	   main()|};  /* { dg-error "only a visible variable may be used in a block byref declaration" } */

	/* Assigning to byref variables. */
	^{|x| x = 1;}; /* { dg-warning "has been deprecated in blocks" } */

	^{ |x,y| /* { dg-warning "has been deprecated in blocks" } */
	   if (x != y)
	     x = y = 100;
	 };



}

