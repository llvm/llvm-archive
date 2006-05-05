# Copyright (C) 2006 HLVM Group. All Rights Reserved.
# 
# This program is open source software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License (GPL) as published by 
# the Free Software Foundation. You should have received a copy of the GPL
# in a file named COPYING that was included with this file.
#
################################################################################

LEVEL := ..

EXTRA_DIST := Doxyfile.in doxygen.css doxygen.footer doxygen.header \
	      doxygen.intro www

include $(LEVEL)/Makefile.xps

DOCS_TAR_FILE := $(PROJ_OBJ_DIR)/xps.www.tar
DOCS_TGZ_FILE := $(PROJ_OBJ_DIR)/xps.www.tar.gz

.PHONY: doxygen

doxygen:: $(DOCS_TGZ_FILE)

DOXYGEN_DEPS = \
	Doxyfile \
	www/apis/html/.dir \
	$(PROJ_SRC_DIR)/doxygen.css \
	$(PROJ_SRC_DIR)/doxygen.header \
	$(PROJ_SRC_DIR)/doxygen.footer \
	$(PROJ_SRC_DIR)/doxygen.intro \
	$(PROJ_SRC_ROOT)/xps/base/Base.h \
	$(PROJ_SRC_ROOT)/xps/xml/Handler.h \
	$(PROJ_SRC_ROOT)/xps/xpl/Handler.h \
	$(PROJ_SRC_ROOT)/xps/xplbe/XPLTargetMachine.h 

www/apis/html/index.html : $(DOXYGEN_DEPS)
	$(Echo) Running Doxygen (be patient)
	$(Verb) cp $(PROJ_SRC_DIR)/doxygen.css doxygen.css
	$(Verb) doxygen Doxyfile >doxygen.out 2>&1
	$(Verb) cp $(PROJ_SRC_DIR)/doxygen.css www/apis/html

DOCS_TAR_DEPS = \
	www/apis/html/index.html \
	$(wildcard $(PROJ_SRC_DIR)/www/%.incl) \
	$(wildcard $(PROJ_SRC_DIR)/www/%.shtml)

$(DOCS_TGZ_FILE): www/apis/html/.dir $(DOCS_TAR_DEPS)
	$(Echo) Creating gzipped tar file for documentation
	$(Verb) find $(PROJ_SRC_DIR)/www -type d -exec chmod 755 {} \;
	$(Verb) find $(PROJ_SRC_DIR)/www -type f -exec chmod 644 {} \; 
	$(Verb) tar --directory $(PROJ_SRC_DIR) -lcf $(DOCS_TAR_FILE) www
	$(Verb) tar --directory $(PROJ_OBJ_DIR) -lrf $(DOCS_TAR_FILE) www
	$(Verb) gzip -f9 $(DOCS_TAR_FILE)
