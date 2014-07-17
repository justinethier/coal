CC = gcc
CSTD = -std=c99
CFLAGS = $(CSTD) -g

COBJ = Lexer Parser Expression compiler
CFILES = $(addsuffix .c, $(COBJ))
COBJS = $(addsuffix .o, $(COBJ))

VMOBJ = vm
VMFILES = $(addsuffix .c, $(VMOBJ))
VMOBJS = $(addsuffix .o, $(VMOBJ))

COMOBJ = util
COMFILES = $(addsuffix .c, $(COMOBJ))
COMOBJS = $(addsuffix .o,  $(COMOBJ))


.PHONY: all
all: $(CFILES) $(VMFILES) $(COMFILES)
	$(MAKE) $(COBJS) $(VMOBJS) $(COMOBJS)
	$(CC) $(CFLAGS) -o cyc $(COBJS) $(COMOBJS) $(LIBS)
	$(CC) $(CFLAGS) -o cy $(VMOBJS) $(COMOBJS) $(LIBS)

Lexer.c: Lexer.l
	flex Lexer.l

Parser.c: Parser.y Lexer.c
	bison Parser.y

##wc: $(FILES)
##  $(MAKE) $(SOBJ)
##  $(MAKE) $(OBJS)
##  $(CXX) $(CXXFLAGS) -o $(EXE) $(OBJS) parser.o lexer.o $(LIBS)
##
##parser: mc_parser.yy
##  bison -d -v mc_parser.yy
##  $(CXX) $(CXXFLAGS) -c -o parser.o mc_parser.tab.cc
#
#lexer: lisp_lexer.l
#	flex --outfile=lisp_lexer.yy.c  $<
##	$(CC)  $(CFLAGS) -c lisp_lexer.yy.c -o lexer.o
#	$(CC)  $(CFLAGS) lisp_lexer.yy.c -lfl

test: all
	./cyc test.src
	./cy test.bin

.PHONY: tags
tags:
	ctags -R

.PHONY: clean
clean:
	rm -rf *.o *.yy.c a.out Lexer.c Lexer.h Parser.c Parser.h *.bin tags cy cyc

