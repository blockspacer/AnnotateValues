//
//
//

#include "AnnotateInstructionsPass.hpp"

#include "AnnotateInstructions.hpp"

#include "Stats.hpp"

#include "BWList.hpp"

#include "Utils.hpp"

#include "llvm/Config/llvm-config.h"

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/IR/InstIterator.h"
// using llvm::inst_begin
// using llvm::inst_end

#include "llvm/ADT/iterator_range.h"
// using llvm::make_range

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::desc
// using llvm::cl::cat
// using llvm::cl::OptionCategory

#include "llvm/Support/ErrorHandling.h"
// using llvm_unreachable

#include <algorithm>
// using std::for_each
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

#define STRINGIFY_UTIL(x) #x
#define STRINGIFY(x) STRINGIFY_UTIL(x)

#define PRJ_CMDLINE_DESC(x) x " (version: " STRINGIFY(VERSION_STRING) ")"

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
    StartId("ai-start-id", llvm::cl::desc("instruction start ID"),
            llvm::cl::init(1), llvm::cl::cat(AnnotateInstructionsCategory));

static llvm::cl::opt<unsigned int>
    IdInterval("ai-id-interval",
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

#if ANNOTATELOOPS_DEBUG
static llvm::cl::opt<bool, true>
    Debug("ai-debug", llvm::cl::desc("debug annotate instructions pass"),
          llvm::cl::location(icsa::passDebugFlag),
          llvm::cl::cat(AnnotateInstructionsCategory));

static llvm::cl::opt<LogLevel, true> DebugLevel(
    "ai-debug-level",
    llvm::cl::desc("debug level for annotate instructions pass"),
    llvm::cl::location(icsa::passLogLevel),
    llvm::cl::values(
        clEnumValN(LogLevel::info, "info", "informational messages"),
        clEnumValN(LogLevel::notice, "notice", "significant conditions"),
        clEnumValN(LogLevel::warning, "warning", "warning conditions"),
        clEnumValN(LogLevel::error, "error", "error conditions"),
        clEnumValN(LogLevel::debug, "debug", "debug messages"), nullptr),
    llvm::cl::cat(AnnotateInstructionsCategory));
#endif // ANNOTATELOOPS_DEBUG

//

namespace icsa {

namespace {

template <typename T> decltype(auto) make_inst_range(T &&Unit) {
  return llvm::make_range(llvm::inst_begin(std::forward<T>(Unit)),
                          llvm::inst_end(std::forward<T>(Unit)));
}

} // namespace

bool AnnotateInstructionsPass::runOnModule(llvm::Module &CurModule) {
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
    if (CurFunc.isDeclaration() ||
        (useFuncWhitelist &&
         !funcWhiteList.matches(CurFunc.getName().data()))) {
      continue;
    }

    auto instructions = make_inst_range(CurFunc);

    if (AIOpts::Write == OperationMode) {
      AnnotateInstructions::Writer writer{StartId, IdInterval};

      for (auto &e : instructions) {
        hasChanged |= true;
        writer.put(e);
      }

      if (shouldReportStats && instructions.begin() != instructions.end()) {
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
