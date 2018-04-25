//
//
//

#ifndef ANNOTATEVALUES_ANNOTATEINSTRUCTIONSPASS_HPP
#define ANNOTATEVALUES_ANNOTATEINSTRUCTIONSPASS_HPP

#include "Config.hpp"

#include "AnnotateInstructions.hpp"

#include "llvm/Pass.h"
// using llvm::ModulePass
// using llvm::AnalysisUsage
// using llvm::RegisterPass

namespace llvm {
class Module;
class AnalysisUsage;
} // namespace llvm

namespace icsa {

struct AnnotateInstructionsPass : public llvm::ModulePass {
  static char ID;
  AnnotateInstructions::InstructionIDTy InstructionID;

  AnnotateInstructionsPass() : llvm::ModulePass(ID), InstructionID(0) {}

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(llvm::Module &CurModule) override;
};

} // namespace icsa

#endif // header
