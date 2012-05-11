//===--- NDiff.h - NDiff interface ----------------------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
// 
//
//===--------------------------------------------------------------------===//
//
// This file defines the Ndiff interface.
//
//===----------------------------------------------------------------------===

#ifndef NDIFF_H
#define NDIFF_H

/* Flex variables. */
extern int yylineno;
extern int yyleng;
extern const char *yytext;
extern const char *yyfilename;
extern int yylex();
extern FILE *yyin;

class NDiff {
  /// The list of Anchors found from the files being compared.
  std::vector<Anchor> anchorList;

  /// The list of DiffBlocks created by the files being compared.
  std::vector<DiffBlock> blockList;

  /// The token stream representing the left file in the comparison.
  std::vector<Token> tokArray0;

  /// The token stream representing the right file in the comparison.
  std::vector<Token> tokArray1;
public:
  /// NDiff default constructor - Create a new NDiff instance.
  NDiff();

  /// anchorList - Return the list of Anchors found between the two files
  ///              currently beeing compared.
  std::vector<Anchor> anchorList() const;

  /// files - Return the list of files begin compared. (Default is two files).
  ///         Each file is represetned as a sequence of Tokens.
  std::vector<std::vector<Token> > files() const;

  /// blockList - Return the list of DiffBlocks created by the two files
  ///             currently being compared.
  std::vector<DiffBlock> blockList() const;

  /// Runs the ndiff algorithm on the files located at Filename0 and Filename1.
  void computeDifference(const std::string &Filename0, 
                         const std::string &Filename1);

  /// generateEditScript - Use the anchors to extract runs of tokens we wish 
  ///                      to process with diff.
  std::vector<std::vector<DiffBlock> > generateEditScript(
      const std::vector<Anchor> &anchList,
      const std::vector<Token> &tokArray0, 
      const std::vector<Token> &tokArray1);

  /// commonPrefix - Return the number of tokens common to the start of each
  ///                token stream.
  int commonPrefix(const std::vector<Token> &tokArray0, 
                   const std::vector<Token> &tokArray1);

  /// commonSuffix - Return the number of tokens common to the end of each
  ///                token stream.
  int commonSuffix(const std::vector<Token> &tokArray0, 
                   const std::vector<Token> &tokArray1);

  /// printNormalScript - Print the blocklist to the terminal.
  void printNormalScript(std::vector<DiffBlock> blockList);

  /// insertWhitespace - Add whitespace inforamtion back into the edit script.
  void insertWhitespace(std::vector<DiffBlock> &script, 
                        const std::vector<std::vector<Token> > &files);
};

#endif // NDIFF_H
