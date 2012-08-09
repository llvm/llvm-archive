// { dg-options "-Wno-non-literal-null-conversion" }

// PR c++/16489

const int NULL = 0;
int main() { 
  double* p = NULL; 
}
