// PR c++/16489

const int NULL = 0;
int main() { 
  double* p = NULL; // { dg-warning "expression which evaluates to zero treated as a null pointer constant" }
}
