//
//
//

#include "AnnotateValues/Config.hpp"

#include "AnnotateValues/Debug.hpp"

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::desc
// using llvm::cl::OptionCategory

static llvm::cl::OptionCategory
    AnnotateValuesCategory("AnnotateValues passes",
                           "Common options for AnnotateValues passes");

#if ANNOTATEVALUES_DEBUG
static llvm::cl::opt<bool, true>
    Debug("av-debug", llvm::cl::desc("debug annotate values passes"),
          llvm::cl::location(icsa::debug::passDebugFlag),
          llvm::cl::cat(AnnotateValuesCategory));

static llvm::cl::opt<LogLevel, true> DebugLevel(
    "av-debug-level", llvm::cl::desc("debug level for annotate values passes"),
    llvm::cl::location(icsa::debug::passLogLevel),
    llvm::cl::values(
        clEnumValN(LogLevel::Info, "Info", "informational messages"),
        clEnumValN(LogLevel::Notice, "Notice", "significant conditions"),
        clEnumValN(LogLevel::Warning, "Warning", "warning conditions"),
        clEnumValN(LogLevel::Error, "Error", "error conditions"),
        clEnumValN(LogLevel::Debug, "Debug", "debug messages")
// clang-format off
#if (LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR < 9)
                                , clEnumValEnd
#endif
        // clang-format on
        ),
    llvm::cl::cat(AnnotateValuesCategory));
#endif // ANNOTATEVALUES_DEBUG
