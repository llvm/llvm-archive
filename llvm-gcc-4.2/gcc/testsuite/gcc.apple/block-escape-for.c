/* APPLE LOCAL file radar 6083129 byref escapes */
/* { dg-options "-fblocks" } */
/* { dg-do run } */


extern void abort(void);

static int count;
static void _Block_object_dispose(void * arg, int flag) {
  ++count;
}

int main() {
  {
    __block int O1;
    int i;
    for (i = 1; i <= 5; i++)
      {
	__block int I1;
      }
    if (count != 5)
      abort();
  }
  return count - 6;
}
