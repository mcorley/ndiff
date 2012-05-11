//===--- DiffBlockCleanup.cpp  ----------------------------------------------===//
//
//                     The NDiff File Comparison Utility
//
//===----------------------------------------------------------------------===//
//
//  This file implements the DiffBlockCleanup interface.
//
//===----------------------------------------------------------------------===//

#include "DiffBlock.h"
#include "DiffBlockCleanup.h"
#include "Token.h"
#include <string>
#include <vector>

void DiffBlockCleanup::cleanupMerge(std::vector<DiffBlock>& script, 
                                    const std::vector<Token> &tokArray0,
                                    const std::vector<Token> &tokArray1) {
  // We want to try and combine consecutive diff blocks that are for the same
  // operation. To do this we can compare the cost of ouputing the original diff
  // blocks vs what it would cost to display a single diff consisting of all
  // changes together.
  if (script.size() == 1)
    return;

  std::vector<DiffBlock> merged;
  std::vector<DiffBlock>::iterator db(script.begin() + 1),
                                   end(script.end());
  for (; db != end; db++) {    
    DiffBlock prevdb = *(db - 1);
    if (prevdb.operation != (*db).operation) {
      merged.push_back(prevdb);
      continue;
    }

    switch ((*db).operation) {
      case INSERT: {
        int runStart = prevdb.inserted[0].file_pos;
        int runEnd = (*db).inserted[(*db).inserted.size() - 1].file_pos;
        (*db).inserted = mid(files[FILE1], runStart, runEnd - runStart + 1);
        if (!merged.empty() && merged.back() == prevdb)
          merged.pop_back();
        merged.push_back((*db));
        break;
      }
      case SUBST: {
        int runStart = prevdb.inserted[0].file_pos;
        int runEnd = (*db).inserted[(*db).inserted.size() - 1].file_pos;
        (*db).inserted = mid(files[FILE1], runStart, runEnd - runStart + 1);
        runStart = prevdb.deleted[0].file_pos;
        runEnd = (*db).deleted[(*db).deleted.size() - 1].file_pos;
        (*db).deleted = mid(files[FILE0], runStart, runEnd - runStart + 1);
        if (!merged.empty() && merged.back() == prevdb)
          merged.pop_back();
        merged.push_back((*db));
        break;
      }
      case DELETE: {
        int runStart = prevdb.deleted[0].file_pos;
        int runEnd = (*db).deleted[(*db).deleted.size() - 1].file_pos;
        (*db).deleted = mid(files[FILE0], runStart, runEnd - runStart + 1);
        if (!merged.empty() && merged.back() == prevdb)
          merged.pop_back();
        merged.push_back((*db));
        break;
      }
    }
  }
  script.clear();
  script = merged;
}

void DiffBlockCleanup::cleanupSemantic(std::vector<DiffBlock>& script, 
                                       const std::vector<Token> &tokArray0,
                                       const std::vector<Token> &tokArray1) {
  // ---TODO
}
