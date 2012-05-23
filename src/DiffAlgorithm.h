//===--- DiffAlgorithm.h - DiffAlgorithm interface ------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
//
//===--------------------------------------------------------------------===//
//
// This file defines the DiffAlgorithm interface.
//
//===----------------------------------------------------------------------===

#include "DiffBlock.h"
#include <list>
#include <string>
#include <vector>

class Token;

/// DiffAlgorithm - Wrapper class around the GNU diff file comparison utility.
class DiffAlgorithm {
public:
  DiffAlgorithm() {}
  ~DiffAlgorithm() {}

  /// Creates two temporary files where each line of the file holds one token. 
  /// When then execute the diff command with the popen function and parse the 
  /// results.
  std::list<DiffBlock> computeDifference(const std::vector<Token> &sourceTokenStream, 
                                         const std::vector<Token> &targetTokenStream);

  /// captureEqualities - The DiffBlocks returned by diff only represent the
  /// changes (insertions and deltions) that occured in the sourceTokenStream
  /// and the targetTokenStream. This method traverses the list of DiffBlocks
  /// and captures the missing equalities to provide a more complete result.
  std::list<DiffBlock> captureEqualities(
      const std::list<DiffBlock> &DBs,
      const std::vector<Token> &sourceTokenStream, 
      const std::vector<Token> &targetTokenStream);
private:
  /// Parse diffs.
  void processDiff(const std::string &changecmd, 
                   const std::vector<Token> &sourceTokenStream, 
                   const std::vector<Token> &targetTokenStream, 
                   std::list<DiffBlock> &DBs);

  /// Parse a normal format diff control string.  Return the type of the
  /// diff (ERROR if the format is bad).  All of the other important
  /// information is filled into to the structure pointed to by db, and
  /// the string pointer (whose location is passed to this routine) is
  /// updated to point beyond the end of the string parsed.  Note that
  /// only the ranges in the diff_block will be set by this routine.
  /// 
  /// If some specific pair of numbers has been reduced to a single
  /// number, then both corresponding numbers in the diff block are set
  /// to that number.  In general these numbers are interpreted as ranges
  /// inclusive, unless being used by the ADD or DELETE commands.  It is
  /// assumed that these will be special cased in a superior routine.   
  Operation processDiffControl(const std::string &changecmd, int ranges[2][2]);

  /// Skip whitespace.
  static inline const char *skipwhite(const char *s);

  /// Read a nonnegative line number from S, returning the address of the
  /// first character after the line number, and storing the number into
  /// PNUM. Return 0 if S does not point to a valid line number.
  static inline const char *readnum(const char *s, int *pnum);
};
