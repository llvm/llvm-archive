// { dg-do compile  }
// prms-id: 7180

class String {
public:
   String(const char*); // { dg-error "note" }
   ~String();
};
 
String::String(const char* str = "") { // { dg-error "addition of default argument on redeclaration" }
}
 
String::~String(void) {
}
 
int main() {
   const String array[] = {"3"};
}
