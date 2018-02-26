# This is a general use makefile for robotics cape projects written in C.
# Just change the target name to match your main source code filename.
TARGET = rc_battery_monitor

INCLUDEDIR	:= ../../libraries/include
LIBDIR		:= ../../libraries/lib

CC		:= gcc
LINKER		:= gcc

CFLAGS		:= -Wall -Wall -Wextra -I $(INCLUDEDIR)
OPT_FLAGS	:= -O1
LFLAGS		:= -L $(LIBDIR) -lm -lrt -lpthread -lroboticscape

SOURCES		:= $(wildcard *.c)
INCLUDES	:= $(wildcard *.h)
OBJECTS		:= $(SOURCES:$%.c=$%.o)


prefix		:= /usr
RM		:= rm -f
INSTALL		:= install -o root -g root -m 4755
INSTALLDIR	:= install -d -m 755

LINK		:= ln -s -f
LINKDIR		:= /etc/roboticscape
LINKNAME	:= link_to_startup_program


# linking
$(TARGET): $(OBJECTS)
	@$(LINKER) $(LFLAGS) -o $(TARGET) $(OBJECTS)

# rule for objects
$(OBJECTS): %.o : %.c $(INCLUDES)
	@$(CC) -c $(CFLAGS) $(OPT_FLAGS) $(DEBUGFLAG) $< -o $(@)


all:
	$(TARGET)
	@echo "$(TARGET) Make Complete"

# compile with debug symbols and DEBUG defined
debug:
	$(MAKE) $(MAKEFILE) DEBUGFLAG="-g -D DEBUG"

install:
	@$(MAKE) --no-print-directory
	@$(INSTALLDIR) $(DESTDIR)$(PREFIX)/bin
	@$(INSTALL) $(TARGET) $(DESTDIR)$(PREFIX)/bin
	@echo "$(TARGET) Install Complete"

clean:
	@$(RM) $(OBJECTS)
	@$(RM) $(TARGET)
	@echo "$(TARGET) Clean Complete"

uninstall:
	@$(RM) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	@echo "$(TARGET) Uninstall Complete"



