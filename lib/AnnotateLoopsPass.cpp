//
//
//

#include "llvm/IR/Type.h"
// using llvm::Module

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Constants.h"
// using llvm::Constant;
// using llvm::ConstantInt;

#include "llvm/IR/LLVMContext.h"
// using llvm::LLVMContext;

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass;
// using llvm::LoopInfo;
// using llvm::Loop;

#include "llvm/IR/Metadata.h"
// using llvm::Metadata
// using llvm::MDNode

#include "llvm/IR/MDBuilder.h"
// using llvm::MDBuilder

#include "llvm/IR/Constants.h"
// using llvm::ConstantInt

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/Debug.h"
// using DEBUG macro
// using llvm::dbgs

#include "AnnotateLoops.hpp"

#include "AnnotateLoopsPass.hpp"

#define DEBUG_TYPE "annotateloops"

#ifndef NDEBUG
#define PLUGIN_OUT llvm::outs()
//#define PLUGIN_OUT llvm::nulls()

// convenience macro when building against a NDEBUG LLVM
#undef DEBUG
#define DEBUG(X)                                                               \
  do {                                                                         \
    X;                                                                         \
  } while (0);
#else // NDEBUG
#define PLUGIN_OUT llvm::dbgs()
#endif // NDEBUG

#define PLUGIN_ERR llvm::errs()

// plugin registration for opt

char icsa::AnnotateLoopsPass::ID = 0;
static llvm::RegisterPass<icsa::AnnotateLoopsPass>
    X("annotate-loops", "annotate-loops", false, false);

// plugin registration for clang

// the solution was at the bottom of the header file
// 'llvm/Transforms/IPO/PassManagerBuilder.h'
// create a static free-floating callback that uses the legacy pass manager to
// add an instance of this pass and a static instance of the
// RegisterStandardPasses class

static void registerAnnotateLoopsPass(const llvm::PassManagerBuilder &Builder,
                                      llvm::legacy::PassManagerBase &PM) {
  PM.add(new icsa::AnnotateLoopsPass());

  return;
}

static llvm::RegisterStandardPasses
    RegisterAnnotateLoopsPass(llvm::PassManagerBuilder::EP_EarlyAsPossible,
                              registerAnnotateLoopsPass);

//

namespace icsa {

void AnnotateLoopsPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addRequired<llvm::LoopInfoWrapperPass>();
  AU.setPreservesAll();

  return;
}

bool AnnotateLoopsPass::runOnModule(llvm::Module &CurModule) {
  llvm::MDBuilder LoopMDBuilder(CurModule.getContext());

  for (auto &CurFunc : CurModule) {
    if (CurFunc.isDeclaration())
      continue;

    const auto &LIPass = Pass::getAnalysis<llvm::LoopInfoWrapperPass>(CurFunc);
    const auto &LI = LIPass.getLoopInfo();
    for (const auto *CurLoop : LI) {
      llvm::SmallVector<llvm::Metadata *, 2> LoopIDVals;

      LoopIDVals.push_back(LoopMDBuilder.createString("icsa.dynapar.loop.id"));
      auto *IntType = llvm::Type::getInt32Ty(CurModule.getContext());
      LoopIDVals.push_back(LoopMDBuilder.createConstant(
          llvm::ConstantInt::get(IntType, m_LoopID)));

      auto *const LoopIDMD =
          llvm::MDNode::get(CurModule.getContext(), LoopIDVals);

      llvm::SmallVector<llvm::Metadata *, 4> MDs;
      MDs.push_back(nullptr); // reserve the first position for self
      MDs.push_back(LoopIDMD);

      auto *LoopMD = CurLoop->getLoopID();

      if (LoopMD) {
        for (auto i = 0; i < LoopMD->getNumOperands(); ++i)
          MDs.push_back(LoopMD->getOperand(i));
      }

      auto NewLoopIDMD = llvm::MDNode::get(CurModule.getContext(), MDs);
      NewLoopIDMD->replaceOperandWith(0, NewLoopIDMD);

      CurLoop->setLoopID(NewLoopIDMD);
    } // loopinfo loop end
  }   // func loop end

  return false;
}

} // namespace icsa end
