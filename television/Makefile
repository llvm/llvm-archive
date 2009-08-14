#===- television/Makefile ----------------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = .

# Directories that need to be built.
DIRS = lib tools

#
# This is needed since the tags generation code expects a tools directory
# to exist.
#
#all::
#	mkdir -p tools

#
# Include the Master Makefile that knows how to build all.
#
include $(LEVEL)/Makefile.common

distclean:: clean
	${RM} -f Makefile.common Makefile.config config.log config.status
