//
//
//

#include <memory>
// using std::unique_ptr

#include <cassert>
// using assert

#include "llvm/Config/llvm-config.h"

#include "llvm/IR/LLVMContext.h"
// using llvm::LLVMContext

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Dominators.h"
// using llvm::DominatorTree

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfo

#include "llvm/Support/SourceMgr.h"
// using llvm::SMDiagnostic

#include "llvm/AsmParser/Parser.h"
// using llvm::parseAssemblyFile
// using llvm::parseAssemblyString

#include "llvm/IR/Verifier.h"
// using llvm::verifyModule

#include "llvm/Support/ErrorHandling.h"
// using llvm::report_fatal_error

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_string_ostream

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include "gtest/gtest.h"
// using testing::Test

#include "AnnotateLoops.hpp"

namespace icsa {
namespace {

class IRAssemblyTest {
public:
  IRAssemblyTest() : m_Module{nullptr}, m_TestDataDir{"./unittests/data/"} {
#if (LLVM_VERSION_MAJOR >= 4) ||                                               \
    (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9)
    llvm::Context theContext;
    m_Context = &theContext;
#else
    m_Context = &(llvm::getGlobalContext());
#endif
  }

  void parseAssemblyFile(llvm::StringRef AssemblyHolder) {
    m_Module = llvm::parseAssemblyFile((m_TestDataDir + AssemblyHolder).str(),
                                       m_Diagnostic, *m_Context);

    report();
  }

  void parseAssemblyString(llvm::StringRef AssemblyHolder) {
    m_Module =
        llvm::parseAssemblyString(AssemblyHolder, m_Diagnostic, *m_Context);

    report();
  }

protected:
  void report() {
    std::string msg;
    llvm::raw_string_ostream os(msg);
    m_Diagnostic.print("", os);

    if (llvm::verifyModule(*m_Module, &(llvm::errs())))
      llvm::report_fatal_error("module verification failed\n");

    if (!m_Module)
      llvm::report_fatal_error(os.str().c_str());
  }

  std::unique_ptr<llvm::Module> m_Module;
  const char *m_TestDataDir;
  llvm::LLVMContext *m_Context;
  llvm::SMDiagnostic m_Diagnostic;
};

//

struct LoopIDAnnotationTestData {
  std::string assemblyFile;
  bool isAnnotated;
  unsigned currentID;
  unsigned nextID;
};

class AnnotateLoopsTest
    : public IRAssemblyTest,
      public testing::TestWithParam<LoopIDAnnotationTestData> {};

//

static llvm::LoopInfo calculateLoopInfo(llvm::Function &Func) {
  llvm::DominatorTree DT;
  llvm::LoopInfo LI;

  DT.recalculate(Func);
#if (LLVM_VERSION_MAJOR >= 4) ||                                               \
    (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 8)
  LI.analyze(DT);
#else
  LI.Analyze(DT);
#endif

  return LI;
}

TEST_P(AnnotateLoopsTest, NoAnnotation) {
  auto td = GetParam();
  AnnotateLoops al{2, 3};

  parseAssemblyFile(td.assemblyFile);
  auto LI = calculateLoopInfo(*m_Module->begin());
  auto *curLoop = *LI.begin();
  al.hasAnnotatedId(*curLoop);

  EXPECT_EQ(al.hasAnnotatedId(*curLoop), true);
}

INSTANTIATE_TEST_CASE_P(Default, AnnotateLoopsTest,
                        testing::Values(LoopIDAnnotationTestData{
                            "test01.ll", false, 2u, 5u}));

} // namespace anonymous end
} // namespace icsa end
