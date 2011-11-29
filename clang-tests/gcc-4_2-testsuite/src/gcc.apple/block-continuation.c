/* APPLE LOCAL file radar 5732232 - blocks */
/* { dg-do compile } */
/* { dg-options "-fblocks" } */

void takeblock(void (^f0)());

int main() {
while (1) {
          takeblock(^{
            break;   /* { dg-error "'break' statement not in loop or switch" } */
            while(1) break;  /* ok */
	    goto label1; /* { dg-error "undeclared label 'label1'" } */
          });
	  label1:
          break; /* OK */
	  if (1)
	    continue; /* OK */
        }

  void (^vcl)(void) =
	^{
		break; /* { dg-error "'break' statement not in loop or switch" } */

		while (1) {
		  void (^vcl1) (void) = ^{};

			break;
		}
	};

  void (^VCL)(void) =
	^{
		while (1) {
	          int i;
		  void (^vcl1) (void) = ^{ continue; };/* { dg-error "'continue' statement not in loop" } */
			break;
		  for (i = 0; i < 100; i++)
		   if (i == 10)
		    break;
		}
	};
    goto label1;
}
