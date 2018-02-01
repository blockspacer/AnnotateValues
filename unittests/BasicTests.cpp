//
//
//

#include "llvm/Config/llvm-config.h"

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Dominators.h"
// using llvm::DominatorTree

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfo

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_string_ostream

#include "gtest/gtest.h"
// using testing::Test

#include <array>
// using std::array

#include "TestIRAssemblyParser.hpp"

#include "AnnotateLoops.hpp"

namespace icsa {
namespace {

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

struct AnnotateLoopsTestData {
  std::string assemblyFile;
  unsigned nextId;
};

std::ostream &operator<<(std::ostream &os, const AnnotateLoopsTestData &td) {
  auto delim = ' ';
  return os << delim << "assembly file: " << td.assemblyFile << delim
            << "next id: " << td.nextId << delim;
}

class NoAnnotationTest : public TestIRAssemblyParser,
                         public testing::TestWithParam<AnnotateLoopsTestData> {
};

//

TEST_P(NoAnnotationTest, NoAnnotation) {
  auto td = GetParam();
  AnnotateLoops al{2, 3};

  parseAssemblyFile(td.assemblyFile);
  auto LI = calculateLoopInfo(*m_Module->begin());
  auto *curLoop = *LI.begin();
  al.hasAnnotatedId(*curLoop);

  EXPECT_EQ(al.hasAnnotatedId(*curLoop), false);
  EXPECT_EQ(al.getId(), td.nextId);
}

std::array<AnnotateLoopsTestData, 3> testData1 = {"regular_loop.ll",        2u,
                                                  "regular_nested_loop.ll", 2u,
                                                  "exit_call_loop.ll",      2u};

INSTANTIATE_TEST_CASE_P(DefaultInstance, NoAnnotationTest,
                        testing::ValuesIn(testData1));

//

class PostAnnotationTest
    : public TestIRAssemblyParser,
      public testing::TestWithParam<AnnotateLoopsTestData> {};

TEST_P(PostAnnotationTest, PostAnnotation) {
  auto td = GetParam();
  AnnotateLoops al{2, 3};

  parseAssemblyFile(td.assemblyFile);
  auto LI = calculateLoopInfo(*m_Module->begin());

  for (auto *e : LI)
    al.annotateWithId(*e);

  auto *curLoop = *LI.begin();

  EXPECT_EQ(al.hasAnnotatedId(*curLoop), true);
  EXPECT_EQ(al.getId(), td.nextId);
}

std::array<AnnotateLoopsTestData, 3> testData2 = {"regular_loop.ll",        5u,
                                                  "regular_nested_loop.ll", 5u,
                                                  "exit_call_loop.ll",      5u};

INSTANTIATE_TEST_CASE_P(DefaultInstance, PostAnnotationTest,
                        testing::ValuesIn(testData2));

} // namespace anonymous end
} // namespace icsa end
