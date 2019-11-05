//
//
//

#include "AnnotateValues/AnnotateInstructions.hpp"

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Type.h"
// using llvm::IntType

#include "llvm/IR/Constants.h"
// using llvm::ConstantInt

#include "llvm/IR/Metadata.h"
// using llvm::Metadata
// using llvm::MDNode
// using llvm::ConstantAsMetadata

#include "llvm/IR/MDBuilder.h"
// using llvm::MDBuilder

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/Casting.h"
// using llvm::dyn_cast

#include <cassert>
// using assert

namespace icsa {
namespace AnnotateInstructions {

bool Reader::has(const llvm::Instruction &CurInstruction) const {
  return nullptr != CurInstruction.getMetadata(key());
}

InstructionIDTy Reader::get(const llvm::Instruction &CurInstruction) const {
  const auto *IDNode = CurInstruction.getMetadata(key());

  assert(nullptr != IDNode &&
         "Instruction does not have the requested  metadata!");

  const auto *constantMD =
      llvm::dyn_cast<llvm::ConstantAsMetadata>(IDNode->getOperand(1).get());
  const auto &IDConstant = constantMD->getValue()->getUniqueInteger();

  return IDConstant.getLimitedValue();
}

//

InstructionIDTy Writer::put(llvm::Instruction &CurInstruction) {
  auto &curContext = CurInstruction.getParent()->getParent()->getContext();
  llvm::MDBuilder builder{curContext};

  auto *intType = llvm::Type::getInt32Ty(curContext);
  auto curID = current();
  llvm::SmallVector<llvm::Metadata *, 1> IDValues{
      builder.createConstant(llvm::ConstantInt::get(intType, curID))};

  next();

  CurInstruction.setMetadata(key(), llvm::MDNode::get(curContext, IDValues));

  return curID;
}

} // namespace AnnotateInstructions
} // namespace icsa
