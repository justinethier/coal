CC = gcc
CSTD = -std=c99
CFLAGS = $(CSTD)

COBJ = Lexer Parser Expression compiler
FILES = $(addsuffix .c, $(COBJ))
OBJS = $(addsuffix .o, $(COBJ))

clc: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o clc

vm: vm.c
	$(CC) $(CFLAGS) vm.c -o vm

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

.PHONY: clean
clean:
	rm -rf vm *.o *.yy.c a.out
