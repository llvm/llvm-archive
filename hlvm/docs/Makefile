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
	      doxygen.intro apis

include $(LEVEL)/Makefile.hlvm

DOCS_TAR_FILE := $(PROJ_OBJ_DIR)/hlvm.www.tar
DOCS_TGZ_FILE := $(PROJ_OBJ_DIR)/hlvm.www.tar.gz

.PHONY: doxygen

doxygen:: $(DOCS_TGZ_FILE)

DOXYGEN_DEPS = \
	Doxyfile \
	apis/html/.dir \
	$(PROJ_SRC_DIR)/doxygen.css \
	$(PROJ_SRC_DIR)/doxygen.header \
	$(PROJ_SRC_DIR)/doxygen.footer \
	$(PROJ_SRC_DIR)/doxygen.intro \
	$(PROJ_SRC_ROOT)/hlvm/Base/Config.h.in \

apis/html/index.html : $(DOXYGEN_DEPS)
	$(Echo) "Running Doxygen (be patient)"
	$(Verb) cp $(PROJ_SRC_DIR)/doxygen.css doxygen.css
	$(Verb) doxygen Doxyfile >doxygen.out 2>&1
	$(Verb) cp $(PROJ_SRC_DIR)/doxygen.css apis/html

DOCS_TAR_DEPS = \
	apis/html/index.html \
	$(wildcard $(PROJ_SRC_DIR)/%.incl) \
	$(wildcard $(PROJ_SRC_DIR)/%.shtml)

$(DOCS_TGZ_FILE): apis/html/.dir $(DOCS_TAR_DEPS)
	$(Echo) Creating gzipped tar file for documentation
	$(Verb) find $(PROJ_SRC_DIR) -type d -exec chmod 755 {} \;
	$(Verb) find $(PROJ_SRC_DIR) -type f -exec chmod 644 {} \; 
	$(Verb) tar --directory $(PROJ_OBJ_DIR) -lrf $(DOCS_TAR_FILE) apis
	$(Verb) gzip -f9 $(DOCS_TAR_FILE)

WWW_ROOT := /var/www/sites/hlvm
doxygen-install: $(DOCS_TGZ_FILE)
	$(Echo) "Copying API docs to web site"
	$(Verb) cp $(DOCS_TGZ_FILE) $(WWW_ROOT)
	$(Verb) cd $(WWW_ROOT); \
	        tar zxf $(DOCS_TGZ_FILE) ; \
		mv $(DOCS_TGZ_FILE) apis/doxygen.tar.gz
	$(Verb) cd $(WWW_ROOT)/apis/html ; svn add *
