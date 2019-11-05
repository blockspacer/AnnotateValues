//
//
//

#pragma once

#include "AnnotateValues/Config.hpp"

#include "AnnotateValues/AnnotateLoops.hpp"

#include "llvm/Pass.h"
// using llvm::ModulePass
// using llvm::AnalysisUsage
// using llvm::RegisterPass

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass

namespace llvm {
class Module;
class AnalysisUsage;
} // namespace llvm

namespace icsa {

struct AnnotateLoopsPass : public llvm::ModulePass {
  static char ID;
  AnnotateLoops::LoopIDTy LoopID;

  AnnotateLoopsPass() : llvm::ModulePass(ID), LoopID(0) {}

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.addRequired<llvm::LoopInfoWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnModule(llvm::Module &CurModule) override;
};

} // namespace icsa

