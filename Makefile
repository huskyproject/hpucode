# Generic Makefile for hpucode

include ../huskymak.cfg

ifeq ($(DEBUG), 1)
  CFLAGS = -I$(INCDIR) -I.$(DIRSEP)h $(DEBCFLAGS) $(WARNFLAGS)
  LFLAGS = $(DEBLFLAGS)
else
  CFLAGS = -I$(INCDIR) -I.$(DIRSEP)h $(OPTCFLAGS) $(WARNFLAGS)
  LFLAGS = $(OPTLFLAGS)
endif

ifeq ($(SHORTNAME), 1)
  LIBS  = -L$(LIBDIR) -lfidoconf -lsmapi
else
  LIBS  = -L$(LIBDIR) -lfidoconfig -lsmapi
endif

CDEFS = -D$(OSTYPE) $(ADDCDEFS)

CFLAGS += -Wall -pedantic -Wno-char-subscripts

OBJS = uuecode.o uuefile.o scanmsg.o dupe.o

SRC_DIR = .$(DIRSEP)src$(DIRSEP)


hpucode: $(OBJS)
		gcc $(OBJS) $(LFLAGS) $(LIBS) -o hpucode$(EXE)

%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) $(CDEFS) -c $<
        
clean:
	$(RM) $(RMOPT) *.o *~

distclean: clean
	-$(RM) $(RMOPT) hpucode
	-$(RM) $(RMOPT) hpucode.info
	-$(RM) $(RMOPT) hpucode.html

info:
	makeinfo --no-split hpucode.texi

html:
	makeinfo --html hpucode.texi

docs: info html

all: hpucode docs
        
install: all
	$(INSTALL) $(IBOPT) hpucode$(EXE) $(BINDIR)
ifdef INFODIR
	-$(MKDIR) $(MKDIROPT) $(INFODIR)
	$(INSTALL) hpucode.info $(INFODIR)
	-install-info --info-dir=$(INFODIR)  $(INFODIR)$(DIRSEP)hpucode.info
endif
ifdef HTMLDIR
	-$(MKDIR) $(MKDIROPT) $(HTMLDIR)
	$(INSTALL) hpucode*html $(HTMLDIR)
endif

uninstall:
	$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)hpucode$(EXE)
ifdef INFODIR
	$(RM) $(RMOPT) $(INFODIR)$(DIRSEP)hpucode.info
endif
ifdef HTMLDIR
	$(RM) $(RMOPT) $(HTMLDIR)$(DIRSEP)hpucode.html
endif
