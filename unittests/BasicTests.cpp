//
//
//

#include "gtest/gtest.h"
// using testing::Test

#include <array>
// using std::array

#include "TestIRAssemblyParser.hpp"

#include "TestCommon.hpp"

#include "AnnotateLoops.hpp"

namespace icsa {
namespace testing {

class NoAnnotationTest
    : public TestIRAssemblyParser,
      public ::testing::TestWithParam<AnnotateLoopsTestData> {};

//

TEST_P(NoAnnotationTest, NoAnnotation) {
  auto td = GetParam();
  AnnotateLoops al{2, 3};

  parseAssemblyFile(td.assemblyFile);
  auto LI = calculateLoopInfo(*module().begin());
  auto *curLoop = *LI.begin();
  al.has(*curLoop);

  EXPECT_EQ(al.has(*curLoop), false);
  EXPECT_EQ(al.current(), td.nextId);
}

std::array<AnnotateLoopsTestData, 3> testData1 = {"regular_loop.ll",        2u,
                                                  "regular_nested_loop.ll", 2u,
                                                  "exit_call_loop.ll",      2u};

INSTANTIATE_TEST_CASE_P(DefaultInstance, NoAnnotationTest,
                        ::testing::ValuesIn(testData1));

//

class PostAnnotationTest
    : public TestIRAssemblyParser,
      public ::testing::TestWithParam<AnnotateLoopsTestData> {};

TEST_P(PostAnnotationTest, PostAnnotation) {
  auto td = GetParam();
  AnnotateLoops::LoopIDTy startId = 2;
  AnnotateLoops al{startId, 3};

  parseAssemblyFile(td.assemblyFile);
  auto LI = calculateLoopInfo(*module().begin());

  for (auto *e : LI)
    al.annotate(*e);

  auto *curLoop = *LI.begin();

  EXPECT_EQ(al.has(*curLoop), true);
  EXPECT_EQ(al.current(), td.nextId);
  EXPECT_EQ(al.get(*curLoop), startId);
}

std::array<AnnotateLoopsTestData, 3> testData2 = {"regular_loop.ll",        5u,
                                                  "regular_nested_loop.ll", 5u,
                                                  "exit_call_loop.ll",      5u};

INSTANTIATE_TEST_CASE_P(DefaultInstance, PostAnnotationTest,
                        ::testing::ValuesIn(testData2));

//

class PostAnnotationNestedTest
    : public TestIRAssemblyParser,
      public ::testing::TestWithParam<AnnotateLoopsTestData> {};

TEST_P(PostAnnotationNestedTest, PostAnnotationNested) {
  auto td = GetParam();
  AnnotateLoops::LoopIDTy startId = 2;
  AnnotateLoops al{startId, 3};

  parseAssemblyFile(td.assemblyFile);
  auto LI = calculateLoopInfo(*module().begin());

  for (auto *e : LI) {
    al.annotate(*e);

    for (auto k : e->getSubLoops())
      if (k->getLoopDepth() <= 2)
        al.annotate(*k);
  }

  auto *curLoop = *LI.begin();

  EXPECT_EQ(al.has(*curLoop), true);
  EXPECT_EQ(al.current(), td.nextId);
  EXPECT_EQ(al.get(*curLoop), startId);
}

std::array<AnnotateLoopsTestData, 3> testData3 = {"regular_loop.ll",        5u,
                                                  "regular_nested_loop.ll", 8u,
                                                  "exit_call_loop.ll",      5u};

INSTANTIATE_TEST_CASE_P(DefaultInstance, PostAnnotationNestedTest,
                        ::testing::ValuesIn(testData3));

} // namespace testing
} // namespace icsa
