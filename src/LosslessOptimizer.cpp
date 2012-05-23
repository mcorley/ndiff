//===--- LosslessOptimizer.cpp  -------------------------------------------===//
//
//                     The NDiff File Comparison Utility
//
//===----------------------------------------------------------------------===//
//
//  This file implements the LosslessOptimizer interface.
//
//===----------------------------------------------------------------------===//

#include "DiffBlock.h"
#include "LosslessOptimizer.h"

void LosslessOptimizer::splitCoincidentalEqualities(std::list<DiffBlock> &DBs) {
  if (DBs.empty())
    return;
  std::list<DiffBlock> equalities; // Stack of equalities.
  std::vector<Token> lastequality; // Always equal to equalities.back().tokens()
  // Number of tokens that changed prior to the equality.
  int insertsBefore = 0;
  int deletesBefore = 0;
  // Number of tokens that changed after the equality.
  int insertsAfter = 0;
  int deletesAfter = 0;
  std::list<DiffBlock>::iterator thisDB(DBs.begin()), end(DBs.end());
  for (; thisDB != end; ++thisDB) {
    if ((*thisDB).getOperation() == EQUAL) {
      equalities.push_back(*thisDB);
      insertsBefore = insertsAfter;
      deletesBefore = deletesAfter;
      insertsAfter = 0;
      deletesAfter = 0;
      lastequality = (*thisDB).getTokens();
    } else {
      // An insertion or deletion.
      if ((*thisDB).getOperation() == DELETE) {
        deletesAfter += (*thisDB).getTokens().size();
      } else {
        insertsAfter += (*thisDB).getTokens().size();
      }
      // Eliminate an equality that is smaller or equal to the edits on both
      // sides of it.
      if (!lastequality.empty() && 
          (lastequality.size() <= std::max(insertsBefore, deletesBefore)) && 
          (lastequality.size() <= std::max(insertsAfter, deletesAfter))) {
        // Walk back to offending equality.
        while (*thisDB != equalities.back()) {
          --thisDB;
        }
        // Replace equality with a delete.
        thisDB = DBs.erase(thisDB);
        thisDB = DBs.insert(thisDB, DiffBlock(DELETE, lastequality));

        // Insert a DB corresponding to an insert.
        thisDB = DBs.insert(thisDB, DiffBlock(INSERT, lastequality));

        equalities.pop_back(); // Throw away the equality we just deleted.
        if (!equalities.empty()) {
          // Throw away the previous equality (it needs to be reevaluated).
          equalities.pop_back();
        }
        if (equalities.empty()) {
          // There are no previous equalities, walk back to the start.
          while (thisDB != DBs.begin()) {
            --thisDB;
          }
        } else {
          // There is a safe equality we can fall back to.
        }

        insertsBefore = 0;  // Reset the counters.
        deletesBefore = 0;
        insertsAfter = 0;
        deletesAfter = 0;
        lastequality.clear();      
      }
    }
  }
}

void LosslessOptimizer::mergeCoincidentalEqualities(std::list<DiffBlock> &DBs) {
  if (DBs.empty())
    return;
  // Add a dummy entry at the end of the DBs.
  DBs.push_back(DiffBlock(EQUAL, std::vector<Token>()));
  int deleteCount = 0;
  int insertCount = 0;
  std::vector<Token> deleted, inserted;
  deleted.reserve(2048);
  inserted.reserve(2048);
  std::list<DiffBlock>::iterator prevEqual(DBs.end());
  std::list<DiffBlock>::iterator thisDB(DBs.begin()), end(DBs.end());
  for (; thisDB != end; ++thisDB) {
    std::vector<Token> toks((*thisDB).tokens());
    switch ((*thisDB).getOperation()) {
      case DELETE:
        ++deleteCount;
        deleted.insert(deleted.end(), toks.begin(), toks.end());
        prevEqual = end;
        break;
      case INSERT:
        ++insertCount;
        inserted.insert(inserted.end(), toks.begin(), toks.end());
        prevEqual = end;
        break;
      case EQUAL:
        if (deleteCount + insertCount > 1) {
          bool bothTypes = (deleteCount != 0) && (insertCount != 0);
          // Delete the offending records.
          while (deleteCount-- > 0) {
            --thisDB;
            thisDB = DBs.erase(thisDB);
          }
          while (insertCount-- > 0) {
            --thisDB;
            thisDB = DBs.erase(thisDB);
          } 
          if (bothTypes) {/*
            // Factor out any common prefixies.
            int commonlength = commonPrefix(inserted, deleted);
            if (commonlength != 0) {
              std::vector<Token> v = left(inserted, commonlength);
              std::vector<Token>::iterator i((*thisDB).tokens().end());
              if (thisDB != DBs.begin()) {
                --thisDB;
                (*thisDB).tokens().insert(i, v.begin(), v.end());
                ++thisDB;
              } else {
                DBs.insert(thisDB, DiffBlock(EQUAL, v));
              }
              deleted = mid(deleted, commonlength);
              inserted = mid(inserted, commonlength);
            }
            // Factor out any common suffixies.
            commonlength = commonSuffix(inserted, deleted);
            if (commonlength != 0) {
              ++thisDB;
              std::vector<Token> v = mid(inserted, inserted.size() - commonlength);
              v.insert(v.end(), (*thisDB).getTokens().begin(), (*thisDB).getTokens().end());
              (*thisDB).tokens() = v;
              inserted = left(inserted, inserted.size() - commonlength);
              deleted = left(deleted, deleted.size() - commonlength);
              --thisDB;
            }*/
          }
          // Insert the merged records.
          if (!deleted.empty()) {
            thisDB = DBs.insert(thisDB, DiffBlock(DELETE, deleted));
          }
          if (!inserted.empty()) {
            thisDB = DBs.insert(thisDB, DiffBlock(INSERT, inserted));
          }
          // Step forward to the equality.
          if (thisDB != DBs.end()) ++thisDB;

        } else if (prevEqual != end) {
          // TODO - Merge this equality with the previous one.
        }
        insertCount = 0;
        deleteCount = 0;
        deleted.clear();
        inserted.clear();
        prevEqual = thisDB;
        break;
    }
  }
  if (DBs.back().tokens().empty())
    DBs.pop_back();  // Remove the dummy entry at the end.
}

void LosslessOptimizer::mergeMore(std::list<DiffBlock> &DBs) {
  if (DBs.empty())
    return;

  std::vector<Token> deleted, inserted;
  int insertCount = 0;
  int deleteCount = 0;
  int equalCount = 0;
  int changed = 0;
  int unchanged = 0;
  int currentLine = 1;

  std::list<DiffBlock>::iterator thisDB(DBs.begin()), end(DBs.end());
  for (; thisDB != end; ++thisDB) {

    std::vector<Token> toks = (*thisDB).getTokens();
    Operation op = (*thisDB).getOperation();

    if (toks.empty()) {
      if (op == DELETE) ++deleteCount;
      else if (op == INSERT) ++insertCount;
      else ++equalCount;
      continue;
    }

    if (currentLine == toks.front().getLine()) {
      if (op == DELETE) {
        ++deleteCount;
        changed += toks.size();
        deleted.insert(deleted.end(), toks.begin(), toks.end());
      } else if (op == INSERT) {
        ++insertCount;
        changed += toks.size();
        inserted.insert(inserted.end(), toks.begin(), toks.end());
      } else { // op == EQUAL
        ++equalCount;
        unchanged += toks.size();
        deleted.insert(deleted.end(), toks.begin(), toks.end());
        inserted.insert(inserted.end(), toks.begin(), toks.end());
      }
    } else {
      currentLine = toks.front().getLine();
      const int whole = unchanged + changed;
      if (whole * 0.75 <= changed) {
        // Delete the offending records.
        while (deleteCount-- > 0) {
          --thisDB;
          thisDB = DBs.erase(thisDB);
        }
        while (insertCount-- > 0) {
          --thisDB;
          thisDB = DBs.erase(thisDB);
        }
        while (equalCount-- > 0) {
          --thisDB;
          thisDB = DBs.erase(thisDB);
        }
        // Insert the merged records.
        if (!deleted.empty()) {
          thisDB = DBs.insert(thisDB, DiffBlock(DELETE, deleted));
        }
        if (!inserted.empty()) {
          thisDB = DBs.insert(thisDB, DiffBlock(INSERT, inserted));
        }
      }
      insertCount = 0;
      deleteCount = 0;
      equalCount = 0;
      changed = 0;
      unchanged = 0;
      deleted.clear();
      inserted.clear();
    }
  }
}
