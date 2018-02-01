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
namespace {

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
  AnnotateLoops::LoopID_t startId = 2;
  AnnotateLoops al{startId, 3};

  parseAssemblyFile(td.assemblyFile);
  auto LI = calculateLoopInfo(*m_Module->begin());

  for (auto *e : LI)
    al.annotateWithId(*e);

  auto *curLoop = *LI.begin();

  EXPECT_EQ(al.hasAnnotatedId(*curLoop), true);
  EXPECT_EQ(al.getId(), td.nextId);
  EXPECT_EQ(al.getAnnotatedId(*curLoop), startId);
}

std::array<AnnotateLoopsTestData, 3> testData2 = {"regular_loop.ll",        5u,
                                                  "regular_nested_loop.ll", 5u,
                                                  "exit_call_loop.ll",      5u};

INSTANTIATE_TEST_CASE_P(DefaultInstance, PostAnnotationTest,
                        testing::ValuesIn(testData2));

//

class PostAnnotationNestedTest
    : public TestIRAssemblyParser,
      public testing::TestWithParam<AnnotateLoopsTestData> {};

TEST_P(PostAnnotationNestedTest, PostAnnotationNested) {
  auto td = GetParam();
  AnnotateLoops::LoopID_t startId = 2;
  AnnotateLoops al{startId, 3};

  parseAssemblyFile(td.assemblyFile);
  auto LI = calculateLoopInfo(*m_Module->begin());

  for (auto *e : LI) {
    al.annotateWithId(*e);

    for (auto k : e->getSubLoops())
      if (k->getLoopDepth() <= 2)
        al.annotateWithId(*k);
  }

  auto *curLoop = *LI.begin();

  EXPECT_EQ(al.hasAnnotatedId(*curLoop), true);
  EXPECT_EQ(al.getId(), td.nextId);
  EXPECT_EQ(al.getAnnotatedId(*curLoop), startId);
}

std::array<AnnotateLoopsTestData, 3> testData3 = {"regular_loop.ll",        5u,
                                                  "regular_nested_loop.ll", 8u,
                                                  "exit_call_loop.ll",      5u};

INSTANTIATE_TEST_CASE_P(DefaultInstance, PostAnnotationNestedTest,
                        testing::ValuesIn(testData3));

} // namespace anonymous end
} // namespace icsa end
