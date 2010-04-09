// Copyright (C) 2004, 2005, 2006 Free Software Foundation
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// 27.6.1.2.3 basic_istream::operator>>

#include <istream>
#include <sstream>
#include <testsuite_hooks.h>

// stringbufs.
void test01() 
{
  typedef std::wios::traits_type ctraits_type;

  bool test __attribute__((unused)) = true;
  const std::wstring str_01;
  const std::wstring str_02(L"art taylor kickin it on DAKAR");
  std::wstring strtmp;

  std::wstringbuf isbuf_00(std::ios_base::in);
  std::wstringbuf isbuf_01(std::ios_base::in | std::ios_base::out);
  std::wstringbuf isbuf_02(str_01, std::ios_base::in);
  std::wstringbuf isbuf_03(str_01, std::ios_base::in | std::ios_base::out);
  std::wstringbuf isbuf_04(str_02, std::ios_base::in);
  std::wstringbuf isbuf_05(str_02, std::ios_base::in | std::ios_base::out);

  std::wistream is_00(NULL);
  std::wistream is_01(&isbuf_01);
  std::wistream is_02(&isbuf_02);
  std::wistream is_03(&isbuf_03);
  std::wistream is_04(&isbuf_04);
  std::wistream is_05(&isbuf_05);
  std::ios_base::iostate state1, state2, statefail, stateeof;
  statefail = std::ios_base::failbit;
  stateeof = std::ios_base::eofbit;


  // template<_CharT, _Traits>
  //  basic_istream& operator>>(basic_streambuf*)

  // null istream to empty in_buf
  state1 = is_00.rdstate();
  is_00 >> &isbuf_00;   
  state2 = is_00.rdstate();
  VERIFY( state1 != state2 );
  VERIFY( static_cast<bool>(state2 & statefail) );
  VERIFY( isbuf_00.str() == str_01 ); 

  // null istream to empty in_out_buf
  is_00.clear(std::ios_base::goodbit);
  state1 = is_00.rdstate();
  is_00 >> &isbuf_01;   
  state2 = is_00.rdstate();
  VERIFY( state1 != state2 );
  VERIFY( static_cast<bool>(state2 & statefail) );
  VERIFY( isbuf_01.str() == str_01 ); 

  // null istream to full in_buf
  is_00.clear(std::ios_base::goodbit);
  state1 = is_00.rdstate();
  is_00 >> &isbuf_04;   
  state2 = is_00.rdstate();
  VERIFY( state1 != state2 );
  VERIFY( static_cast<bool>(state2 & statefail) );
  VERIFY( isbuf_04.str() == str_02 ); 

  // null istream to full in_out_buf
  is_00.clear(std::ios_base::goodbit);
  state1 = is_00.rdstate();
  is_00 >> &isbuf_05;   
  state2 = is_00.rdstate();
  VERIFY( state1 != state2 );
  VERIFY( static_cast<bool>(state2 & statefail) );
  VERIFY( isbuf_05.str() == str_02 ); 

  // empty but non-null istream to full in_buf
  state1 = is_02.rdstate();
  is_02 >> &isbuf_04;   
  state2 = is_02.rdstate();
  VERIFY( state1 != state2 );
  VERIFY( static_cast<bool>(state2 & statefail) );
  VERIFY( isbuf_04.str() == str_02 ); // as only an "in" buffer
  VERIFY( isbuf_04.sgetc() == L'a' );

  // empty but non-null istream to full in_out_buf
  is_02.clear(std::ios_base::goodbit);
  state1 = is_02.rdstate();
  is_02 >> &isbuf_05;   
  state2 = is_02.rdstate();
  VERIFY( state1 != state2 );
  VERIFY( static_cast<bool>(state2 & statefail) );
  VERIFY( isbuf_05.str() == str_02 ); // as only an "in" buffer
  VERIFY( isbuf_05.sgetc() == L'a' );

  // full istream to empty in_buf (need out_buf, you know?)
  state1 = is_04.rdstate();
  is_04 >> &isbuf_02;   
  state2 = is_04.rdstate();
  VERIFY( state1 != state2 );
  VERIFY( static_cast<bool>(state2 & statefail) );
  VERIFY( isbuf_02.str() == str_01 ); // as only an "in" buffer
  VERIFY( isbuf_02.sgetc() == ctraits_type::eof() );
  VERIFY( is_04.peek() == ctraits_type::eof() ); // as failed

  // full istream to empty in_out_buf
  is_04.clear(std::ios_base::goodbit);
  state1 = is_04.rdstate();
  is_04 >> &isbuf_03;   
  state2 = is_04.rdstate();
  VERIFY( state1 != state2 );
  VERIFY( !static_cast<bool>(state2 & statefail) );
  VERIFY( state2 == stateeof );
  strtmp = isbuf_03.str();
  VERIFY( strtmp == str_02 ); // as only an "in" buffer
  VERIFY( isbuf_03.sgetc() == L'a' );
  VERIFY( is_04.peek() == ctraits_type::eof() );
}

int main()
{
  test01();
  return 0;
}
