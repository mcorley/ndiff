//===--- LosslessOptimizer.h - LosslessOptimizer interface ----*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
// 
//===--------------------------------------------------------------------===//
//
// This file defines the LosslessOptimizer interface.
//
//===--------------------------------------------------------------------===//

class DiffBlock;

#include <list>

/// LosslessOptimizer - This class implements various methods for optimizing
/// the DiffBlocks returned from a file comparison to be used for human use.
/// The goal is to provide more meaningful divisions while still allowing the 
/// exact original data to be later reconstructed.
class LosslessOptimizer {
public:
  /// LosslessOptimizer default constructor - Create a new LosslessOptimizer 
  /// instance.
  LosslessOptimizer() {}
  ~LosslessOptimizer() {}

  /// splitCoincidentalEqualities - Reduce the number of edits by eliminating 
  /// semantically trivial equalities. This method passes over the data looking 
  /// for equalities that are smaller than or equal to the insertions and 
  /// deletions on both sides of them. When such an equality is found, it is 
  /// split into a deletion and an insertion.
  void splitCoincidentalEqualities(std::list<DiffBlock> &DBs);

  /// mergeCoincidentalEqualities - Passes over the DiffBlocks and reorders and 
  /// mergees like edit sections. Consecutive equalities are also merged. Any 
  /// edit section can move as long as it doesn't cross an equality. 
  void mergeCoincidentalEqualities(std::list<DiffBlock> &DBs);

  void mergeMore(std::list<DiffBlock> &DBs);
};
