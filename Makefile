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

uninstall:
	$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)hpucode$(EXE)
