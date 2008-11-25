/* APPLE LOCAL file radar 6083129 byref escapes */
/* { dg-options "-fblocks" } */
/* { dg-do run } */


extern void abort(void);

static int count;
static void _Block_object_dispose(void * arg, int flag) {
	++count;
}

int main()
{
  {
    __block int O1;
    int i = 0;
    while (++i != 5)
    {
            __block int I1;
    }
    if (count != 4)
	abort();
  }
    return count - 5;
}

