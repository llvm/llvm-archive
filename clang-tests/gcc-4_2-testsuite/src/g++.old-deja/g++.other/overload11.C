// { dg-options "-Wno-unused-value -ferror-limit=0" }

// Copyright (C) 1999 Free Software Foundation, Inc.
// Contributed by Nathan Sidwell 5 Sep 1999 <nathan@acm.org>

// [over.match] 13.3 tells us where overload resolution occurs.
// [over.match.call] 13.3.1.1 says that in
//  (...( postfix-expression )...) (expression-list)
// the postfix-expression must be the name of a function (amongst some other
// choices). This means comma and conditional exprs cannot be placed there.
// This clause is the only one I can find which bans
//  (cond ? fna : fnb) (arglist)
// which would be a major headache to have to implement.
// [over.over] 13.4 tells us when the use of a function name w/o arguments is
// resolved to the address of a particular function. These are determined by
// the context of the function name, and it does allow more complicated primary
// expressions.

// Using a naked function name is rather strange, we used to warn about it
// (rather inconsistently), but subsequent changes broke the warning. Make
// sure that doesn't happen again.


void ovl (int);          // { dg-error "" } candidate
void ovl (float);        // { dg-error "" } candidate
void fn (int);
void fna (int);

int main (int argc, char **argv)
{
  void (*ptr) (int);
  void (*vptr) ();
  
  (ovl) (1);                // ok
  (&ovl) (1);               // ok
  (ovl) ();                 // { dg-error "no matching function" }
  (&ovl) ();                // { dg-error "no matching function" }
  
  // 13.3.1.1 indicates that the following are errors -- the primary expression
  // is not the name of a function.
  (0, ovl) (1);             // { dg-error "reference to overloaded function could not be resolved" }
  (0, &ovl) (1);            // { dg-error "reference to overloaded function could not be resolved" }
  (argc ? ovl : ovl) (1);   // { dg-error "reference to overloaded function could not be resolved" }
  (argc ? &ovl : &ovl) (1); // { dg-error "reference to overloaded function could not be resolved" }
  
  (fn) (1);                 // ok
  (&fn) (1);                // ok (no overload resolution)
  (0, fn) (1);              // ok (no overload resolution)
  (0, &fn) (1);             // ok (no overload resolution)
  (argc ? fna : fn) (1);    // ok (no overload resolution)
  (argc ? &fna : &fn) (1);  // ok (no overload resolution)
  
  ptr = (ovl);              // ok
  ptr = (&ovl);             // ok
  // 13.4 indicates these are ok.
  ptr = (0, ovl);           // { dg-error "reference to overloaded function could not be resolved" }
  ptr = (0, &ovl); // { dg-error "reference to overloaded function could not be resolved" }
  ptr = (argc ? ovl : ovl); // { dg-error "reference to overloaded function could not be resolved" }
  ptr = (argc ? &ovl : &ovl); // { dg-error "reference to overloaded function could not be resolved" }
  
  vptr = (ovl);              // { dg-error "assigning to|from incompatible type" }
  vptr = (&ovl);             // { dg-error "assigning to|from incompatible type" } 
  vptr = (0, ovl);           // { dg-error "could not be resolved" } 
  vptr = (0, &ovl);          // { dg-error "could not be resolved" } 
  vptr = (argc ? ovl : ovl); // { dg-error "could not be resolved" } 
  vptr = (argc ? &ovl : &ovl);// { dg-error "could not be resolved" } 
  
  ptr = (fn);
  ptr = (&fn);
  ptr = (0, fn);
  ptr = (0, &fn);
  ptr = (argc ? fna : fn);
  ptr = (argc ? &fna : &fn);
  
  f;                // { dg-error "use of undeclared identifier" }
  ovl;              // { dg-error "overloaded function could not be resolved" }
  &ovl;             // { dg-error "overloaded function could not be resolved" }
  (void)f;          // { dg-error "use of undeclared identifier" }
  (void)ovl;        // { dg-error "address of overloaded function 'ovl' cannot be cast to type 'void'" }
  (void)&ovl;       // { dg-error "address of overloaded function 'ovl' cannot be cast to type 'void'" }
  static_cast<void>(f);          // { dg-error "use of undeclared identifier 'f'" }
  static_cast<void>(ovl);        // { dg-error "address of overloaded function 'ovl' cannot be static_cast to type 'void'" }
  static_cast<void>(&ovl);       // { dg-error "address of overloaded function 'ovl' cannot be static_cast to type 'void'" }
  ((void)1, f);             // { dg-error "use of undeclared identifier 'f'" }
  ((void)1, ovl);           // { dg-error "overloaded function could not be resolved" }
  ((void)1, &ovl);          // { dg-error "overloaded function could not be resolved" }
  (void)((void)1, f);           // { dg-error "use of undeclared identifier 'f'" }
  (void)((void)1, ovl);         // { dg-error "overloaded function could not be resolved" }
  (void)((void)1, &ovl);        // { dg-error "overloaded function could not be resolved" }

  return 0;
}
