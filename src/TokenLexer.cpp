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
#include <map>
#include <string>
#include <vector>

vector<Token> TokenLexer::tokenize(const std::string &filename);
{
  int equiv, sym;
  std::vector<Token> tokArray;
  std::map<std::string, int>::iterator eq;

  if ((yyin = fopen(filename, "r"))) {
    int line_pos = 0, prev_line = 1;
    // Walk the files, pulling out a substring for each token. and
    // simultaneously compute the equivalence class for each token.
    for (yylineno = 1;  sym = yylex(); ) {
      if (sym == TOK_WS) {
        equiv = -1;
      } else if ((eq = equivclasses.find(yytext)) != equivclasses.end()) {
        equiv = (*eq).second;
      } else { // Create a new equivalence class.
        equiv = equivs_index++;
        equivclasses.insert(pair<string,int>(yytext,equiv));
      }
      int file_pos = tokArray.size();
      if (prev_line == yylineno) {
        line_pos++;
      } else { 
        prev_line++;
        line_pos = 1;
      }
      tokArray.push_back(Token(yytext, 
                               yylineno, 
                               equiv, 
                               file_pos, 
                               line_pos));
    }
    fclose(yyin);	
  }
  return tokArray;
}
