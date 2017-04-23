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

_OBJALL=calcmain.o \
		sqrtmain.o \
		term.o \
		calculation.o

_OBJ=	calcmain.o \
		term.o \
		calculation.o

_OBJ2=	sqrtmain.o \
		term.o \
		calculation.o


OBJ=$(addprefix $(ODIR)/, $(_OBJ))
OBJ2=$(addprefix $(ODIR)/, $(_OBJ2))
OBJALL=$(addprefix $(ODIR)/, $(_OBJALL))


$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

calculator: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

squareroot: $(OBJ2)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)



all: $(OBJALL)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir -p $(ODIR)

.PHONY:	clean

clean: 
	rm -fr $(ODIR) calculator* squareroot* *~ $(IDIR)/*~ $(SDIR)/*~ log.txt

