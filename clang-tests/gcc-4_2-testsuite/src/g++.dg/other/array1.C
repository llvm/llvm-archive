// Test typeid of multidimensional array with no bounds.
// { dg-do compile }

#include <typeinfo>

int main()
{
    const char *s = typeid(double[][]).name(); // { dg-error "array has incomplete element type" }
    return 0;
}
