// Test for syntax support of various attribute permutations.

void
__attribute__((noreturn))
__attribute__((unused))
one(void); // OK 

__attribute__((noreturn))
__attribute__((unused))
void
two(void); // OK

void
__attribute__((unused))
three (void)
__attribute__((noreturn)); // OK

__attribute__((unused))
void
four (void)
__attribute__((noreturn)); // OK

void
five(void)
__attribute__((noreturn))
__attribute__((unused));  // OK

__attribute__((noreturn))
void
__attribute__((unused)) // parse error before '__attribute__' in C++
six (void);              // OK in C
