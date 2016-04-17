#
# A simple makefile for managing build of project composed of C source files.
#
# Julie Zelenski, for CS107, April 2009
# 

# It is likely that default C compiler is already gcc, but explicitly
# set, just to be sure
CC = gcc 

# The CFLAGS variable sets compile flags for gcc:
#  -g        compile with debug information
#  -Wall     give verbose compiler warnings
#  -0O       do not optimize generated code
#  -std=c99  use the C99 standard language definition
CFLAGS = -D_POSIX_C_SOURCE=3 -std=c99   

# The LDFLAGS variable sets flags for linker
#  -lm   says to link in libm (the math library)
LDFLAGS = -lm 

# In this section, you list the files that are part of the project.
# If you add/change names of header/source files, here is where you
# edit the Makefile.
HEADERS = pdp8.h 
SOURCES = pdp8.c main.c 
OBJECTS = $(SOURCES:.c=.o)
TARGET = pdp8


# The first target defined in the makefile is the one
# used when make is invoked with no argument. Given the definitions
# above, this Makefile file will build the one named TARGET and
# assume that it depends on all the named OBJECTS files.

$(TARGET) : $(OBJECTS) Makefile.dependencies
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

# In make's default rules, a .o automatically depends on its .c file
# (so editing the .c will cause recompilation into its .o file).
# The line below creates additional dependencies, most notably that it
# will cause the .c to reocmpiled if any included .h file changes.

Makefile.dependencies:: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -MM $(SOURCES) > Makefile.dependencies

-include Makefile.dependencies

# Phony means not a "real" target, it doesn't build anything
# The phony target "clean" that is used to remove all compiled object files.

.PHONY: clean

clean:
	@rm -fr $(TARGET) $(OBJECTS) core Makefile.dependencies