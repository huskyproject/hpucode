# hpucode/Makefile
#
# This file is part of hpucode, part of the Husky fidonet software project
# Use with GNU make v.3.82 or later
# Requires: husky enviroment
#

hpucode_LIBS := $(fidoconf_TARGET_BLD) $(smapi_TARGET_BLD) $(huskylib_TARGET_BLD)

hpucode_CDEFS := $(CDEFS) -I$(fidoconf_ROOTDIR) \
                        -I$(smapi_ROOTDIR) \
                        -I$(huskylib_ROOTDIR) \
                        -I$(hpucode_ROOTDIR)$(hpucode_H_DIR)

hpucode_TARGET     = hpucode$(_EXE)
hpucode_TARGET_BLD = $(hpucode_BUILDDIR)$(hpucode_TARGET)
hpucode_TARGET_DST = $(BINDIR_DST)$(hpucode_TARGET)

ifdef MAN1DIR
    hpucode_MAN1PAGES := hpucode.1
    hpucode_MAN1BLD := $(hpucode_BUILDDIR)$(hpucode_MAN1PAGES)$(_COMPR)
    hpucode_MAN1DST := $(DESTDIR)$(MAN1DIR)$(DIRSEP)$(hpucode_MAN1PAGES)$(_COMPR)
endif


.PHONY: hpucode_build hpucode_install hpucode_uninstall hpucode_clean hpucode_distclean \
        hpucode_depend hpucode_doc hpucode_doc_install hpucode_doc_uninstall \
        hpucode_doc_clean hpucode_doc_distclean hpucode_rmdir_DEP hpucode_rm_DEPS \
        hpucode_clean_OBJ hpucode_main_distclean

hpucode_build: $(hpucode_TARGET_BLD) $(hpucode_MAN1BLD) hpucode_doc

ifneq ($(MAKECMDGOALS), depend)
    include $(hpucode_DOCDIR)Makefile
    ifneq ($(MAKECMDGOALS), distclean)
        ifneq ($(MAKECMDGOALS), uninstall)
            include $(hpucode_DEPS)
        endif
    endif
endif


# Build application
$(hpucode_TARGET_BLD): $(hpucode_ALL_OBJS) $(hpucode_LIBS) | do_not_run_make_as_root
	$(CC) $(LFLAGS) $(EXENAMEFLAG) $@ $^ $(hpucode_LIBZ)

# Compile .c files
$(hpucode_ALL_OBJS): $(hpucode_OBJDIR)%$(_OBJ): $(hpucode_SRCDIR)%.c | $(hpucode_OBJDIR)
	$(CC) $(hpucode_CFLAGS) $(hpucode_CDEFS) -o $(hpucode_OBJDIR)$*$(_OBJ) $(hpucode_SRCDIR)$*.c

$(hpucode_OBJDIR): | $(hpucode_BUILDDIR) do_not_run_make_as_root
	[ -d $@ ] || $(MKDIR) $(MKDIROPT) $@


# Build man pages
ifdef MAN1DIR
    $(hpucode_MAN1BLD): $(hpucode_MANDIR)$(hpucode_MAN1PAGES)
	@[ $$($(ID) $(IDOPT)) -eq 0 ] && echo "DO NOT run \`make\` from root" && exit 1 || true
    ifdef COMPRESS
		$(COMPRESS) -c $< > $@
    else
		$(CP) $(CPOPT) $< $@
    endif

else
    $(hpucode_MAN1BLD): ;
endif


# Install
ifneq ($(MAKECMDGOALS), install)
    hpucode_install: ;
else
    hpucode_install: $(hpucode_TARGET_DST) hpucode_install_man hpucode_doc_install ;
endif

$(hpucode_TARGET_DST): $(hpucode_TARGET_BLD) | $(DESTDIR)$(BINDIR)
	$(INSTALL) $(IBOPT) $< $(DESTDIR)$(BINDIR); \
	$(TOUCH) "$@"

ifndef MAN1DIR
    hpucode_install_man: ;
else
    hpucode_install_man: $(hpucode_MAN1DST)

    $(hpucode_MAN1DST): $(hpucode_MAN1BLD) | $(DESTDIR)$(MAN1DIR)
	$(INSTALL) $(IMOPT) $< $(DESTDIR)$(MAN1DIR); $(TOUCH) "$@"
endif


# Clean
hpucode_clean: hpucode_clean_OBJ hpucode_doc_clean
	-[ -d "$(hpucode_OBJDIR)" ] && $(RMDIR) $(hpucode_OBJDIR) || true

hpucode_clean_OBJ:
	-$(RM) $(RMOPT) $(hpucode_OBJDIR)*

# Distclean
hpucode_distclean: hpucode_doc_distclean hpucode_main_distclean hpucode_rmdir_DEP
	-[ -d "$(hpucode_BUILDDIR)" ] && $(RMDIR) $(hpucode_BUILDDIR) || true

hpucode_rmdir_DEP: hpucode_rm_DEPS
	-[ -d "$(hpucode_DEPDIR)" ] && $(RMDIR) $(hpucode_DEPDIR) || true

hpucode_rm_DEPS:
	-$(RM) $(RMOPT) $(hpucode_DEPDIR)*

hpucode_main_distclean: hpucode_clean
	-$(RM) $(RMOPT) $(hpucode_TARGET_BLD)
ifdef MAN1DIR
	-$(RM) $(RMOPT) $(hpucode_MAN1BLD)
endif


# Uninstall
hpucode_uninstall: hpucode_doc_uninstall
	-$(RM) $(RMOPT) $(hpucode_TARGET_DST)
ifdef MAN1DIR
	-$(RM) $(RMOPT) $(hpucode_MAN1DST)
endif
