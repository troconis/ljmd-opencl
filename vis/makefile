
# OS Name (Linux or Darwin)
OSUPPER = $(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
OSLOWER = $(shell uname -s 2>/dev/null | tr [:upper:] [:lower:])

# Flags to detect 32-bit or 64-bit OS platform
OS_SIZE = $(shell uname -m | sed -e "s/i.86/32/" -e "s/x86_64/64/")
OS_ARCH = $(shell uname -m | sed -e "s/i386/i686/")

# Flags to detect either a Linux system (linux) or Mac OSX (darwin)
DARWIN = $(strip $(findstring DARWIN, $(OSUPPER)))

CC             := g++

# OS-specific build flags
ifneq ($(DARWIN),) 
      CCFLAGS   := -arch $(OS_ARCH) 
else
  ifeq ($(OS_SIZE),32)
      CCFLAGS   := -m32
  else
      CCFLAGS   := -m64
  endif
endif

TOP      = $(shell pwd)
LCL_OBJDIR = objs
LCL_DOCDIR = docs

EXE         := ljmd-vis

LCL_SOURCES := $(TOP)/ljmd-memhand.cpp $(TOP)/ljmd-povtools.cpp $(TOP)/ljmd-vis.cpp

LCL_HEADERS := $(TOP)/ljmd-memhand.h $(TOP)/ljmd-povtools.h

LCL_OBJS    := $(LCL_OBJDIR)/ljmd-memhand.o  $(LCL_OBJDIR)/ljmd-povtools.o \
               $(LCL_OBJDIR)/ljmd-trajclass.o $(LCL_OBJDIR)/ljmd-vis.o

all: build

build: $(LCL_OBJS) $(EXE)

$(LCL_OBJDIR)/ljmd-memhand.o: $(TOP)/ljmd-memhand.cpp $(TOP)/ljmd-memhand.h
	$(CC) $(CCFLAGS) -o $@ -c $<

$(LCL_OBJDIR)/ljmd-povtools.o: $(TOP)/ljmd-povtools.cpp $(TOP)/ljmd-povtools.h
	$(CC) $(CCFLAGS) -o $@ -c $<

$(LCL_OBJDIR)/ljmd-trajclass.o: $(TOP)/ljmd-trajclass.cpp $(TOP)/ljmd-trajclass.h
	$(CC) $(CCFLAGS) -o $@ -c $<

$(LCL_OBJDIR)/ljmd-vis.o: $(TOP)/ljmd-vis.cpp
	$(CC) $(CCFLAGS) -o $@ -c $<

$(EXE): $(LCL_OBJS)
	$(CC) $(LCL_OBJS) -o $(EXE)

documents:
	cd $(LCL_DOCDIR); doxygen Doxyfile

clean:
	rm -f $(LCL_OBJDIR)/*.o $(TOP)/*.jpg output.avi $(EXE)
	rm -rf  $(LCL_DOCDIR)/html/*

test: $(EXE)
	./$(EXE) test.xyz
	@./callmencoder

