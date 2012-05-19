/* APPLE LOCAL file 10.5 debug mode 6621704 */
/* { dg-do run { target *-*-darwin* } } */

#define _GLIBCXX_DEBUG 1

#include <string>
#include <map>
#include <iostream>

std::string a;
std::map<int, int> m;

using namespace std;

string a1;
map<int, int> m1;

int main(int argc, char *argv[]) {
  m[1]=2;
  a += "World\n";
  m1[1]=2;
  a1 += "hi";
  cout << "Hello " << a;
}
