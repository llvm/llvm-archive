-- E28002B.ADA

--                             Grant of Unlimited Rights
--
--     Under contracts F33600-87-D-0337, F33600-84-D-0280, MDA903-79-C-0687,
--     F08630-91-C-0015, and DCA100-97-D-0025, the U.S. Government obtained 
--     unlimited rights in the software and documentation contained herein.
--     Unlimited rights are defined in DFAR 252.227-7013(a)(19).  By making 
--     this public release, the Government intends to confer upon all 
--     recipients unlimited rights  equal to those held by the Government.  
--     These rights include rights to use, duplicate, release or disclose the 
--     released technical data and computer software in whole or in part, in 
--     any manner and for any purpose whatsoever, and to have or permit others 
--     to do so.
--
--                                    DISCLAIMER
--
--     ALL MATERIALS OR INFORMATION HEREIN RELEASED, MADE AVAILABLE OR
--     DISCLOSED ARE AS IS.  THE GOVERNMENT MAKES NO EXPRESS OR IMPLIED 
--     WARRANTY AS TO ANY MATTER WHATSOEVER, INCLUDING THE CONDITIONS OF THE
--     SOFTWARE, DOCUMENTATION OR OTHER INFORMATION RELEASED, MADE AVAILABLE 
--     OR DISCLOSED, OR THE OWNERSHIP, MERCHANTABILITY, OR FITNESS FOR A
--     PARTICULAR PURPOSE OF SAID MATERIAL.
--*
-- OBJECTIVE:
--     CHECK THAT A PREDEFINED OR AN UNRECOGNIZED PRAGMA MAY HAVE
--     ARGUMENTS INVOLVING OVERLOADED IDENTIFIERS WITHOUT ENOUGH
--     CONTEXTUAL INFORMATION TO RESOLVE THE OVERLOADING.

-- PASS/FAIL CRITERIA:
--     THIS TEST IS PASSED IF IT REPORTS "TENTATIVELY PASSED" AND
--     THE STARRED COMMENT DOES NOT APPEAR IN THE LISTING.

--     AN IMPLEMENTATION FAILS THIS TEST IF THE STARRED COMMENT
--     LINE APPEARS IN THE COMPILATION LISTING.

-- HISTORY:
--     TBN 02/24/86  CREATED ORIGINAL TEST.
--     JET 01/13/88  ADDED CALLS TO SPECIAL_ACTION AND UPDATED HEADER.
--     EDS 10/28/97  ADDED DECLARATIONS FOR PROCEDURES XYZ.

WITH REPORT, SYSTEM; USE REPORT, SYSTEM;
PROCEDURE E28002B IS

     FUNCTION OFF RETURN INTEGER IS
     BEGIN
          RETURN 1;
     END OFF;

     FUNCTION OFF RETURN BOOLEAN IS
     BEGIN
          RETURN TRUE;
     END OFF;

     PRAGMA LIST (OFF);
--***** THIS LINE MUST NOT APPEAR IN COMPILATION LISTING.
     PRAGMA LIST (ON);

     FUNCTION ELABORATION_CHECK RETURN INTEGER IS
     BEGIN
          RETURN 1;
     END ELABORATION_CHECK;

     FUNCTION ELABORATION_CHECK RETURN BOOLEAN IS
     BEGIN
          RETURN TRUE;
     END ELABORATION_CHECK;

     PRAGMA SUPPRESS (ELABORATION_CHECK, ELABORATION_CHECK);

     FUNCTION TIME RETURN INTEGER IS
     BEGIN
          RETURN 1;
     END TIME;

     FUNCTION TIME RETURN BOOLEAN IS
     BEGIN
          RETURN TRUE;
     END TIME;

     PRAGMA OPTIMIZE (TIME);

     PROCEDURE XYZ;
     PROCEDURE XYZ (COUNT : INTEGER);

     PRAGMA INLINE (XYZ);
     PRAGMA PHIL_BRASHEAR (XYZ);

     PROCEDURE XYZ IS
     BEGIN
          NULL;
     END XYZ;

     PROCEDURE XYZ (COUNT : INTEGER) IS
     BEGIN
          NULL;
     END XYZ;

BEGIN
     TEST ("E28002B", "CHECK THAT A PREDEFINED OR AN UNRECOGNIZED " &
                      "PRAGMA MAY HAVE ARGUMENTS INVOLVING " &
                      "OVERLOADED IDENTIFIERS WITHOUT ENOUGH " &
                      "CONTEXTUAL INFORMATION TO RESOLVE THE " &
                      "OVERLOADING");

     SPECIAL_ACTION ("CHECK THAT THE COMPILATION LISTING DOES NOT " &
                     "SHOW THE STARRED COMMENT LINE");

     RESULT;

END E28002B;
