//===-- Runtime Math Functions Interface ------------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Math.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/08/01
/// @since 0.2.0
/// @brief Declares the interface to the runtime math functions
//===----------------------------------------------------------------------===//

#ifndef HLVM_RUNTIME_MATH_H
#define HLVM_RUNTIME_MATH_H

typedef double hlvm_f64;
typedef float hlvm_f32;

extern hlvm_f32 hlvm_f32_ispinf(hlvm_f32);
extern hlvm_f32 hlvm_f32_isninf(hlvm_f32);
extern hlvm_f32 hlvm_f32_isnan(hlvm_f32);
extern hlvm_f32 hlvm_f32_trunc(hlvm_f32);
extern hlvm_f32 hlvm_f32_round(hlvm_f32);
extern hlvm_f32 hlvm_f32_floor(hlvm_f32);
extern hlvm_f32 hlvm_f32_ceiling(hlvm_f32);
extern hlvm_f32 hlvm_f32_loge(hlvm_f32);
extern hlvm_f32 hlvm_f32_log2(hlvm_f32);
extern hlvm_f32 hlvm_f32_log10(hlvm_f32);
extern hlvm_f32 hlvm_f32_squareroot(hlvm_f32);
extern hlvm_f32 hlvm_f32_cuberoot(hlvm_f32);
extern hlvm_f32 hlvm_f32_factorial(hlvm_f32);
extern hlvm_f32 hlvm_f32_power(hlvm_f32, hlvm_f32);
extern hlvm_f32 hlvm_f32_root(hlvm_f32, hlvm_f32);
extern hlvm_f32 hlvm_f32_gcd(hlvm_f32, hlvm_f32);
extern hlvm_f32 hlvm_f32_lcm(hlvm_f32, hlvm_f32);

extern hlvm_f64 hlvm_f64_ispinf(hlvm_f64);
extern hlvm_f64 hlvm_f64_isninf(hlvm_f64);
extern hlvm_f64 hlvm_f64_isnan(hlvm_f64);
extern hlvm_f64 hlvm_f64_trunc(hlvm_f64);
extern hlvm_f64 hlvm_f64_round(hlvm_f64);
extern hlvm_f64 hlvm_f64_floor(hlvm_f64);
extern hlvm_f64 hlvm_f64_ceiling(hlvm_f64);
extern hlvm_f64 hlvm_f64_loge(hlvm_f64);
extern hlvm_f64 hlvm_f64_log2(hlvm_f64);
extern hlvm_f64 hlvm_f64_log10(hlvm_f64);
extern hlvm_f64 hlvm_f64_squareroot(hlvm_f64);
extern hlvm_f64 hlvm_f64_cuberoot(hlvm_f64);
extern hlvm_f64 hlvm_f64_factorial(hlvm_f64);
extern hlvm_f64 hlvm_f64_power(hlvm_f64, hlvm_f64);
extern hlvm_f64 hlvm_f64_root(hlvm_f64, hlvm_f64);
extern hlvm_f64 hlvm_f64_gcd(hlvm_f64, hlvm_f64);
extern hlvm_f64 hlvm_f64_lcm(hlvm_f64, hlvm_f64);
#endif
