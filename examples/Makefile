# makefile for rc_mpu, builds example programs

# directories
SRCDIR		:= src
BINDIR		:= bin
INCLUDEDIR	:= ../libraries/include
LIBDIR		:= ../libraries/lib

EXAMPLES	:= $(shell find $(SRCDIR) -type f -name *.c)
TARGETS		:= $(EXAMPLES:$(SRCDIR)/%.c=$(BINDIR)/%)
INCLUDES	:= $(shell find $(INCLUDEDIR) -name '*.h')
SOURCES		:= $(shell find $(SRCDIR) -type f -name *.c)
# OBJECTS		:= $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
# .PRECIOUS	:  $(OBJECTS) # stop make from deleting objects

CC		:= gcc
LINKER		:= gcc

WFLAGS		:= -Wall -Wextra
CFLAGS		:= -pthread -I $(INCLUDEDIR)
LFLAGS		:= -lm -lrt -pthread -L $(LIBDIR) -lroboticscape


# commands
RM		:= rm -rf
INSTALL		:= install -m 4755
INSTALLDIR	:= install -d -m 755

# prefix variable in case this is used to make a deb package
prefix		:= /usr/local

LINK		:= ln -s -f
LINKDIR		:= /etc/roboticscape
LINKNAME	:= link_to_startup_program


all : $(TARGETS)

debug :
	$(MAKE) $(MAKEFILE) DEBUGFLAG="-g -D DEBUG"
	@echo " "
	@echo "Make Debug Complete"
	@echo " "

clean :
	@$(RM) $(BINDIR)
	@$(RM) $(BUILDDIR)
	@echo "Clean Complete"

install:
	$(MAKE)
	@$(INSTALL) $(TARGETS) $(prefix)/bin/
	@echo "install complete"

uninstall:
	@for f in $(TARGETS); do $(RM) $(prefix)/$$f; done
	@echo "uinstall complete"



# # rule for compiling all other objects
# $(BUILDDIR)/%.o : $(SRCDIR)/%.c
# 	@mkdir -p $(BUILDDIR)
# 	@$(CC) -c -o $@ $< $(CFLAGS) $(WFLAGS) $(ARCFLAGS) $(OPT_FLAGS) $(FLOAT_FLAG) $(DEBUGFLAG)
# 	@echo "made: $@"

# # rules for linking examples
# $(BINDIR)/% : $(BUILDDIR)/%.o
# 	@mkdir -p $(BINDIR)
# 	@$(LINKER) -o $@ $<  $(LFLAGS) $(OBJECTS)
# 	@echo "made: $@"

$(BINDIR)/% : $(SRCDIR)/%.c
	@mkdir -p $(BINDIR)
	@$(CC) -o $@ $< $(CFLAGS) $(WFLAGS)  $(DEBUGFLAG)  $(LFLAGS)
	@echo "made: $@"
