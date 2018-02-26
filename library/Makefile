#
# makefile for libroboticscape.so
#

SRCDIR		:= src
LIBDIR		:= lib
BUILDDIR	:= build
INCLUDEDIR	:= include
SONAME		:= libroboticscape.so
TARGET		:= $(LIBDIR)/$(SONAME)

SOURCES		:= $(shell find $(SRCDIR) -type f -name *.c)
OBJECTS		:= $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
INCLUDES	:= $(shell find include -name '*.h')

CC		:= gcc
LINKER		:= gcc


# enable O3 optimization and vectorized math only for math libs
MATH_OPT_FLAGS	:= -O3 -ffast-math -ftree-vectorize

# general compiler flags
CFLAGS		:= -fPIC -Wall -Wextra -I $(INCLUDEDIR)
#ARCFLAGS	:= -mfpu=neon -march=armv7-a -mtune=cortex-a8
OPT_FLAGS	:= -O1
#FLOAT_FLAG	:= -mfloat-abi=hard

# linking flags
LFLAGS		:= -lm -lrt -lpthread -shared -Wl,-soname,$(SONAME)

# set prefix if not already set
prefix		?= /usr/local
RM		:= rm -f -r
INSTALL		:= install -m 755
INSTALLDIR	:= install -d -m 755



# linking rules
$(TARGET): $(OBJECTS)
	@mkdir -p $(LIBDIR)
	@$(LINKER) -o $(TARGET) $(OBJECTS) $(LFLAGS)
	@echo "Done linking $(TARGET)"

# rule for math libs
$(BUILDDIR)/math/%.o : $(SRCDIR)/math/%.c $(INCLUDES)
	@mkdir -p $(dir $(@))
	@$(CC) -c $(CFLAGS) $(ARCFLAGS) $(MATH_OPT_FLAGS) $(FLOAT_FLAG)  $(DEBUGFLAG) $< -o $(@)
	@echo "made: $(@)"

# rule for all other objects
$(BUILDDIR)/%.o : $(SRCDIR)/%.c $(INCLUDES)
	@mkdir -p $(dir $(@))
	@$(CC) -c $(CFLAGS) $(ARCFLAGS) $(OPT_FLAGS) $(FLOAT_FLAG) $(DEBUGFLAG) $< -o $(@)
	@echo "made: $(@)"




# rule for target
all:
	$(TARGET)

# compile with soft floating point math for labview
softfloat:
	$(MAKE) $(MAKEFILE) FLOAT_FLAG="-mfloat-abi=soft"
	@echo "$(TARGET) Make softfloat Complete"

# compile with debug symbols and DEBUG defined
debug:
	$(MAKE) $(MAKEFILE) DEBUGFLAG="-g -D DEBUG"
	@echo "$(TARGET) Make Debug Complete"

# make and install everything
install:
	$(MAKE)
	@# includes
	$(INSTALLDIR) $(DESTDIR)$(prefix)/include
	@cp -r include/* $(DESTDIR)$(prefix)/include
	@# library .so
	$(INSTALLDIR) $(DESTDIR)$(prefix)/lib
	$(INSTALL) $(TARGET) $(DESTDIR)$(prefix)/lib
	@echo "Install Complete"


clean:
	@$(RM) $(LIBDIR)
	@$(RM) $(BUILDDIR)
	@echo "Clean Complete"

uninstall:
	$(RM) $(DESTDIR)$(prefix)/$(TARGET)
	$(RM) $(DESTDIR)$(prefix)/include/roboticscape.h
	$(RM) $(DESTDIR)$(prefix)/include/rc_usefulincludes.h
	$(RM) $(DESTDIR)$(prefix)/include/rc
	@echo "Uninstall Complete"




