//
//
//

#ifndef ANNOTATELOOPSPASS_HPP
#define ANNOTATELOOPSPASS_HPP

#include "llvm/Pass.h"
// using llvm::ModulePass
// using llvm::AnalysisUsage
// using llvm::RegisterPass

namespace llvm {
class Module;
class AnalysisUsage;
} // namespace llvm end

namespace {

struct AnnotateLoopsPass : public llvm::ModulePass {
  static char ID;
  unsigned int m_LoopID;

  AnnotateLoopsPass() : llvm::ModulePass(ID), m_LoopID(0) {}

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  bool runOnModule(llvm::Module &CurModule) override;
};

} // namespace unnamed end

#endif // ANNOTATELOOPSPASS_HPP
