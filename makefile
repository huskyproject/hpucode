# Generic Makefile for hpucode

include ../huskymak.cfg

ifeq ($(DEBUG), 1)
  CFLAGS = -I$(INCDIR) $(DEBCFLAGS) $(WARNFLAGS)
  LFLAGS = $(DEBLFLAGS)
else
  CFLAGS = -I$(INCDIR) $(OPTCFLAGS) $(WARNFLAGS)
  LFLAGS = $(OPTLFLAGS)
endif

ifeq ($(SHORTNAME), 1)
  LIBS  = -L$(LIBDIR) -lfidoconf -lsmapi
else
  LIBS  = -L$(LIBDIR) -lfidoconfig -lsmapi
endif

CDEFS= -D$(OSTYPE) $(ADDCDEFS)

OBJS= uuecode.o uuefile.o scanmsg.o dupe.o

all: hpucode

hpucode: $(OBJS)
		gcc $(OBJS) $(LFLAGS) $(LIBS) -o hpucode

%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) $(CDEFS) -c $<
        
clean:
	$(RM) $(RMOPT) *.o *~

distclean: clean
	$(RM) $(RMOPT) hpucode
        
install: hpucode$(EXE)
	$(INSTALL) $(IBOPT) hpucode$(EXE) $(BINDIR)
ifdef INFODIR
	-$(MKDIR) $(MKDIROPT) $(INFODIR)
	$(INSTALL) hpucode.info $(INFODIR)
	-install-info --info-dir=$(INFODIR)  $(INFODIR)$(DIRSEP)hpucode.info
endif
ifdef HTMLDIR
	-$(MKDIR) $(MKDIROPT) $(HTMLDIR)
	$(INSTALL) hpucode.html $(HTMLDIR)
endif
ifdef MANDIR
	-$(MKDIR) $(MKDIROPT) $(MANDIR)$(DIRSEP)man1
	$(INSTALL) $(IMOPT) hpucode.1.gz $(MANDIR)$(DIRSEP)man1
endif

uninstall:
	$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)hpucode$(EXE)
ifdef INFODIR
	$(RM) $(RMOPT) $(INFODIR)$(DIRSEP)hpucode.info
endif
ifdef HTMLDIR
	$(RM) $(RMOPT) $(HTMLDIR)$(DIRSEP)hpucode.html
endif
ifdef MANDIR
	$(RM) $(RMOPT) $(MANDIR)$(DIRSEP)man1$(DIRSEP)hpucode.1.gz
endif


hpucode.1.gz: man/hpucode.1
	gzip -9c man/hpucode.1 > hpucode.1.gz

info:
	makeinfo --no-split hpucode.texi

html:
	makeinfo --html --no-split hpucode.texi

docs: info html
