//===--- TokenLexer.cpp - Lex from a token stream -------------------------===//
//
//                     The NDiff File Comparison Utility
//
//===----------------------------------------------------------------------===//
//
// This file implements the TokenLexer interface.
//
//===------------------------------------------------------------------------===

#include "Token.h"
#include "TokenLexer.h"
#include <cstdio>

std::vector<Token> TokenLexer::tokenize(const std::string &filename) {
  std::vector<Token> tokenStream;
  if (!(yyin = fopen(filename.c_str(), "r")))
    return tokenStream;

  // Initilize location data.
  int col = 0, line = 1; 
  yylineno = 1;

  for (int sym; sym = yylex();) {

    // Assign a hash value if not whitespace.
    int hashVal = -1;
    if (sym != TOK_WS) {
      std::map<std::string, int>::iterator 
        i(tokenHashMap.find(yytext)), e(tokenHashMap.end());
      if (i == e) {
        hashVal = nextHashValue++;
        tokenHashMap.insert(std::pair<std::string, int>(yytext, hashVal));          
      } else
        hashVal = (*i).second;        
    }

    // Update location data.
    const int offset = tokenStream.size();
    if (line != yylineno) { col = 1; ++line; } 
    else { ++col; }    

    // Create a Token object with the data for this lexed token.
    Token tok(yytext, hashVal, offset, line, col);

    // Set appropriate flags.
    if (!tokenStream.empty() && tokenStream.back().isWhitespace()) 
      tok.setFlagValue(Token::leadingSpace, true);
    if (col == 1) tok.setFlagValue(Token::startOfLine, true);
    if (sym == TOK_WS) tok.setFlagValue(Token::whitespace, true);

    // Add the token 
    tokenStream.push_back(tok);    
  }
  fclose(yyin);	
  return tokenStream;
}
