# Makefile for ndiff

SHELL = /bin/sh

CXX = g++
CXXPFLAGS = -Wall -g -O 
LEX = flex
LFLAGS = -p -8 -Ce
LIBS = -lfl
OBJECTS = AnchorAnalysis.o DiffAlgorithm.o Lexer.o NDiff.o \
	  SuffixArray.o TokenLexer.o LosslessOptimizer.o

ndiff: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

Lexer.o: Lexer.c
	$(CXX) $(CXXFLAGS) -o $@ -c $^

Lexer.c: Lexer.l
	$(LEX) $(LFLAGS) -o $@ $^

.PHONY: clean
clean:
	-rm -f ndiff ndiffl.c *.o

