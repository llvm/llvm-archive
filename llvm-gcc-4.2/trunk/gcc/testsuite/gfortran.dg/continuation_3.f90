! { dg-do compile }
! { dg-options -pedantic }
! PR 19262  Test limit on line continuations. Test case derived form case in PR
! by Steve Kargl.  Submitted by Jerry DeLisle  <jvdelisle@gcc.gnu.org>
print *, &
       "1" // & !  1
       "2" // & !  2
       "3" // & !  3
       "4" // & !  4
       "5" // & !  5
       "6" // & !  6
       "7" // & !  7
       "8" // & !  8
       "9" // & !  9
       "0" // & ! 10
       "1" // & ! 11
       "2" // & ! 12
       "3" // & ! 13
       "4" // & ! 14
       "5" // & ! 15
       "6" // & ! 16
       "7" // & ! 17
       "8" // & ! 18
       "9" // & ! 19
       "0" // & ! 20
       "1" // & ! 21
       "2" // & ! 22
       "3" // & ! 23
       "4" // & ! 24
       "5" // & ! 25
       "6" // & ! 26
       "7" // & ! 27
       "8" // & ! 28
       "9" // & ! 29
       "0" // & ! 30
       "1" // & ! 31
       "2" // & ! 32
       "3" // & ! 33
       "4" // & ! 34
       "5" // & ! 35
       "6" // & ! 36
       "7" // & ! 37
       "8" // & ! 38
       "9"
print *, &
       "1" // & !  1
       "2" // & !  2
       "3" // & !  3
       "4" // & !  4
       "5" // & !  5
       "6" // & !  6
       "7" // & !  7
       "8" // & !  8
       "9" // & !  9
       "0" // & ! 10
       "1" // & ! 11
       "2" // & ! 12
       "3" // & ! 13
       "4" // & ! 14
       "5" // & ! 15
       "6" // & ! 16
       "7" // & ! 17
       "8" // & ! 18
       "9" // & ! 19
       "0" // & ! 20
       "1" // & ! 21
       "2" // & ! 22
       "3" // & ! 23
       "4" // & ! 24
       "5" // & ! 25
       "6" // & ! 26
       "7" // & ! 27
       "8" // & ! 28
       "9" // & ! 29
       "0" // & ! 30
       "1" // & ! 31
       "2" // & ! 32
       "3" // & ! 33
       "4" // & ! 34
       "5" // & ! 35
       "6" // & ! 36
       "7" // & ! 37
       "8" // & ! 38
       "9" // & ! 39
       "0"      ! { dg-warning "Limit of 39 continuations exceeded" }

end