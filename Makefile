# Generic Makefile for hpucode

ifeq ($(DEBIAN), 1)
# Every Debian-Source-Paket has one included.
include /usr/share/husky/huskymak.cfg
else
include ../huskymak.cfg
endif


ifeq ($(DEBUG), 1)
  CFLAGS = -I$(INCDIR) -I.$(DIRSEP)h $(DEBCFLAGS) $(WARNFLAGS)
  LFLAGS = $(DEBLFLAGS)
else
  CFLAGS = -I$(INCDIR) -I.$(DIRSEP)h $(OPTCFLAGS) $(WARNFLAGS)
  LFLAGS = $(OPTLFLAGS)
endif

ifeq ($(SHORTNAME), 1)
  LIBS=-L$(LIBDIR) -lhusky -lfidoconf -lsmapi
else
  LIBS=-L$(LIBDIR) -lhusky -lfidoconfig -lsmapi
endif

CDEFS = -D$(OSTYPE) $(ADDCDEFS)

CFLAGS += -Wall -pedantic -Wno-char-subscripts

OBJS = uuecode.o uuefile.o scanmsg.o dupe.o

SRC_DIR = .$(DIRSEP)src$(DIRSEP)


hpucode: $(OBJS)
		gcc $(OBJS) $(LFLAGS) $(LIBS) -o hpucode$(_EXE)

%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) $(CDEFS) -c $<
        
hpucode.1.gz: man/hpucode.1
	gzip -9c man/hpucode.1 > hpucode.1.gz

clean:
	-$(RM) $(RMOPT) *$(_OBJ)
	-$(RM) $(RMOPT) *~

distclean: clean
	-$(RM) $(RMOPT) hpucode$(_EXE)
	-$(RM) $(RMOPT) hpucode.info
	-$(RM) $(RMOPT) hpucode.html
	-$(RM) $(RMOPT) hpucode.1.gz

info:
	makeinfo --no-split hpucode.texi

html:
	makeinfo --html --no-split hpucode.texi

docs: info html

ifdef INFODIR
  all: hpucode info hpucode.1.gz
ifdef HTMLDIR
  all: hpucode docs hpucode.1.gz
endif
else
ifdef HTMLDIR
  all: hpucode html hpucode.1.gz
else
  all: hpucode hpucode.1.gz
endif
endif
        
install: all
	$(INSTALL) $(IBOPT) hpucode$(_EXE) $(BINDIR)
ifdef INFODIR
	-$(MKDIR) $(MKDIROPT) $(INFODIR)
	$(INSTALL) $(IMOPT) hpucode.info $(INFODIR)
	-install-info --info-dir=$(INFODIR)  $(INFODIR)$(DIRSEP)hpucode.info
endif
ifdef HTMLDIR
	-$(MKDIR) $(MKDIROPT) $(HTMLDIR)
	$(INSTALL) $(IMOPT) hpucode.html $(HTMLDIR)
endif
ifdef MANDIR
	-$(MKDIR) $(MKDIROPT) $(MANDIR)$(DIRSEP)man1
	$(INSTALL) $(IMOPT) hpucode.1.gz $(MANDIR)$(DIRSEP)man1
endif

uninstall:
	$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)hpucode$(_EXE)
ifdef INFODIR
	$(RM) $(RMOPT) $(INFODIR)$(DIRSEP)hpucode.info
endif
ifdef HTMLDIR
	$(RM) $(RMOPT) $(HTMLDIR)$(DIRSEP)hpucode.html
endif
ifdef MANDIR
	$(RM) $(RMOPT) $(MANDIR)$(DIRSEP)man1$(DIRSEP)hpucode.1.gz
endif
