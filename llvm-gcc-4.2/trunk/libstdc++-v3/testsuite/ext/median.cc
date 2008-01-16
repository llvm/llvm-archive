// Copyright (C) 2005 Free Software Foundation, Inc.
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

// median - SGI extension

#include <algorithm>
#include <testsuite_hooks.h>

bool pred(const int& l, const int& r)
{
  return l<r;
}

using std::__median;

int main(void)
{
  const int i=1;
  const int j=2;
  const int k=3;
  VERIFY(__median(i, j, k) == j && __median(i, j, k, pred) == j);
  VERIFY(__median(i, k, j) == j && __median(i, k, j, pred) == j);
  VERIFY(__median(j, i, k) == j && __median(j, i, k, pred) == j);
  VERIFY(__median(j, k, i) == j && __median(j, k, i, pred) == j);
  VERIFY(__median(k, i, j) == j && __median(k, i, j, pred) == j);
  VERIFY(__median(k, j, i) == j && __median(k, j, i, pred) == j);
}
