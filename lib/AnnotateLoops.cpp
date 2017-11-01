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
// using llvm::MDTuple
// using llvm::MDString
// using llvm::ConstantAsMetadata

#include "llvm/IR/MDBuilder.h"
// using llvm::MDBuilder

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/Casting.h"
// using llvm::dyn_cast

#include <cassert>

#include "AnnotateLoops.hpp"

namespace icsa {

const llvm::Metadata *
AnnotateLoops::getAnnotatedIdNode(const llvm::Metadata *node) const {
  if (node->getMetadataID() != llvm::Metadata::MDTupleKind)
    return nullptr;

  const auto *tupleMD = llvm::dyn_cast<llvm::MDTuple>(node);

  if(tupleMD->getNumOperands() < 2)
    return nullptr;

  if (tupleMD->getOperand(0)->getMetadataID() != llvm::Metadata::MDStringKind)
    return nullptr;

  if (tupleMD->getOperand(1)->getMetadataID() !=
      llvm::Metadata::ConstantAsMetadataKind)
    return nullptr;

  const auto *strMD = llvm::dyn_cast<llvm::MDString>(tupleMD->getOperand(0));
  if (!strMD)
    return nullptr;

  if (strMD->getString().equals(m_idKey))
    return tupleMD;

  return nullptr;
}

const llvm::MDTuple *
AnnotateLoops::getAnnotatedIdNode(const llvm::Loop &CurLoop) const {
  if (!CurLoop.getLoopID())
    return nullptr;

  const auto *loopMD = CurLoop.getLoopID();

  for (auto i = 0; i < loopMD->getNumOperands(); ++i) {
    const auto *loopIdNode = getAnnotatedIdNode(loopMD->getOperand(i).get());

    if (loopIdNode)
      return llvm::dyn_cast<llvm::MDTuple>(loopIdNode);
  }

  return nullptr;
}

bool AnnotateLoops::hasAnnotatedId(const llvm::Loop &CurLoop) const {
  return getAnnotatedIdNode(CurLoop) != nullptr;
}

AnnotateLoops::LoopID_t
AnnotateLoops::getAnnotatedId(const llvm::Loop &CurLoop) const {
  const auto *idNode = getAnnotatedIdNode(CurLoop);

  assert(nullptr != idNode);

  const auto *idOperand = idNode->getOperand(1).get();
  const auto *constantMD = llvm::dyn_cast<llvm::ConstantAsMetadata>(idOperand);
  const auto &idConstant = constantMD->getValue()->getUniqueInteger();

  return idConstant.getLimitedValue();
}

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
  for (auto i = 1; loopMD && i < loopMD->getNumOperands(); ++i)
    if (!getAnnotatedIdNode(loopMD->getOperand(i)))
      newLoopMDs.push_back(loopMD->getOperand(i));

  // place loop id first
  auto newLoopIdMD = llvm::MDNode::get(curContext, newLoopMDs);
  newLoopIdMD->replaceOperandWith(0, newLoopIdMD);

  CurLoop.setLoopID(newLoopIdMD);

  return;
}

} // namespace icsa end
