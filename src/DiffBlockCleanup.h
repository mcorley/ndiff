//===--- DiffBlockCleanup.h - DiffBlockCleanup interface ------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
// 
//
//===--------------------------------------------------------------------===//
//
// This file defines the DiffBlockCleanup interface.
//
//===----------------------------------------------------------------------===

#ifndef DIFFBLOCKCLEANUP_H
#define DIFFBLOCKCLEANUP_H

class DiffBlockCleanup {
public:
  /// DiffBlockCleanup constructor - Create a new DiffBlockCleanup object.
  DiffBlockCleanup() {}

  /// cleanupMerge - Reorder and merge like edit sections.  Merge equalities.
  ///                Any edit section can move as long as it doesn't cross an 
  ///                equality.
  void cleanupMerge(std::vector<DiffBlock>& script, 
                    const std::vector<Token> &tokArray0,
                    const std::vector<Token> &tokArray1);

  /// cleanupSemantic - Look for single edits surrounded on both sides by 
  ///                   equalities which can be shifted sideways to align the 
  ///                   edit to a word boundary.
  void cleanupSemantic(std::vector<DiffBlock>& script, 
                       const std::vector<Token> &tokArray0,
                       const std::vector<Token> &tokArray1);
};

#endif // DIFFBLOCKCLEANUP_H
