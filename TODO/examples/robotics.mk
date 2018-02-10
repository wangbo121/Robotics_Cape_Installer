# shared makefile for all the examples

INCLUDEDIR	:= ../../libraries/include
LIBDIR		:= ../../libraries/lib

SOURCES		:= $(wildcard *.c)
INCLUDES	:= $(wildcard *.h)
OBJECTS		:= $(SOURCES:$%.c=$%.o)

CC		:= gcc
LINKER		:= gcc

CFLAGS		:= -Wall -Wextra -I $(INCLUDEDIR)
OPT_FLAGS	:= -O1
LFLAGS		:= -L $(LIBDIR) -lm -lrt -lpthread -lroboticscape

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



# rule for target
all:
	$(TARGET)

# compile with debug symbols and DEBUG defined
debug:
	$(MAKE) $(MAKEFILE) DEBUGFLAG="-g -D DEBUG"

install:
	@$(MAKE) --no-print-directory
	@$(INSTALLDIR) $(DESTDIR)$(prefix)/bin
	@$(INSTALL) $(TARGET) $(DESTDIR)$(prefix)/bin

clean:
	@$(RM) $(OBJECTS)
	@$(RM) $(TARGET)

uninstall:
	@$(RM) $(DESTDIR)$(prefix)/bin/$(TARGET)

runonboot:
	@$(MAKE) install --no-print-directory
	@$(LINK) $(DESTDIR)$(prefix)/bin/$(TARGET) $(LINKDIR)/$(LINKNAME)
	@echo "$(TARGET) Set to Run on Boot"