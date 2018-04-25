//
//
//

#include "AnnotateInstructionsPass.hpp"

#include "AnnotateInstructions.hpp"

#include "Stats.hpp"

#include "BWList.hpp"

#include "Util.hpp"

#include "Debug.hpp"

#include "llvm/Config/llvm-config.h"
// version macros

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::desc
// using llvm::cl::cat
// using llvm::cl::OptionCategory

#include "llvm/Support/raw_ostream.h"
// using llvm::errs

#include "llvm/Support/ErrorHandling.h"
// using llvm_unreachable

#include <algorithm>
// using std::any_of

#include <string>
// using std::string

#include <fstream>
// using std::ifstream

#include <utility>
// using std::forward

#include <cassert>
// using assert

#define DEBUG_TYPE "annotate-instructions"

// plugin registration for opt

char icsa::AnnotateInstructionsPass::ID = 0;
static llvm::RegisterPass<icsa::AnnotateInstructionsPass>
    X("annotate-instructions", PRJ_CMDLINE_DESC("annotate instructions"), false,
      false);

// plugin registration for clang

// the solution was at the bottom of the header file
// 'llvm/Transforms/IPO/PassManagerBuilder.h'
// create a static free-floating callback that uses the legacy pass manager to
// add an instance of this pass and a static instance of the
// RegisterStandardPasses class

static void
registerAnnotateInstructionsPass(const llvm::PassManagerBuilder &Builder,
                                 llvm::legacy::PassManagerBase &PM) {
  PM.add(new icsa::AnnotateInstructionsPass());
}

static llvm::RegisterStandardPasses RegisterAnnotateInstructionsPass(
    llvm::PassManagerBuilder::EP_EarlyAsPossible,
    registerAnnotateInstructionsPass);

//

static llvm::cl::OptionCategory
    AnnotateInstructionsCategory("AnnotateInstructions pass",
                                 "Options for AnnotateInstructions pass");

enum struct AIOpts {
  Write,
  Read,
};

static llvm::cl::opt<AIOpts> OperationMode(
    "ai-mode", llvm::cl::desc("operation mode"), llvm::cl::init(AIOpts::Write),
    llvm::cl::values(clEnumValN(AIOpts::Write, "Write",
                                "write instructions with annotated ID mode"),
                     clEnumValN(AIOpts::Read, "Read",
                                "read instructions with annotated ID mode")
// clang-format off
#if (LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR < 9)
                                , clEnumValEnd
#endif
                     // clang-format on
                     ),
    llvm::cl::cat(AnnotateInstructionsCategory));

static llvm::cl::opt<unsigned int>
    StartID("ai-start-id", llvm::cl::desc("instruction start ID"),
            llvm::cl::init(1), llvm::cl::cat(AnnotateInstructionsCategory));

static llvm::cl::opt<unsigned int>
    IDInterval("ai-id-interval",
               llvm::cl::desc("instruction ID increment interval"),
               llvm::cl::init(1), llvm::cl::cat(AnnotateInstructionsCategory));
//

static llvm::cl::opt<std::string> ReportStatsFilename(
    "ai-stats", llvm::cl::desc("annotate instructions stats report filename"),
    llvm::cl::cat(AnnotateInstructionsCategory));

static llvm::cl::opt<std::string>
    FuncWhiteListFilename("ai-fn-whitelist",
                          llvm::cl::desc("function whitelist"),
                          llvm::cl::cat(AnnotateInstructionsCategory));

//

namespace {

void checkCmdLineOptions() {
  if (OperationMode == AIOpts::Read) {
    assert(!StartID.getPosition() && "Cannot specify this option combination!");
    assert(!IDInterval.getPosition() &&
           "Cannot specify this option combination!");
  }

#if ANNOTATEVALUES_HAS_JSON == 0
  assert(!ReportStatsFilename.getPosition() &&
         "Cannot export stats when not configured with a JSON library");
#endif // ANNOTATEVALUES_HAS_JSON
}

} // namespace

namespace icsa {

bool AnnotateInstructionsPass::runOnModule(llvm::Module &CurModule) {
  checkCmdLineOptions();

  bool shouldReportStats = !ReportStatsFilename.empty();
  bool useFuncWhitelist = !FuncWhiteListFilename.empty();
  bool hasChanged = false;
  AnnotateInstructionsStats Stats;

  BWList funcWhiteList;
  if (useFuncWhitelist) {
    std::ifstream funcWhiteListFile{FuncWhiteListFilename};

    if (funcWhiteListFile.is_open()) {
      funcWhiteList.addRegex(funcWhiteListFile);
      funcWhiteListFile.close();
    } else {
      llvm::errs() << "could not open file: \'" << FuncWhiteListFilename
                   << "\'\n";
    }
  }

  for (auto &CurFunc : CurModule) {
    if (CurFunc.isDeclaration()) {
      continue;
    }

    if (useFuncWhitelist && !funcWhiteList.matches(CurFunc.getName().data())) {
      continue;
    }

    auto instructions = make_inst_range(CurFunc);

    if (AIOpts::Write == OperationMode) {
      AnnotateInstructions::Writer writer{StartID, IDInterval};

      for (auto &e : instructions) {
        hasChanged |= true;
        writer.put(e);
      }

      if (shouldReportStats && !is_range_empty(instructions)) {
        Stats.addProcessedFunction(CurFunc.getName());
      }
    } else if (AIOpts::Read == OperationMode && shouldReportStats) {
      AnnotateInstructions::Reader reader{};

      bool hasAnnotation = std::any_of(instructions.begin(), instructions.end(),
                                       [&](auto &e) { return reader.has(e); });

      if (shouldReportStats && hasAnnotation) {
        Stats.addProcessedFunction(CurFunc.getName());
      }
    } else {
      llvm_unreachable("Operation mode was not specified!");
    }
  }

  if (shouldReportStats && Stats) {
    Stats.save(ReportStatsFilename);
  }

  return hasChanged;
}

} // namespace icsa
