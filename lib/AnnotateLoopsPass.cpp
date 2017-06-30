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

#include "llvm/Support/FileSystem.h"
// using llvm::sys::fs::OpenFlags

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::desc

#include "llvm/Support/Debug.h"
// using DEBUG macro
// using llvm::dbgs

#include <string>
// using std::string

#include <utility>
// using std::pair

#include <map>
// using std::map

#include <fstream>
// using std::ifstream

#include <system_error>
// using std::error_code

#include <cstring>
// using std::strncmp

#include "Config.hpp"

#include "BWList.hpp"

#include "AnnotateLoops.hpp"

#include "AnnotateLoopsPass.hpp"

#define DEBUG_TYPE "annotate_loops"

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
//

static llvm::cl::opt<std::string>
    ReportStatsFilename("al-stats",
                        llvm::cl::desc("annotate loops stats report filename"));

static llvm::cl::opt<std::string>
    FuncWhileListFilename("al-fn-whitelist",
                          llvm::cl::desc("function whitelist"));

namespace icsa {

namespace {

using FunctionName_t = std::string;
using LoopIdRange_t =
    std::pair<AnnotateLoops::LoopID_t, AnnotateLoops::LoopID_t>;

long NumFunctionsProcessed = 0;
std::map<FunctionName_t, LoopIdRange_t> FunctionsAltered;

void ReportStats(void) {
  PLUGIN_OUT << NumFunctionsProcessed << "\n";

  for (const auto &e : FunctionsAltered) {
    PLUGIN_OUT << e.first << " " << e.second.first << " " << e.second.second
               << "\n";
  }

  return;
}

void ReportStats(const char *Filename) {
  const char *stdout_marker = "--";
  if (0 == std::strncmp(stdout_marker, Filename, strlen(stdout_marker))) {
    ReportStats();

    return;
  }

  std::error_code err;

  llvm::raw_fd_ostream report(Filename, err, llvm::sys::fs::F_Text);

  if (err)
    PLUGIN_ERR << "could not open file: \"" << ReportStatsFilename
               << "\" reason: " << err.message() << "\n";
  else {
    report << NumFunctionsProcessed << "\n";

    for (const auto &e : FunctionsAltered)
      report << e.first << " " << e.second.first << " " << e.second.second
             << "\n";
  }

  return;
}

} // namespace anonymous end

void AnnotateLoopsPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addRequired<llvm::LoopInfoWrapperPass>();
  AU.setPreservesAll();

  return;
}

bool AnnotateLoopsPass::runOnModule(llvm::Module &CurModule) {
  bool shouldReportStats = !ReportStatsFilename.empty();
  bool useFuncWhitelist = !FuncWhileListFilename.empty();
  bool hasChanged = false;

  BWList funcWhileList;
  if (useFuncWhitelist) {
    std::ifstream funcWhiteListFile{FuncWhileListFilename};

    if (funcWhiteListFile.is_open()) {
      funcWhileList.addRegex(funcWhiteListFile);
      funcWhiteListFile.close();
    } else
      PLUGIN_ERR << "could not open file: \'" << FuncWhileListFilename
                 << "\'\n";
  }

  AnnotateLoops annotator{LoopDepthThreshold, LoopStartId, LoopIdInterval};

  for (auto &CurFunc : CurModule) {
    if (useFuncWhitelist && !funcWhileList.matches(CurFunc.getName().data()))
      continue;

    if (CurFunc.isDeclaration())
      continue;

    auto &LIPass = Pass::getAnalysis<llvm::LoopInfoWrapperPass>(CurFunc);
    auto &LI = LIPass.getLoopInfo();

    auto rangeStart = annotator.getId();

    annotator.annotateWithId(LI);
    NumFunctionsProcessed++;

    auto rangeOpenEnd = annotator.getId();

    if (shouldReportStats && rangeStart != rangeOpenEnd) {
      FunctionsAltered.emplace(CurFunc.getName(),
                               std::make_pair(rangeStart, rangeOpenEnd));
    }

    hasChanged = true;
  }

  if (shouldReportStats)
    ReportStats(ReportStatsFilename.c_str());

  return hasChanged;
}

} // namespace icsa end
