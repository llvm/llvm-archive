// { dg-do assemble  }
// Warn if a enum cannot fit into a small bit-field.

enum TypeKind { ATK, BTK, CTK, DTK } ;

struct Type {
  enum TypeKind kind : 1;
  void setBTK();
};

void Type::setBTK() { kind = DTK; } // { dg-warning "implicit truncation from" }
