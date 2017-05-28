//
//
//

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Type.h"
// using llvm::IntType

#include "llvm/IR/Constants.h"
// using llvm::ConstantInt

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfo;
// using llvm::Loop;

#include "llvm/IR/Metadata.h"
// using llvm::Metadata
// using llvm::MDNode

#include "llvm/IR/MDBuilder.h"
// using llvm::MDBuilder

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "AnnotateLoops.hpp"

namespace icsa {

void AnnotateLoops::annotateWithId(llvm::Loop &CurLoop) {
  auto &curContext =
      CurLoop.getHeader()->getParent()->getParent()->getContext();
  llvm::MDBuilder loopMDBuilder(curContext);
  llvm::SmallVector<llvm::Metadata *, 2> loopIDValues;

  // create loop metadata node with custom id
  loopIDValues.push_back(loopMDBuilder.createString(m_idKey));
  auto *intType = llvm::Type::getInt32Ty(curContext);
  loopIDValues.push_back(loopMDBuilder.createConstant(
      llvm::ConstantInt::get(intType, m_currentId)));

  m_currentId += m_idInterval;

  auto *const loopIdMD = llvm::MDNode::get(curContext, loopIDValues);

  // create storage for loop metadata
  llvm::SmallVector<llvm::Metadata *, 4> newLoopMDs;
  newLoopMDs.push_back(nullptr); // reserve the first position for self
  newLoopMDs.push_back(loopIdMD);

  // preserve any existing loop metadata
  auto *loopMD = CurLoop.getLoopID();
  for (auto i = 0; loopMD && i < loopMD->getNumOperands(); ++i)
    newLoopMDs.push_back(loopMD->getOperand(i));

  // place loop id first
  auto newLoopIdMD = llvm::MDNode::get(curContext, newLoopMDs);
  newLoopIdMD->replaceOperandWith(0, newLoopIdMD);

  CurLoop.setLoopID(newLoopIdMD);

  return;
}

void AnnotateLoops::annotateWithId(llvm::LoopInfo &LI) {
  for (auto *CurLoop : LI) {
    annotateWithId(*CurLoop);

    for (auto &SubLoopIt : *CurLoop) {
      if ((*SubLoopIt).getLoopDepth() <= m_loopDepthThreshold)
        annotateWithId(*SubLoopIt);
    }
  }

  return;
}

} // namespace icsa end
