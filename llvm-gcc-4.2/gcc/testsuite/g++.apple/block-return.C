/* APPLE LOCAL file radar 5732232 - blocks */
/* { dg-do compile } */
/* { dg-options "-fblocks" } */

typedef void (^CL)(void);

CL foo() {

	short y;
        short (^add1)(void) = ^{ return y+1; };  /* { dg-error "cannot convert" } */

	CL X = ^{if (2)
                   return;
           	 return 1;   /* { dg-error "void block should not return a value" } */
			     /* { dg-error "return-statement with a value" "" { target *-*-* } 14 } */
	 	};

	int (^Y) (void)  = ^{ if (3)
	     	        return 1;
	   	       else
	     	        return;  /* { dg-error "non-void block should return a value" } */
	             };

	char *(^Z)(void) = ^{ if (3)
             			return "";  /* { dg-error "cannot convert" } */
           		      else
             		       return (char*)0; /* { dg-error "incompatible type returning" } */ 
         		    };	/* { dg-error "cannot convert" } */

        double (^A)(void) = ^ { if (1)
				 return (float)1.0;
				else
				  if (2)
				    return (double)2.0;  /* { dg-error "incompatible type returning" } */
				return 1;  /* { dg-error "incompatible type returning" } */
			      }; /* { dg-error "cannot convert" } */
        char *(^B)(void) =
		^{ if (3)
             	     return ""; /* { dg-error "cannot convert" } */
           	   else
             	    return 2; /* { dg-error "incompatible type returning" } */
         	};	     /* { dg-error "cannot convert" } */

        return ^{ return 1; };  /* { dg-error "cannot convert" } */

}

