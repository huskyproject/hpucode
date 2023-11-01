# A makefile for hpucode documentation
# hpucode/doc/Makefile

.PHONY: hpucode_info hpucode_html hpucode_txt hpucode_dvi hpucode_pdf \
        hpucode_rmdir_DOC hpucode_clean_DOC \
        hpucode_install_info hpucode_install_html hpucode_install_txt \
        hpucode_install_dvi hpucode_install_pdf hpucode_info_uninstall hpucode_docs_uninstall

ifdef DOCDIR
    hpucode_doc: hpucode_info hpucode_html hpucode_txt hpucode_dvi hpucode_pdf ;
else
    hpucode_doc: hpucode_info ;
endif

ifdef INFODIR
    hpucode_info: $(hpucode_BUILDDIR)hpucode.info$(_COMPR)
else
    hpucode_info: ;
endif

ifeq ($(HTML), 1)
    hpucode_html: $(hpucode_BUILDDIR)hpucode.html
else
    hpucode_html: ;
endif

ifeq ($(TXT), 1)
    hpucode_txt: $(hpucode_BUILDDIR)hpucode.txt
else
    hpucode_txt: ;
endif

ifeq ($(DVI), 1)
    hpucode_dvi: $(hpucode_BUILDDIR)hpucode.dvi
else
    hpucode_dvi: ;
endif

ifeq ($(PDF), 1)
    hpucode_pdf: $(hpucode_BUILDDIR)hpucode.pdf
else
    hpucode_pdf: ;
endif

hpucode_DOCDIR_BLD=$(hpucode_BUILDDIR)doc$(DIRSEP)
hpucode_TEXI=$(hpucode_DOCDIR_BLD)hpucode.texi

ifdef COMPRESS
$(hpucode_BUILDDIR)hpucode.info$(_COMPR): $(hpucode_BUILDDIR)hpucode.info
	$(COMPRESS) -c $? > $@
endif

$(hpucode_BUILDDIR)hpucode.info: $(hpucode_TEXI)
	-$(MAKEINFO) $(MINOSPLIT) "$<" $(MIOUT) "$@"

$(hpucode_BUILDDIR)hpucode.html: $(hpucode_TEXI)
	-LC_ALL=C $(MAKEINFO) $(MIHTML) $(MINOSPLIT) "$<" $(MIOUT) "$@"

$(hpucode_BUILDDIR)hpucode.txt: $(hpucode_TEXI)
	-LC_ALL=C $(MAKEINFO) $(MIPTEXT) $(MINOSPLIT) "$<" $(MIOUT) "$@"

$(hpucode_BUILDDIR)hpucode.dvi: $(hpucode_DOCDIR_BLD)hpucode.dvi
	$(LN) $(LNHOPT) "$<" $(hpucode_BUILDDIR)

$(hpucode_DOCDIR_BLD)hpucode.dvi: $(hpucode_TEXI)
	-cd $(hpucode_DOCDIR_BLD); LC_ALL=C $(TEXI2DVI) ../../../"$<"

$(hpucode_BUILDDIR)hpucode.pdf: $(hpucode_DOCDIR_BLD)hpucode.pdf
	$(LN) $(LNHOPT) "$<" $(hpucode_BUILDDIR)

$(hpucode_DOCDIR_BLD)hpucode.pdf: $(hpucode_TEXI)
	-cd $(hpucode_DOCDIR_BLD); \
	LC_ALL=C $(MAKEINFO) $(MIPDF) $(MINOSPLIT) hpucode.texi $(MIOUT) hpucode.pdf

$(hpucode_TEXI): $(hpucode_DOCDIR)hpucode.texi | $(hpucode_DOCDIR_BLD)
	$(LN) $(LNHOPT) "$<" $(hpucode_DOCDIR_BLD)

$(hpucode_DOCDIR_BLD):
	[ -d "$(hpucode_BUILDDIR)" ] || $(MKDIR) $(MKDIROPT) "$(hpucode_BUILDDIR)"
	[ -d "$@" ] || $(MKDIR) $(MKDIROPT) "$@"


# Install
ifdef DOCDIR
# See "all_info_install" in the root Makefile
    hpucode_doc_install: all_info_install hpucode_install_html hpucode_install_txt \
                 hpucode_install_dvi hpucode_install_pdf ;
else
    hpucode_doc_install: all_info_install ;
endif

ifeq ($(HTML), 1)
    hpucode_install_html: $(DOCDIR_DST)hpucode.html ;

    $(DOCDIR_DST)hpucode.html: $(hpucode_BUILDDIR)hpucode.html | $(DOCDIR_DST)
		$(INSTALL) $(IMOPT) "$<" "$|"; \
		$(TOUCH) "$@"
else
    hpucode_install_html: ;
endif

ifeq ($(TXT), 1)
    hpucode_install_txt: $(DOCDIR_DST)hpucode.txt ;

    $(DOCDIR_DST)hpucode.txt: $(hpucode_BUILDDIR)hpucode.txt | $(DOCDIR_DST)
		$(INSTALL) $(IMOPT) "$<" "$|"; \
		$(TOUCH) "$@"
else
    hpucode_install_txt: ;
endif

ifeq ($(DVI), 1)
    hpucode_install_dvi: $(DOCDIR_DST)hpucode.dvi ;

    $(DOCDIR_DST)hpucode.dvi: $(hpucode_BUILDDIR)hpucode.dvi | $(DOCDIR_DST)
		$(INSTALL) $(IMOPT) "$<" "$|"; \
		$(TOUCH) "$@"
else
    hpucode_install_dvi: ;
endif

ifeq ($(PDF), 1)
    hpucode_install_pdf: $(DOCDIR_DST)hpucode.pdf ;

    $(DOCDIR_DST)hpucode.pdf: $(hpucode_BUILDDIR)hpucode.pdf | $(DOCDIR_DST)
		$(INSTALL) $(IMOPT) "$<" "$|"; \
		$(TOUCH) "$@"
else
    hpucode_install_pdf: ;
endif


# Clean
hpucode_doc_clean: hpucode_rmdir_DOC ;

hpucode_rmdir_DOC: hpucode_clean_DOC
	-[ -d "$(hpucode_DOCDIR_BLD)" ] && $(RMDIR) $(hpucode_DOCDIR_BLD) || true

hpucode_clean_DOC:
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.fns
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.aux
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.toc
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.cp
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.fn
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.vr
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.kr
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.ke
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.tp
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.kes
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.krs
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.ky
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.pg
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.cps
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)*.log
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.texi
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.dvi
	-$(RM) $(RMOPT) $(hpucode_DOCDIR_BLD)hpucode.pdf

# Distclean
hpucode_doc_distclean: hpucode_doc_clean
	-$(RM) $(RMOPT) $(hpucode_BUILDDIR)hpucode*dvi
	-$(RM) $(RMOPT) $(hpucode_BUILDDIR)hpucode.inf
	-$(RM) $(RMOPT) $(hpucode_BUILDDIR)hpucode*html
	-$(RM) $(RMOPT) $(hpucode_BUILDDIR)hpucode.ps
	-$(RM) $(RMOPT) $(hpucode_BUILDDIR)hpucode.doc
	-$(RM) $(RMOPT) $(hpucode_BUILDDIR)hpucode*txt
	-$(RM) $(RMOPT) $(hpucode_BUILDDIR)hpucode.pdf
	-$(RM) $(RMOPT) $(hpucode_BUILDDIR)hpucode.info*


# Uninstall
ifdef DOCDIR
    hpucode_doc_uninstall: hpucode_info_uninstall hpucode_docs_uninstall ;
else
    hpucode_doc_uninstall: hpucode_info_uninstall ;
endif

ifdef INFODIR
# See "all_info_uninstall" in the root Makefile
    hpucode_info_uninstall: all_info_uninstall
		-$(RM) $(RMOPT) $(INFODIR_DST)hpucode.info$(_COMPR)
else
    hpucode_info_uninstall: ;
endif

hpucode_docs_uninstall:
ifeq ($(HTML), 1)
	-$(RM) $(RMOPT) $(DOCDIR_DST)hpucode.html
endif
ifeq ($(TXT), 1)
	-$(RM) $(RMOPT) $(DOCDIR_DST)hpucode.txt
endif
ifeq ($(DVI), 1)
	-$(RM) $(RMOPT) $(DOCDIR_DST)hpucode.dvi
endif
ifeq ($(PDF), 1)
	-$(RM) $(RMOPT) $(DOCDIR_DST)hpucode.pdf
endif
