//
//
//

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass;
// using llvm::LoopInfo;
// using llvm::Loop;

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::desc

#include "llvm/Support/Debug.h"
// using DEBUG macro
// using llvm::dbgs

#include "Config.hpp"

#include "AnnotateLoops.hpp"

#include "AnnotateLoopsPass.hpp"

#define DEBUG_TYPE "annotate-loops"

#define STRINGIFY_UTIL(x) #x
#define STRINGIFY(x) STRINGIFY_UTIL(x)

#define PRJ_CMDLINE_DESC(x) x " (version: " STRINGIFY(ANNOTATELOOPS_VERSION) ")"

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
    X("annotate-loops", PRJ_CMDLINE_DESC("annotate loops"), false, false);

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

static llvm::cl::opt<unsigned int>
    LoopDepthThreshold("al-loop-depth-threshold",
                       llvm::cl::desc("loop depth threshold"),
                       llvm::cl::init(1));

static llvm::cl::opt<unsigned int> LoopStartId("al-loop-start-id",
                                               llvm::cl::desc("loop start id"),
                                               llvm::cl::init(1));

static llvm::cl::opt<unsigned int>
    LoopIdInterval("al-loop-id-interval", llvm::cl::desc("loop id interval"),
                   llvm::cl::init(1));

namespace icsa {

void AnnotateLoopsPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addRequired<llvm::LoopInfoWrapperPass>();
  AU.setPreservesAll();

  return;
}

bool AnnotateLoopsPass::runOnModule(llvm::Module &CurModule) {
  for (auto &CurFunc : CurModule) {
    if (CurFunc.isDeclaration())
      continue;

    AnnotateLoops annotator{LoopDepthThreshold, LoopStartId, LoopIdInterval};

    auto &LIPass = Pass::getAnalysis<llvm::LoopInfoWrapperPass>(CurFunc);
    auto &LI = LIPass.getLoopInfo();
    annotator.annotateWithId(LI);
  }

  return false;
}

} // namespace icsa end
