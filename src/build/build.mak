###########################################################################
#
#   build.mak
#
#   MAME build tools makefile
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

OBJDIRS += \
	$(BUILDOBJ) \



#-------------------------------------------------
# set of build targets
#-------------------------------------------------

VERINFO = $(BUILDOUT)/verinfo$(BUILD_EXE)

ifneq ($(CROSS_BUILD),1)
BUILD += \
	$(VERINFO) \

#-------------------------------------------------
# verinfo
#-------------------------------------------------

VERINFOOBJS = \
	$(BUILDOBJ)/verinfo.o

$(VERINFO): $(VERINFOOBJS) $(LIBOCORE)
	@echo Linking $@...
	$(NATIVELD) $(NATIVELDFLAGS) $^ $(LIBS) -o $@

endif
