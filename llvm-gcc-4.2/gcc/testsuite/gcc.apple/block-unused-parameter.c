/* APPLE LOCAL file radar 5925784 */
/* Don't issue warning with -Wunused-parameter on '_self' parameter. */
/* { dg-do compile } */
/* { dg-options "-Wunused-parameter" } */

int main()
{
	int i = 1;
	^ { |i| i = 1; return i; }; /* { dg-warning "has been deprecated in blocks" } */
	return 0;
}

