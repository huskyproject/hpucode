# $Id$
#
# Common makefiles part (include-file) for hpucode

# Use defines before including this file:
# _OBJ	object files suffix begins with point (".o", ".obj", ... )
# _LIB	library files suffix begins with point (".a", ".lib", ... )
# _DLL	dinamic linking library files suffix begins with point (".dll", ".so", ...)
# LIBPREFIX  library filename prefix ("lib", "", ...)
# LIBSUFFIX  library filename suffix (compiler ID, three characters, somewhere empty)
# DLLPREFIX  dinamic linking library prefix (usually empty)
# DLLSUFFIX  dinamic linking library suffix (compiler ID or empty)
# _SRC_DIR  sources dir

LIBNAME = hpucode

# Version
VERMAJOR= 1
VERMINOR= 9
VERPATCH= 0
VERH	= $(VERMAJOR).$(VERMINOR)
VER	= $(VERH).$(VERPATCH)

# Object files (please sort list to easy check by human)
OBJS    = uuecode$(_OBJ) uuefile$(_OBJ) scanmsg$(_OBJ) dupe$(_OBJ)

# Header files
HEADERS	= cvsdate.h version.h dupe.h uuecode.h

# Sources directory
_SRC_DIR= ../src/

# Headers directory
H_DIR	= ../h/

# Binary file(s) to build from sources
PROGRAMS= $(LIBNAME)$(_EXE)

# List of libraries required to build binary file(s)
LIBS	=
