# makefile for big numbers calculator 

IDIR=.
SDIR=.
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR=./lib

LIBS=-lm

_DEPS=	longmath.h

DEPS=$(addprefix $(IDIR)/, $(_DEPS))

_OBJ=	calcmain.o \
		term.o \
		calculation.o

OBJ=$(addprefix $(ODIR)/, $(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

calculator: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

all: $(OBJ)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir -p $(ODIR)

.PHONY:	clean

clean: 
	rm -fr $(ODIR) calculator* *~ $(IDIR)/*~ $(SDIR)/*~ log.txt

