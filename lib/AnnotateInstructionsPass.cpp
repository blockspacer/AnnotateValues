//
//
//

#include "AnnotateInstructionsPass.hpp"

#include "AnnotateInstructions.hpp"

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

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/FileSystem.h"
// using llvm::sys::fs::OpenFlags

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::desc
// using llvm::cl::cat
// using llvm::cl::OptionCategory

#include "llvm/IR/DebugInfoMetadata.h"
// using llvm::DIScope

#include <algorithm>
// using std::for_each

#include <string>
// using std::string

#include <utility>
// using std::pair

#include <tuple>
// using std::tuple
// using std::make_tuple
// using std::get

#include <map>
// using std::map

#include <fstream>
// using std::ifstream

#include <system_error>
// using std::error_code

#include <cstring>
// using std::strncmp

#include <cstdint>
// using std::uint64_t

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
bool passDebugFlag = false;
static llvm::cl::opt<bool, true>
    Debug("ai-debug", llvm::cl::desc("debug annotate instructions pass"),
          llvm::cl::location(passDebugFlag),
          llvm::cl::cat(AnnotateInstructionsCategory));

LogLevel passLogLevel = LogLevel::info;
static llvm::cl::opt<LogLevel, true> DebugLevel(
    "ai-debug-level",
    llvm::cl::desc("debug level for annotate instructions pass"),
    llvm::cl::location(passLogLevel),
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

using FunctionNameTy = std::string;
using InstructionIDRange = std::pair<AnnotateInstructions::InstructionIDTy,
                                     AnnotateInstructions::InstructionIDTy>;
std::uint64_t NumFunctionsProcessed = 0;
std::map<FunctionNameTy, InstructionIDRange> FunctionsAltered;

void ReportStats(llvm::StringRef Filename) {
  std::error_code err;
  llvm::raw_fd_ostream report(Filename, err, llvm::sys::fs::F_Text);

  if (err) {
    llvm::errs() << "could not open file: \"" << Filename
                 << "\" reason: " << err.message() << "\n";
  } else {
    report << NumFunctionsProcessed << "\n";

    for (const auto &e : FunctionsAltered) {
      report << e.first << " " << e.second.first << " " << e.second.second
             << "\n";
    }
  }
}

} // namespace anonymous

bool AnnotateInstructionsPass::runOnModule(llvm::Module &CurModule) {
  bool shouldReportStats = !ReportStatsFilename.empty();
  bool useFuncWhitelist = !FuncWhiteListFilename.empty();
  bool hasChanged = false;

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

  llvm::SmallVector<llvm::Instruction *, 16> workList;
  AnnotateInstructions annotator{StartId, IdInterval};

  for (auto &CurFunc : CurModule) {
    if (CurFunc.isDeclaration() ||
        (useFuncWhitelist &&
         !funcWhiteList.matches(CurFunc.getName().data()))) {
      continue;
    }

    NumFunctionsProcessed++;
    workList.clear();

    auto rangeStart = annotator.current();

    if (AIOpts::Write == OperationMode && workList.size()) {
      for (auto *e : workList) {
        auto id = annotator.annotate(*e);
      }
    }

    if (AIOpts::Read == OperationMode && shouldReportStats) {
      auto pred = [&](const auto *e) {
        if (annotator.has(*e))
          ;
      };

      std::for_each(workList.begin(), workList.end(), pred);
    }

    auto rangeEnd = annotator.current();

    if (shouldReportStats && AIOpts::Write == OperationMode &&
        workList.size()) {
      FunctionsAltered.emplace(CurFunc.getName(),
                               std::make_pair(rangeStart, rangeEnd));
      hasChanged = true;
    }
  }

  if (shouldReportStats) {
    ReportStats(ReportStatsFilename);
  }

  return hasChanged;
}

} // namespace icsa
