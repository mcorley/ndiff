//===--- TokenLexer.h - TokenLexer interface ------------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
//
//===--------------------------------------------------------------------===//
//
// This file defines the TokenLexer interface.
//
//===----------------------------------------------------------------------===

#ifndef TOKENLEXER_H
#define TOKENLEXER_H

#include <map>
#include <string>
#include <vector>

class Token;

/* Flex variables. */
#define TOK_WS 255
extern int yylineno;
extern int yyleng;
extern const char *yytext;
extern const char *yyfilename;
extern int yylex();
extern FILE *yyin;

/// TokenLexer - This implements a lexer that returns tokens from a character
///              stream.
class TokenLexer {  
  std::map<std::string, int> tokenHashMap;
  int nextHashValue;
public:
  /// TokenLexer constructor - Create a new TokenLexer object with reserving
  ///                          the default number of sentinel characters.
  TokenLexer() : nextHashValue(2) {}

  /// TokenLexer constructor - Create a new TokenLexer object with reserving
  ///                          the specified number of sentinel characters.
  explicit TokenLexer(int sentinels) : nextHashValue(sentinels) {}

  /// tokenize - Convert the stream of characters corresponding to the filename
  ///            into a stream of tokens. Reduce the tokens to a string of hashes 
  ///            where each Unicode character represents one token.
  std::vector<Token> tokenize(const std::string &filename);
};

#endif // TOKENLEXER_H
