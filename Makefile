CC = gcc
CSTD = -std=c99
CFLAGS = $(CSTD) -g

COBJ = Lexer Parser Expression compiler
FILES = $(addsuffix .c, $(COBJ))
OBJS = $(addsuffix .o, $(COBJ))

all: cyc cy

cyc: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o cyc

cy: vm.c vm.h
	$(CC) $(CFLAGS) vm.c -o cy

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

test: cyc vm
	./cyc test.src
	./cy test.bin

.PHONY: tags
tags:
	ctags -R

.PHONY: clean
clean:
	rm -rf *.o *.yy.c a.out Lexer.c Lexer.h Parser.c Parser.h *.bin tags cy cyc

