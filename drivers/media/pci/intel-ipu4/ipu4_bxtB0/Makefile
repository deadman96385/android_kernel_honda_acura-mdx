#
#  Copyright (c) 2010 - 2017, Intel Corporation.
#
#  This program is free software; you can redistribute it and/or modify it
#  under the terms and conditions of the GNU General Public License,
#  version 2, as published by the Free Software Foundation.
#
#  This program is distributed in the hope it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.
#

ifneq ($(EXTERNAL_BUILD), 1)
srcpath := $(srctree)
endif

IPU_STEP = bxtB0
ccflags-y += -DIPU_STEP_BXTB0 -DIPU_STEP_BXTC0

include $(srcpath)/$(src)/../Makefile.link
include $(srcpath)/$(src)/../Makefile.common
ifdef CONFIG_VIDEO_INTEL_IPU_FW_LIB
include $(srcpath)/$(src)/../Makefile.lib2600psys
endif
include $(srcpath)/$(src)/../Makefile.isys
ifdef CONFIG_VIDEO_INTEL_IPU_FW_LIB
include $(srcpath)/$(src)/../Makefile.lib2600
endif
include $(srcpath)/$(src)/../Makefile.mmu
include $(srcpath)/$(src)/../Makefile.deferred
ccflags-y += -I$(srcpath)/$(src)/../../../../../include/
ccflags-y += -I$(srcpath)/$(src)/../

ccflags-y += -DPARAMETER_INTERFACE_V2
