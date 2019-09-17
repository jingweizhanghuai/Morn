OPENMP=1
JPG_PNG=1
DEBUG=0

CC = gcc
LIB = ar
LIBDIR = ./lib/x64/gnu
REMOVE = rm -rf

CCFLAGS = -O2 -fPIC -Wall
ifeq ($(OPENMP), 1) 
CCFLAGS += -fopenmp
endif
ifeq ($(DEBUG), 1) 
CCFLAGS += -DDEBUG
endif

STATICLIB = $(LIBDIR)/libmorn.a
SHAREDLIB = ./tool/libmorn.so

TOOLSOURCES = $(wildcard ./tool/*.c)
TOOLEXE = $(patsubst %.c,%.exe,$(TOOLSOURCES))

CSOURCES  = $(wildcard ./src/util/*.c)
CSOURCES += $(wildcard ./src/math/*.c)
CSOURCES += $(wildcard ./src/wave/*.c)
CSOURCES += $(wildcard ./src/image/*.c)
# CSOURCES += $(wildcard ./src/machine_learning/*.c)
CSOURCES += $(wildcard ./src/deep_learning/*.c)
ifeq ($(JPG_PNG), 1) 
CSOURCES += $(wildcard ./src/wrap/*.c)
endif

OBJS = $(patsubst %.c,%.o,$(CSOURCES))

DEPENDLIB = -lopenblas -ljpeg -lpng -lz -lm

INCLUDEDIR = -I ./include -I ./lib/include/

all:$(STATICLIB) $(SHAREDLIB) $(TOOLEXE)

tool:$(TOOLEXE)

%.exe:%.c $(STATICLIB)
	$(CC) $(CCFLAGS) $(INCLUDEDIR) $< $(STATICLIB) -L $(LIBDIR) $(DEPENDLIB) -o $@

static:$(STATICLIB)

$(STATICLIB):$(OBJS)
	$(LIB) -rc $(STATICLIB) $(OBJS)

shared:$(SHAREDLIB)

$(SHAREDLIB):$(OBJS)
	$(CC) $(CCFLAGS) -shared $(OBJS) -o $(SHAREDLIB)

%.o:%.c
	$(CC) $(CCFLAGS) $(INCLUDEDIR) -c $< -o $@

clean:
	$(REMOVE) $(OBJS)
	$(REMOVE) $(STATICLIB)
	$(REMOVE) $(SHAREDLIB)
	$(REMOVE) $(TOOLEXE)



