#----------------------------------------------------------------------------
#	Local macros
#----------------------------------------------------------------------------

BIN   	= client

LDLIBS  = -leasyace 
LDFLAGS = -static -L../../ace_using/easyace 
CCFLAGS += -I../../easyace -I../comm/ -I./

FILES 	= client 

SRC	= $(addsuffix .cpp,$(FILES))
OBJ	= $(addsuffix .o,$(FILES))

#----------------------------------------------------------------------------
#	Include macros and targets
#----------------------------------------------------------------------------

include	$(ACE_ROOT)/include/makeinclude/wrapper_macros.GNU
include	$(ACE_ROOT)/include/makeinclude/macros.GNU
include	$(ACE_ROOT)/include/makeinclude/rules.common.GNU
include	$(ACE_ROOT)/include/makeinclude/rules.nonested.GNU
include	$(ACE_ROOT)/include/makeinclude/rules.bin.GNU
include	$(ACE_ROOT)/include/makeinclude/rules.local.GNU

#----------------------------------------------------------------------------
#	Local targets
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#	Dependencies
#----------------------------------------------------------------------------

# DO NOT DELETE THIS LINE -- g++dep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

