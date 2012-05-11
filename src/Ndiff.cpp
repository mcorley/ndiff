//===--- NDiff.cpp  ----------------------------------------------===//
//
//                     The NDiff File Comparison Utility
//
//===----------------------------------------------------------------------===//
//
//  This file implements the NDiff interface.
//
//===----------------------------------------------------------------------===//

#include "Anchor.h"
#include "DiffAlgorithm.h"
#include "DiffBlock.h"
#include "DiffBlockCleanup.h"
#include "NDiff.h"
#include "TokenLexer.h"
#include "Token.h"
#include <algorithm>
#include <string>
#include <vector>

void NDiff::computeDifference(const std::string &filename0, 
                              const std::string &filename1) {
  // Transform the stream of characters composing the two files we are comparing
  // into streams of tokens we can operate on.
  TokenLexer theTokenLexer;
  tokArray0 = theTokenLexer.tokenize(filename0);
  tokArray1 = theTokenLexer.tokenize(filename0);

  // We want to ignore whitespace characters for the remainder of the 
  // comparison algorithm, however we need them for later accuretly 
  // displaying the changes. These structures represents the files if 
  // we were to ignore whitespace information.
  std::vector<Token> ignoreWS0, ignoreWS1;
  std::vector<Token>::iterator Tok(tokArray0.begin()), E(tokArray0.end());
  for (; Tok != E; ++Tok) 
    if (!(*Tok).isWhitespace()) ignoreWS0.push_back((*Tok));
  for (Tok = tokArray1.begin(), E = tokArray1.end(); Tok != E; ++Tok)
    if (!(*Tok).isWhitespace()) ignoreWS1.push_back((*Tok));

  // Check for equality.
  std::vector<DiffBlock> blockList;
  if (ignoreWS0 == ignoreWS1) {
    return;
  }

  // Discard common prefix.
  int commonlength = commonPrefix(ignoreWS0, ignoreWS1);
  const std::vector<Token> commonprefix(left(ignoreWS0, commonlength));
  std::vector<Token> undiscarded0(mid(ignoreWS0, commonlength));
  std::vector<Token> undiscarded1(mid(ignoreWS1, commonlength));

  // Discard common suffix.
  commonlength = commonSuffix(undiscarded0, undiscarded1);
  const std::vector<Token> commonsuffix(right(undiscarded0, commonlength));
  undiscarded0 = left(undiscarded0, undiscarded0.size() - commonlength);
  undiscarded1 = left(undiscarded1, undiscarded1.size() - commonlength);

  // Now we attempt to identify runs of tokens which remain unchanged 
  // between the two files.
  AnchorAnalysis anchorAnalyzer;
  anchorList = anchorAnalyzer.findAnchors(undiscarded0, undiscarded1);

  // If we didn't find any anchors, simply diff the two files normally.
  if (anchorList.empty()) {
    DiffAlgorithm theDiffAlgorithm;
    blockList = theDiffAlgorithm.computeDifference(undiscarded0, undiscarded1);
    insertWhitespace(blockList, files);
    printNormalScript(blockList);
    return;
  }

  // Otherwise, we use the anchors to split the token streams into regions. 
  // A region is labeled as either being unchanged, or different in which case
  // there exists an insertion, deletion, or substution.
  blockList(generateEditScript(anchList, undiscarded0, undiscarded1));
  printNormalScript(blockList);
}

std::vector<std::vector<DiffBlock> > NDiff::generateEditScript(
      const std::vector<Anchor> &anchList,
      const std::vector<Token> &tokArray0, 
      const std::vector<Token> &tokArray1) {
  // Use the anchors to extract runs of tokens we wish to process with diff.
  // e.g.
  //      ----x----         ------y------        ---z---
  // [ |  |\\|\\|\\|  |  |  |\\|\\|\\|\\|  |  |  |\\|\\|  ]
  // 0    x1                y1                   z1       m
  //         ----x----               ------y------        ---z---
  // [ |  |  |\\|\\|\\|  |  |  |  |  |\\|\\|\\|\\|  |  |  |\\|\\|  ]
  // 0       x2                      y2                   z2       n
  //
  // With this layout, we want to compare four different runs of tokens.
  // From the first file, we get the intervals 
  //    [0, x1), [x1+len(x), y1), [y1+len(y), z1), [z1+len(z), m)
  // Similarly, from the second file, we get the intervals 
  //    [0, x2), [x2+len(x), y2), [y2+len(y), z2), [z2+len(z), n)
  DiffAlgorithm theDiffAlgorithm; 
  std::vector<std::vector<DiffBlock> > script;
  int begin0, begin1, end0, end1;
  for (int i = 0, e = anchList.size(); i <= e; ++i) {
    if (i == 0) {
      begin0 = 0;
      begin1 = 0;
      end0 = anchList[i].index0;
      end1 = anchList[i].index1;
    } else if (i == anchList.size()) {
      begin0 = anchList[i-1].index0 + anchList[i-1].length;
      begin1 = anchList[i-1].index1 + anchList[i-1].length;
      end0 = tokArray0.size();
      end1 = tokArray1.size();
    } else {
      begin0 = anchList[i-1].index0 + anchList[i-1].length;
      begin1 = anchList[i-1].index1 + anchList[i-1].length;
      end0 = anchList[i].index0;
      end1 = anchList[i].index1;
    }
    std::vector<std::vector<Token> > runs_to_diff(2);
    runs_to_diff[0] = mid(undiscarded[0], begin0, end0 - begin0);
    runs_to_diff[1] = mid(undiscarded[1], begin1, end1 - begin1);
    std::vector<DiffBlock> blocks = 
      theDiffAlgorithm.computeDifference(runs_to_diff);
    script.push_back(blocks);
  }
  return script;
}

void NDiff::printNormalScript(std::vector<DiffBlock> blockList) {
  vector<DiffBlock>::iterator bptr(blockList.begin()), end(blockList.end());
  for (; bptr != end; ++bptr) {
    DiffBlock db = *bptr;
    switch (db.operation) {
      case INSERT: {
        cout << db.inserted[0].line << "a" << db.inserted.size() << endl;
        cout << "> ";
        const int inserted = db.inserted.size();
        for (int i = 0; i < inserted; ++i) {
          cout << db.inserted[i].chars;
          if (db.inserted[i].chars.find("\n") != string::npos) 
            cout << "> ";
        }
        break;
      }
      case SUBST: {
        cout << db.deleted[0].line << "c" << db.deleted.size() << endl;
        cout << "< ";
        const int deleted = db.deleted.size();
        for (int i = 0; i < deleted; ++i) {
          cout << db.deleted[i].chars;
          if (db.deleted[i].chars.find("\n") != string::npos) 
            cout << "< ";
        }
        cout << "\n---\n";
        cout << "> ";
        const int inserted = db.inserted.size();
        for (int i = 0; i < inserted; ++i) {
          cout << db.inserted[i].chars;
          if (db.inserted[i].chars.find("\n") != string::npos) 
            cout << "> ";
        }       
        break;
      }
      case DELETE: {
        cout << db.deleted[0].line << "d" << db.deleted.size() << endl;
        cout << "< ";
        const int deleted = db.deleted.size();
        for (int i = 0; i < deleted; ++i) {
          cout << db.deleted[i].chars;
          if (db.deleted[i].chars.find("\n") != string::npos) 
            cout << "< ";
        }
        break;
      }
      default:
        return; // Bad format 
    }
    cout << endl << endl;
  }
}

void NDiff::insertWhitespace(std::vector<DiffBlock> &script, 
                             const vector<vector<Token> > &files) {
  vector<DiffBlock>::iterator db(script.begin()), end(script.end());
  for (; db != end; db++) {
    switch ((*db).operation) {
      case INSERT: {
        int runStart = (*db).inserted[0].file_pos;
        int runEnd = (*db).inserted[(*db).inserted.size() - 1].file_pos;
        (*db).inserted = mid(files[FILE1], runStart, runEnd - runStart + 1);
        break;
      }
      case SUBST: {
        int runStart = (*db).inserted[0].file_pos;
        int runEnd = (*db).inserted[(*db).inserted.size() - 1].file_pos;
        (*db).inserted = mid(files[FILE1], runStart, runEnd - runStart + 1);
        runStart = (*db).deleted[0].file_pos;
        runEnd = (*db).deleted[(*db).deleted.size() - 1].file_pos;
        (*db).deleted = mid(files[FILE0], runStart, runEnd - runStart + 1);
        break;
      }
      case DELETE: {
        int runStart = (*db).deleted[0].file_pos;
        int runEnd = (*db).deleted[(*db).deleted.size() - 1].file_pos;
        (*db).deleted = mid(files[FILE0], runStart, runEnd - runStart + 1);
        break;
      }
    }
  }
}
