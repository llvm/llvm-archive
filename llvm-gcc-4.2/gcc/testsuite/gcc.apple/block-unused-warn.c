/* APPLE LOCAL file radar 5834569 */
/* Do not issue unused variable warning on blocks. */
/* { dg-do compile } */
/* { dg-options "-Wall" } */

int main()
{
int x = 10;
int y = 1;

int (^myBlock)(void) = ^{ |x| return x+y; }; /* { dg-warning "has been deprecated in blocks" } */

myBlock();

return 0;
}

