//
//
//

#include "AnnotateLoopsPass.hpp"

#include "AnnotateLoops.hpp"

#include "BWList.hpp"

#include "Utils.hpp"

#include "llvm/Config/llvm-config.h"

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfo
// using llvm::Loop

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

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

#define DEBUG_TYPE "annotateloops"

#define STRINGIFY_UTIL(x) #x
#define STRINGIFY(x) STRINGIFY_UTIL(x)

#define PRJ_CMDLINE_DESC(x) x " (version: " STRINGIFY(VERSION_STRING) ")"

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

static llvm::cl::OptionCategory
    AnnotateLoopsCategory("AnnotateLoops pass",
                          "Options for AnnotateLoops pass");

enum struct ALOpts {
  write,
  read,
};

static llvm::cl::opt<ALOpts> OperationMode(
    "al-mode", llvm::cl::desc("operation mode"), llvm::cl::init(ALOpts::write),
    llvm::cl::values(clEnumValN(ALOpts::write, "write",
                                "write looops with annotated id mode"),
                     clEnumValN(ALOpts::read, "read",
                                "read loops with annotated id mode")
// clang-format off
#if (LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR < 9)
                                , clEnumValEnd
#endif
                     // clang-format on
                     ),
    llvm::cl::cat(AnnotateLoopsCategory));

static llvm::cl::opt<unsigned int>
    LoopDepthThreshold("al-loop-depth-threshold",
                       llvm::cl::desc("loop depth threshold"),
                       llvm::cl::init(1), llvm::cl::cat(AnnotateLoopsCategory));

static llvm::cl::opt<unsigned int>
    LoopStartId("al-loop-start-id", llvm::cl::desc("loop start id"),
                llvm::cl::init(1), llvm::cl::cat(AnnotateLoopsCategory));

static llvm::cl::opt<unsigned int>
    LoopIdInterval("al-loop-id-interval", llvm::cl::desc("loop id interval"),
                   llvm::cl::init(1), llvm::cl::cat(AnnotateLoopsCategory));
//

static llvm::cl::opt<bool>
    ReportLoopLineNumbers("al-loop-lines",
                          llvm::cl::desc("report loop file lines"),
                          llvm::cl::cat(AnnotateLoopsCategory));

static llvm::cl::opt<std::string>
    ReportStatsFilename("al-stats",
                        llvm::cl::desc("annotate loops stats report filename"),
                        llvm::cl::cat(AnnotateLoopsCategory));

static llvm::cl::opt<std::string>
    FuncWhiteListFilename("al-fn-whitelist",
                          llvm::cl::desc("function whitelist"),
                          llvm::cl::cat(AnnotateLoopsCategory));

#if ANNOTATELOOPS_DEBUG
bool passDebugFlag = false;
static llvm::cl::opt<bool, true>
    Debug("al-debug", llvm::cl::desc("debug annotate loops pass"),
          llvm::cl::location(passDebugFlag),
          llvm::cl::cat(AnnotateLoopsCategory));

LogLevel passLogLevel = LogLevel::info;
static llvm::cl::opt<LogLevel, true> DebugLevel(
    "al-debug-level", llvm::cl::desc("debug level for annotate loops pass"),
    llvm::cl::location(passLogLevel),
    llvm::cl::values(
        clEnumValN(LogLevel::info, "info", "informational messages"),
        clEnumValN(LogLevel::notice, "notice", "significant conditions"),
        clEnumValN(LogLevel::warning, "warning", "warning conditions"),
        clEnumValN(LogLevel::error, "error", "error conditions"),
        clEnumValN(LogLevel::debug, "debug", "debug messages"), nullptr),
    llvm::cl::cat(AnnotateLoopsCategory));
#endif // ANNOTATELOOPS_DEBUG

//

namespace icsa {

namespace {

using FunctionNameTy = std::string;
using LoopIDRange = std::pair<AnnotateLoops::LoopIDTy, AnnotateLoops::LoopIDTy>;
using LineNumberTy = std::uint64_t;
using FileNameTy = std::string;

std::uint64_t NumFunctionsProcessed = 0;
std::map<FunctionNameTy, LoopIDRange> FunctionsAltered;

std::map<AnnotateLoops::LoopIDTy,
         std::tuple<FunctionNameTy, LineNumberTy, FileNameTy>> LoopsAnnotated;

void ReportStats(const char *Filename) {
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

    report << "--\n";

    for (const auto &e : LoopsAnnotated) {
      report << e.first << ' ' << std::get<0>(e.second);

      if (ReportLoopLineNumbers) {
        report << ' ' << std::get<1>(e.second);
        report << ' ' << std::get<2>(e.second);
      }

      report << '\n';
    }
  }

  return;
}

} // namespace anonymous

bool AnnotateLoopsPass::runOnModule(llvm::Module &CurModule) {
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

  llvm::SmallVector<llvm::Loop *, 16> workList;
  AnnotateLoops annotator{LoopStartId, LoopIdInterval};

  for (auto &CurFunc : CurModule) {
    if (CurFunc.isDeclaration() ||
        (useFuncWhitelist &&
         !funcWhiteList.matches(CurFunc.getName().data()))) {
      continue;
    }

    NumFunctionsProcessed++;
    workList.clear();
    auto &LI = getAnalysis<llvm::LoopInfoWrapperPass>(CurFunc).getLoopInfo();

    std::for_each(LI.begin(), LI.end(),
                  [&workList](llvm::Loop *e) { workList.push_back(e); });

    // TODO this needs documentation
    for (auto i = 0; i < workList.size(); ++i) {
      for (auto &e : workList[i]->getSubLoops()) {
        workList.push_back(e);
      }
    }

    workList.erase(
        std::remove_if(workList.begin(), workList.end(), [](const auto *e) {
          auto d = e->getLoopDepth();
          return d > LoopDepthThreshold;
        }), workList.end());

    auto rangeStart = annotator.current();

    if (ALOpts::write == OperationMode) {
      for (auto *e : workList) {
        auto id = annotator.current();
        annotator.annotate(*e);

        if (shouldReportStats) {
          LineNumberTy line = 0;
          FileNameTy filename;

          if (ReportLoopLineNumbers) {
            line = e->getStartLoc().getLine();
            auto *scope =
                llvm::cast<llvm::DIScope>(e->getStartLoc().getScope());
            filename = scope->getFilename();
          }

          LoopsAnnotated.emplace(
              id, std::make_tuple(CurFunc.getName().str(), line, filename));
        }
      }
    } else {
      for (auto *e : workList) {
        if (annotator.has(*e)) {
          auto id = annotator.get(*e);

          if (shouldReportStats) {
            LoopsAnnotated.emplace(id, CurFunc.getName().str());
          }
        }
      }
    }

    auto rangeOpenEnd = annotator.current();

    if (shouldReportStats && ALOpts::write == OperationMode &&
        workList.size()) {
      FunctionsAltered.emplace(CurFunc.getName(),
                               std::make_pair(rangeStart, rangeOpenEnd));
      hasChanged = true;
    }
  }

  if (shouldReportStats) {
    ReportStats(ReportStatsFilename.c_str());
  }

  return hasChanged;
}

} // namespace icsa
