//===-- Various CPP Macros For Doing Assertions -----------------*- C++ -*-===//
//
//                      High Level Virtual Machine (HLVM)
//
// Copyright (C) 2006 Reid Spencer. All Rights Reserved.
//
// This software is free software; you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published by 
// the Free Software Foundation; either version 2.1 of the License, or (at 
// your option) any later version.
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for 
// more details.
//
// You should have received a copy of the GNU Lesser General Public License 
// along with this library in the file named LICENSE.txt; if not, write to the 
// Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301 USA
//
//===----------------------------------------------------------------------===//
/// @file hlvm/Base/Assert.h
/// @author Reid Spencer <rspencer@reidspencer.com>
/// @date 2006/05/20
/// @since 0.1
/// @brief Assertion and error checking macros.
///
/// This file provides various preprocessor macros that can be used to assist
/// the programmer to check assertions, handle dead code locations, do range
/// checking, etc. These are intended to be used to check class invariants,
/// function arguments, and other points in the logic where an assertion
/// can be made to ensure the program is operating within its parameters.
/// Note that if an assertion macro is triggered, it results in an
/// exception being thrown. Make sure that your code can exit cleanly in
/// the face of the assertion being thrown. Generally this means that
/// you should place all your assertions at the beginning of a function
/// so that you don't attempt to operate on the parameters or objects
/// that are outside of the specified limits. Also note that these macros
/// can be turned off with the HLVM_ASSERT configuration variable. Ensure
/// that no needed side effects are encapsulated in the arguments to these
/// macros as those side effects will disappear when building without the
/// HLVM_ASSERT configuration. Assertions should only be used to check
/// validity of logic that should always be true (such as parameter ranges)
/// but not for end user error messages.
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_BASE_ASSERT_H
#define HLVM_BASE_ASSERT_H

#ifdef HLVM_ASSERT
#include <cassert>
#endif
/// This  macro enforces any expression that the programmer cares to
/// assert at the point in the program where the macro is used. The argument
/// can be any valid logical expression that can be used with the ! operator.
/// Note that the exception thrown will contain the text of the expression so
/// that it will be easier to identify which condition failed. Watch out for
/// side effects of the expression! Do all assignments before the assertion!
/// @param expr Any logic expression to be tested. The macro throws if the expression evaluates to false.
/// @brief Check an arbitrary expression for truth.
#ifdef HLVM_ASSERT
#define hlvmAssert( expr ) assert(expr)
#else
#define hlvmAssert( expr ) {}
#endif

/// The DEAD_CODE macro is intended to be used to signify locations in the 
/// code that should never be reached. It serves as a sentinel in the software 
/// and will \em always throw an exception when it is encountered. Use this to 
/// place some code in the \p default label of a switch statement that should 
/// not be reached or at the end of a function that should normally return 
/// through some other execution path.
/// @param msg Provides a message that will become part of the exception 
/// object thrown.
/// @brief Identify locations in the code that should not be reached.
#ifdef HLVM_ASSERT
#define hlvmDeadCode( msg ) assert(! "Dead Code:" msg)
#else
#define hlvmDeadCode( msg ) {}
#endif

/// This macro can be utilized to state that a function is not implemented. It
/// throws a "Not Implemented" exception that indicates the name of the function
/// that has not been implemented. This can be used as the sole content of any 
/// to ensure that unimplemented functionality is recognized. You should use 
/// this macro in every function when you are doing rapid prototyping.
/// @param msg Provides a message that will become part of the exception 
/// object thrown.
/// @brief Identify unimplemented functionality
#ifdef HLVM_ASSERT
#define hlvmNotImplemented( msg ) assert(! "Not Implemented:" msg)
#else
#define hlvmNotImplemented(msg) {}
#endif

/// This macro allows the programmer to make a range assertion about the 
/// \p value argument when compared with the \p min and \p max arguments. The 
/// \p value can be anything that is compatible with the \c < and \c > 
/// operators.  Note that the range being checked is inclusive of both \p min 
/// and \p max.
/// @param value The value checked against the \p min and \p max parameters.
/// @param min The (inclusive) minimum value to be checked.
/// @param max The (inclusive) maximum value to be checked.
/// @brief Check numerical range of a value.
#ifdef HLVM_ASSERT
#define hlvmRangeCheck( value, min, max ) \
  assert(((value)>=(min) || (value)<=(max)) && "Range Error!")
#else
#define hlvmRangeCheck(value,min,max) {}
#endif

#endif
