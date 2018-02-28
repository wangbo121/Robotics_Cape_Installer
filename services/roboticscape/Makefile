# makefile for roboticscape service

SERVICE		:= roboticscape

# directories
SRCDIR		:= src
BINDIR		:= bin
INCLUDEDIR	:= ../../library/include
LIBDIR		:= ../../library/lib

# basic definitions for rules
TARGET		:= $(BINDIR)/rc_startup_routine
INCLUDES	:= $(shell find $(INCLUDEDIR) -name '*.h')
SOURCES		:= $(SRCDIR)/rc_startup_routine.c

# compiler and linker programs
CC		:= gcc
LINKER		:= gcc

# compile flags
WFLAGS		:= -Wall -Wextra
CFLAGS		:= -pthread -I $(INCLUDEDIR)
LFLAGS		:= -lm -lrt -pthread -L $(LIBDIR) -lroboticscape

# commands
RM		:= rm -rf
INSTALL		:= install -m 4755
INSTALLDIR	:= install -d -m 755
LINK		:= ln -s -f
LINKDIR		:= /etc/roboticscape
LINKNAME	:= link_to_startup_program

# prefix variable in case this is used to make a deb package
prefix		:= /usr/local


all: $(TARGET)

install:
	@$(MAKE)
	@$(INSTALL) $(TARGET) $(prefix)/bin/
	@$(INSTALLDIR) $(DESTDIR)/lib/systemd/system
	@$(INSTALLNONEXEC) $(SERVICE).service $(DESTDIR)/lib/systemd/system/
	@$(INSTALLDIR) $(DESTDIR)/etc/modules-load.d/
	@$(INSTALLNONEXEC) roboticscape_modules.conf $(DESTDIR)/etc/modules-load.d/
	@echo "roboticscape Service Install Complete"

clean:
	@$(RM) $(BINDIR)
	@echo "roboticscape Service Clean Complete"

uninstall:
	@$(RM) $(DESTDIR)/lib/systemd/system/$(SERVICE).service
	@$(RM) $(DESTDIR)/etc/modules-load.d/roboticscape_modules.conf

	@echo "roboticscape Service Uninstall Complete"


$(BINDIR)/% : $(SRCDIR)/%.c
	@mkdir -p $(BINDIR)
	@$(CC) -o $@ $< $(CFLAGS) $(WFLAGS)  $(DEBUGFLAG)  $(LFLAGS)
	@echo "made: $@"
