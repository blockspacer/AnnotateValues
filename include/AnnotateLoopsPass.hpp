//
//
//

#ifndef ANNOTATELOOPSPASS_HPP
#define ANNOTATELOOPSPASS_HPP

#include "Config.hpp"

#include "llvm/Pass.h"
// using llvm::ModulePass
// using llvm::AnalysisUsage
// using llvm::RegisterPass

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass

namespace llvm {
class Module;
class AnalysisUsage;
} // namespace llvm end

namespace icsa {

struct AnnotateLoopsPass : public llvm::ModulePass {
  static char ID;
  unsigned int LoopID;

  AnnotateLoopsPass() : llvm::ModulePass(ID), LoopID(0) {}

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.addRequired<llvm::LoopInfoWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnModule(llvm::Module &CurModule) override;
};

} // namespace icsa end

#endif // ANNOTATELOOPSPASS_HPP
