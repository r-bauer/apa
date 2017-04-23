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
		amrtmain.o \
		calculation.o

_OBJ=	calcmain.o \
		term.o \
		calculation.o

_OBJ2=	sqrtmain.o \
		term.o \
		calculation.o

_OBJ3=  term.o \
		amrtmain.o \
		calculation.o


OBJ=$(addprefix $(ODIR)/, $(_OBJ))
OBJ2=$(addprefix $(ODIR)/, $(_OBJ2))
OBJ3=$(addprefix $(ODIR)/, $(_OBJ3))
OBJALL=$(addprefix $(ODIR)/, $(_OBJALL))


$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

calculator: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

squareroot: $(OBJ2)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

amortization: $(OBJ3)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)


all: $(OBJALL)

$(OBJ): | $(ODIR)
$(OBJ2): | $(ODIR)
$(OBJ3): | $(ODIR)

$(ODIR):
	mkdir -p $(ODIR)

.PHONY:	clean

clean: 
	rm -fr $(ODIR) calculator* squareroot* amortization* *~ $(IDIR)/*~ $(SDIR)/*~ log.txt

