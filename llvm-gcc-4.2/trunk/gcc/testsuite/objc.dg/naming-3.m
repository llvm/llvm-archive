/* APPLE LOCAL begin radar 4281748 */
/* Test for class name same as an unrelated struct field name. */
/* { dg-do compile } */
@interface PassThrough {

}
@end

struct S {
	int (*PassThrough)();
};

int main()
{
	PassThrough* pt;
	struct S s;
	s.PassThrough();
}
/* APPLE LOCAL end radar 4281748 */
