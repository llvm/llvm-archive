;===-- Runtime Programs Glue -------------------------------------*- LLVM -*-===
;
;                      High Level Virtual Machine (HLVM)
;
; Copyright (C) 2006 Reid Spencer. All Rights Reserved.
;
; This software is free software; you can redistribute it and/or modify it 
; under the terms of the GNU Lesser General Public License as published by 
; the Free Software Foundation; either version 2.1 of the License, or (at 
; your option) any later version.
;
; This software is distributed in the hope that it will be useful, but WITHOUT
; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
; FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for 
; more details.
;
; You should have received a copy of the GNU Lesser General Public License 
; along with this library in the file named LICENSE.txt; if not, write to the 
; Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
; MA 02110-1301 USA
;
;===-------------------------------------------------------------------------===
;
; This file provides some glue between the runtime library and the HLVM compiler
; It arranges to get the address of an "appending linkage" constant array via
; the hlvm_get_programs() function. This is necessary to be done in LLVM 
; assembly because you can't express an "appending linkage" constant array in
; C or C++. Additionally, this provides the null-terminator sentinel that marks
; the end of the array. When the compiler generates code for an HLVM program, it
; creates an entry in this array. Since it is legal to have multiple entry 
; points in HLVM, the resulting array (collected from all Program compilations)
; will contain the complete set of entry points that can be started. This is
; useful where a group of related programs should share the same executable. For
; example, "rm", "cp", "mkdir", "rmdir", etc. all have similar needs and, with
; LLVM, could all be in the same executable. This file makes that magic 
; happen.
; 
;===-------------------------------------------------------------------------===
;
; This is the type for the elements of the hlvm_programs array. Each element
; contains a pointer to a string for the name of the program and a pointer to
; the entry function.
%hlvm_programs_element = type { i8*, i32 (i32, i8**)* }

; This is the appending constant array with 1 element here that is zero
; initialized. This forms the zero-terminator in the array. This module MUST
; be linked LAST in order for this to work.
@hlvm_programs = appending constant [1 x %hlvm_programs_element] zeroinitializer

; End of declarations, start the implementation
implementation

; This is a very simple function to get the address of %hlvm_programs.
define %hlvm_programs_element* @hlvm_get_programs() {
entry:
  ret %hlvm_programs_element* getelementptr([1 x %hlvm_programs_element]* @hlvm_programs, i32 0, i32 0)
}
