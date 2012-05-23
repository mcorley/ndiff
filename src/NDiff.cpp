//===--- NDiff.cpp  -------------------------------------------------------===//
//
//                     The NDiff File Comparison Utility
//
//===----------------------------------------------------------------------===//
//
//  This file implements the NDiff interface.
//
//===----------------------------------------------------------------------===//

#include "Anchor.h"
#include "AnchorAnalysis.h"
#include "DiffAlgorithm.h"
#include "DiffBlock.h"
#include "LosslessOptimizer.h"
#include "NDiff.h"
#include "TokenLexer.h"
#include "Token.h"
#include <cstdio>

// Main Driver
int main(int argc, char *argv[]) {
  NDiff ndiff;
  std::list<DiffBlock> DBs;
  DBs = ndiff.computeDifference(std::string(argv[1]), std::string(argv[2]));
  return 0;
}

// This method is the driver for the ndiff comparison algorithm. 
std::list<DiffBlock> NDiff::computeDifference(
    const std::string &sourcePath, const std::string &targetpath) {  
  // The first step is to divide the files into meaningful units that 
  // we can operate on and compare against.
  TokenLexer theTokenLexer;
  const std::vector<Token> lexedSourceTokStream(theTokenLexer.tokenize(sourcePath));
  const std::vector<Token> lexedTargetTokStream(theTokenLexer.tokenize(targetpath));
 
  // Discard tokens from one stream that have matches in the other stream. A 
  // token which is discarded will not be considered by the actual comparison 
  // algorithm; it will be as if that token were not in either stream. The 
  // Token's `offset' value maps virtual indexes (which don't count the 
  // discarded tokens) into real indexes numbers; this is how the actual 
  // comparison algorithm produces results that are comprehensible when the 
  // discarded tokens are counted.
  //
  // When we discard runs of tokens, we also mark them as EQUALS so that they 
  // can be considered in the output.
  std::vector<Token> sourceTokenStream, targetTokenStream;

  // Discard all white space.
  sourceTokenStream = discardWhitespace(lexedSourceTokStream);
  targetTokenStream = discardWhitespace(lexedTargetTokStream);
  
  // Check for equality.
  std::list<DiffBlock> DBs;
  if (sourceTokenStream == targetTokenStream) {
    if (!sourceTokenStream.empty()) {
      DBs.push_back(DiffBlock(EQUAL, lexedSourceTokStream));
    }
    return DBs;
  }

  // Discard common prefix.
  int commonlength = commonPrefix(sourceTokenStream, targetTokenStream);
  const std::vector<Token> commonprefix(left(sourceTokenStream, commonlength));
  sourceTokenStream = mid(sourceTokenStream, commonlength);
  targetTokenStream = mid(targetTokenStream, commonlength);

  // Discard common suffix.
  commonlength = commonSuffix(sourceTokenStream, targetTokenStream);
  const std::vector<Token> commonsuffix(right(sourceTokenStream, commonlength));
  sourceTokenStream = left(sourceTokenStream, sourceTokenStream.size() - commonlength);
  targetTokenStream = left(targetTokenStream, targetTokenStream.size() - commonlength);

  // Find long common sequences of tokens interspersed with 
  // groups of differing tokens. The idea here is to match up these long 
  // sequences between the two streams, and by then comparing the groups of 
  // differing tokens that line up we can yield a tighter result from any
  // longest common subsequence based difference algorithm.
  AnchorAnalysis anchorAnalyzer;
  const std::vector<Anchor> anchors(
      anchorAnalyzer.findAnchors(sourceTokenStream, targetTokenStream));
  if (anchors.empty()) {
    // Normal token-based diff. Run a difference algorithm on the
    // sourceTokenStream and targetTokenStream.
    DiffAlgorithm diff;
    DBs = diff.computeDifference(sourceTokenStream, targetTokenStream);
  } else {
    // Run a difference algorithm on the groups of differing tokens that line up
    // between anchors.
    DBs = compareBetweenAnchors(sourceTokenStream, targetTokenStream, anchors);
  }  

  // Restore the prefix and suffix.
  DBs.push_front(DiffBlock(EQUAL, commonprefix));
  DBs.push_back(DiffBlock(EQUAL, commonsuffix));
  
  // Optimize the output.
  LosslessOptimizer theOptimizer;
  theOptimizer.splitCoincidentalEqualities(DBs);
  theOptimizer.mergeCoincidentalEqualities(DBs);

  // Restore whitespace information from the original lexed token streams.
  DBs = insertWhitespace(DBs, lexedSourceTokStream, lexedTargetTokStream);
  prettyOutput(DBs);
  return DBs;
}

std::vector<Token> NDiff::discardWhitespace(const std::vector<Token> &tokenStream) {
  const int size = tokenStream.size();
  std::vector<Token> result;
  result.reserve(size);
  for (int i = 0; i < size; ++i)
    if (!tokenStream[i].isWhitespace())
      result.push_back(tokenStream[i]);
  return result;
}

int NDiff::commonPrefix(const std::vector<Token> &sourceTokenStream, 
                        const std::vector<Token> &targetTokenStream) {
  const int e = std::min(sourceTokenStream.size(), targetTokenStream.size());
  for (int i = 0; i < e; ++i) 
    if (sourceTokenStream[i] != targetTokenStream[i]) 
      return i; 
  return e;
}

int NDiff::commonSuffix(const std::vector<Token> &sourceTokenStream, 
                        const std::vector<Token> &targetTokenStream) {
  const int m = sourceTokenStream.size(), n = targetTokenStream.size();
  const int e = std::min(m, n);
  for (int i = 1; i <= e; ++i) 
    if (sourceTokenStream[m - i] != targetTokenStream[n - i])
      return i - 1;  
  return e;
}

std::list<DiffBlock> NDiff::compareBetweenAnchors(
    const std::vector<Token> &sourceTokenStream, 
    const std::vector<Token> &targetTokenStream,
    const std::vector<Anchor> &anchVector) {
  // Cache the anchor and token stream lengths to prevent multiple calls.
  const int sourceStreamSize = sourceTokenStream.size();
  const int targetStreamSize = targetTokenStream.size();
  const int anchVecLength = anchVector.size();

  DiffAlgorithm diff; 
  std::list<DiffBlock> DBs;  

  for (int i = 0; i <= anchVecLength; ++i) {
    // Compute the offsets in the token streams corrsonding to groups of 
    // differing tokens that line up between anchors. We need the offset marking
    // the start index and the end index of the sequence in each stream. 
    // The 2x2 array offset is defined as follows:
    //    offset[0][0] = offset to start from in the source stream
    //    offset[1][0] = offset to start from in the target stream
    //    offset[0][1] = offset to end from in the source stream
    //    offset[1][1] = offset to end from in the target stream
    int offset[2][2];

    // We need to distinguish between three different cases when entering
    // offset data. The first and last anchors are special cases where as 
    // all anchors wedged in the middle can be treated the same.
    if (i == 0) {
      offset[0][0] = 0;
      offset[1][0] = 0;
      offset[0][1] = anchVector[i].sourceIdx();
      offset[1][1] = anchVector[i].targetIdx();
    } else if (i == anchVecLength) {
      offset[0][0] = anchVector[i-1].sourceIdxEnd();
      offset[1][0] = anchVector[i-1].targetIdxEnd();
      offset[0][1] = sourceStreamSize;
      offset[1][1] = targetStreamSize;
    } else {
      offset[0][0] = anchVector[i-1].sourceIdxEnd();
      offset[1][0] = anchVector[i-1].targetIdxEnd();
      offset[0][1] = anchVector[i].sourceIdx();
      offset[1][1] = anchVector[i].targetIdx();
    }

    std::vector<Token> fromTokens = mid(sourceTokenStream, 
        offset[0][0], offset[0][1] - offset[0][0]);
    std::vector<Token> toTokens = mid(targetTokenStream, 
        offset[1][0], offset[1][1] - offset[1][0]);
    std::list<DiffBlock> result = diff.computeDifference(fromTokens, toTokens);
    DBs.insert(DBs.end(), result.begin(), result.end());

    // We mark the anchors as EQUAL so they are considered in the output. Since
    // our loop needs to consider after the last anchor, we need to check for
    // this case before trying to acces any anchor data.
    if (i < anchVecLength) {
      const int idx = anchVector[i].sourceIdx();
      const int len = anchVector[i].length();
      DBs.push_back(DiffBlock(EQUAL, mid(sourceTokenStream, idx, len)));
    }
  }

  return DBs;
}

std::list<DiffBlock> NDiff::insertWhitespace(
    const std::list<DiffBlock> &DBs, 
    const std::vector<Token> &sourceTokenStream,
    const std::vector<Token> &targetTokenStream) {
  std::list<DiffBlock> result;
  std::list<DiffBlock>::const_iterator i(DBs.begin()), e(DBs.end());
  for (; i != e; ++i) {
    const DiffBlock &DB = *i;
    if (DB.getTokens().empty())
      continue;

    // Each token in the DB's token vector maps to a token in the token stream 
    // according to its lexedOffset field. Token sequences in the diff blocks
    // have all whitespace data squeezed out and here is where we add it back.
    const int a = DB.getTokens().front().lexedOffset();
    const int b = DB.getTokens().back().lexedOffset();
    const int len = b - a + 1;

    if (len <= 0) 
      continue;

    // Token data for insertions comes from the targetTokenStream. For deletions
    // and equalities, the token data comes from the sourceTokenStream.
    const Operation op = DB.getOperation();
    std::vector<Token> toks = (op == INSERT) ? 
      mid(targetTokenStream, a, len) : mid(sourceTokenStream, a, len);
    result.push_back(DiffBlock(op, toks));
  }

  return result;
}

void NDiff::prettyOutput(std::list<DiffBlock> &DBs) {
  std::list<DiffBlock>::iterator i(DBs.begin()), e(DBs.end());
  for (; i != e; ++i) {
    std::vector<Token> tokenStream = (*i).getTokens();
    Operation op = (*i).getOperation();

    if (op == EQUAL) 
      continue;

    const int lin = tokenStream.front().getLine();
    const int col = tokenStream.front().getColumn();
    const int linEnd = tokenStream.back().getLine();      
    const int colEnd = tokenStream.back().getColumn();
    const char cmd = (op == DELETE) ? 'd' : 'a';
    const char marker = (op == DELETE) ? '<' : '>'; 

    printf("%d,%d%c%d,%d\n", lin, col, cmd, linEnd, colEnd);
    fputc(marker, stdout);
    fputc(' ', stdout);
    for (int j = 0, end = tokenStream.size(); j < end; ++j) {
      std::string chardata = tokenStream[j].getCharData();
      for (int c = 0; c < chardata.size(); ++c) {
        fputc(chardata[c], stdout);
        if (chardata[c] == '\n') {
          fputc(marker, stdout);
          fputc(' ', stdout);
        }
      }
    }
    fputc('\n', stdout);  
  }
}
