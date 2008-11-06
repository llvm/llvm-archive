/* APPLE LOCAL file radar 6083129 byref escapes */
/* { dg-options "-fblocks" } */
/* { dg-do run } */


extern void abort(void);

static int count;
static void _Block_byref_release(void * arg) {
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
