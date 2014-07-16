CC = gcc
CSTD = -std=c99
CFLAGS = $(CSTD) -g

COBJ = Lexer Parser Expression compiler
FILES = $(addsuffix .c, $(COBJ))
OBJS = $(addsuffix .o, $(COBJ))

all: clc vm

clc: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o clc

vm: vm.c vm.h
	$(CC) $(CFLAGS) vm.c -o vm

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

test: clc vm
	./clc
	ls -l out.bin

.PHONY: clean
clean:
	rm -rf vm *.o *.yy.c a.out Lexer.c Lexer.h Parser.c Parser.h clc out.bin

