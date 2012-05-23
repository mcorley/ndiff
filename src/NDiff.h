//===--- NDiff.h - NDiff interface ----------------------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
// 
//===--------------------------------------------------------------------===//
//
// This file defines the Ndiff interface.
//
//===--------------------------------------------------------------------===//

#ifndef NDIFF_H
#define NDIFF_H

class Anchor;
class DiffBlock;
class Token;

#include <algorithm>
#include <list>
#include <string>
#include <vector>

/// NDiff - This class implements the ndiff file comparison algorithm.
class NDiff {
public:
  /// NDiff default constructor - Create a new NDiff instance.
  NDiff() {};

  /// Runs the ndiff algorithm on the files at sourcePath and targetpath.
  std::list<DiffBlock> computeDifference(
      const std::string &sourcePath, const std::string &targetpath);

  /// commonPrefix - Return the number of tokens common to the start of each
  ///                token stream.
  int commonPrefix(const std::vector<Token> &sourceTokenStream, 
                   const std::vector<Token> &targetTokenStream);

  /// commonSuffix - Return the number of tokens common to the end of each
  ///                token stream.
  int commonSuffix(const std::vector<Token> &sourceTokenStream, 
                   const std::vector<Token> &targetTokenStream);
  
  /// prettyOutput - 
  void prettyOutput(std::list<DiffBlock> &DBs);
private:
  /// compareBetweenAnchors - Use the anchors to extract runs of tokens we 
  ///                         wish to process with diff.
  std::list<DiffBlock> compareBetweenAnchors(
      const std::vector<Token> &sourceTokenStream, 
      const std::vector<Token> &targetTokenStream,
      const std::vector<Anchor> &anchVector);

  /// discardWhitespace
  std::vector<Token> discardWhitespace(const std::vector<Token> &tokenStream);

  /// insertWhitespace - Add whitespace inforamtion back into the edit script.
  std::list<DiffBlock> insertWhitespace(
      const std::list<DiffBlock> &DBs, 
      const std::vector<Token> &sourceTokenStream,
      const std::vector<Token> &targetTokenStream);
  
//===--------------------------------------------------------------------===//
// NDIFF PRIVATE STATIC HELPER FUNCTIONS 
//===--------------------------------------------------------------------===//

  /// mid - Returns a subvector that contains sequential tokens of a file, starting 
  /// at the specified position pos. Returns an empty vector when the postion equals 
  /// the file length.
  static inline std::vector<Token> mid(const std::vector<Token>& v, int pos) {
    return (pos == v.size()) ? std::vector<Token>() : 
      std::vector<Token>(v.begin() + pos, v.end());
  }

  /// mid - Returns a subvector that contains len sequential tokens of a file, starting 
  /// at the specified position pos. Returns an empty vector when the postion equals 
  /// the file length.
  static inline std::vector<Token> mid(const std::vector<Token> &v, int pos, int len) {
    return (pos == v.size()) ? std::vector<Token>() : 
      std::vector<Token>(v.begin() + pos, v.begin() + pos + len);
  }

  /// left - Returns a vector that contains the n leftmost tokens of the file. 
  /// The entire vector is returned if n is greater than size() or less than zero.
  static inline std::vector<Token> left(const std::vector<Token> &v, int n) {
    return (n < 0 || v.size() < n) ? v : std::vector<Token>(v.begin(), v.begin() + n);
  }

  /// right - Returns a vector that contains the n rightmost tokens of the file. 
  /// The entire vector is returned if n is greater than size() or less than zero.
  static inline std::vector<Token> right(const std::vector<Token> &v, int n) {
    return (n < 0 || v.size() < n) ? v : std::vector<Token>(v.end() - n, v.end());
  }

  /// indexOf - Searches f0 for the first occurrence of the sequence defined 
  /// by f1, and returns the index position to its first element. 
  static inline int indexOf(const std::vector<Token> &f0, 
                            const std::vector<Token> &f1) {
    std::vector<Token>::const_iterator i;
    i = std::search(f0.begin(), f0.end(), f1.begin(), f1.end());
    return (i == f0.end()) ? -1 : std::distance(f0.begin(), i);
  }

  /// indexOf - Searches f0 for the first occurrence of the sequence defined by f1, 
  /// starting at index pos and returns the index position to its first element.
  static inline int indexOf(const std::vector<Token> &f0, 
                            const std::vector<Token> &f1, 
                            int pos) {
    std::vector<Token>::const_iterator i;
    i = std::search(f0.begin() + pos, f0.end(), f1.begin(), f1.end());
    return (i == f0.end()) ? -1 : std::distance(f0.begin(), i);
  }
};

#endif // NDIFF_H
