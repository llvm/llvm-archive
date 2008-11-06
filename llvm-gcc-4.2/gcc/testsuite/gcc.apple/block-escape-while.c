/* APPLE LOCAL file radar 6083129 byref escapes */
/* { dg-options "-fblocks" } */
/* { dg-do run } */


extern void abort(void);

static int count;
static void _Block_byref_release(void * arg) {
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

