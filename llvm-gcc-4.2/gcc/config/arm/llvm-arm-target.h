/* LLVM LOCAL begin (ENTIRE FILE!)  */
#ifdef ENABLE_LLVM
/* Some target-specific hooks for gcc->llvm conversion
Copyright (C) 2009 Free Software Foundation, Inc.
Contributed by Anton Korobeynikov (asl@math.spbu.ru)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

/* LLVM specific stuff for supporting calling convention output */
#define TARGET_ADJUST_LLVM_CC(CC, type)				\
  {								\
    if (TARGET_AAPCS_BASED)					\
      CC = (TARGET_VFP && TARGET_HARD_FLOAT_ABI ?		\
	    CallingConv::ARM_AAPCS_VFP :			\
	    CallingConv::ARM_AAPCS);				\
    else							\
      CC = CallingConv::ARM_APCS;				\
  }

#endif /* ENABLE_LLVM */
/* LLVM LOCAL end (ENTIRE FILE!)  */
