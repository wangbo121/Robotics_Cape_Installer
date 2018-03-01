

# directories
SRCDIR		:= src
BINDIR		:= bin
INCLUDEDIR	:= include

# basic definitions for rules
TARGET		:= $(BINDIR)/rc_test_gpio
SOURCES		:= $(shell find $(SRCDIR) -type f -name *.c)
INCLUDES	:= $(shell find $(INCLUDEDIR) -name '*.h')

# compiler and linker programs
CC		:= gcc
LINKER		:= gcc

# compile flags
WFLAGS		:= -Wall -Wextra
CFLAGS		:= -pthread -I $(INCLUDEDIR)
LFLAGS		:= -lm -lrt -pthread

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



clean:
	@$(RM) $(BINDIR)
	@echo "roboticscape Service Clean Complete"



$(TARGET) : $(SOURCES) $(INCLUDES)
	@mkdir -p $(BINDIR)
	@$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS) $(WFLAGS) $(DEBUGFLAG) $(LFLAGS)
	@echo "made: $@"
