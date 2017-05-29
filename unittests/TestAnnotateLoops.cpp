
#include <memory>
// using std::unique_ptr

#include <map>
// using std::map

#include <cassert>
// using assert

#include <cstdlib>
// using std::abort

#include "llvm/IR/LLVMContext.h"
// using llvm::LLVMContext

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/LegacyPassManager.h"
// using llvm::legacy::PassMananger

#include "llvm/Pass.h"
// using llvm::Pass
// using llvm::PassInfo

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass
// using llvm::LoopInfo

#include "llvm/Support/SourceMgr.h"
// using llvm::SMDiagnostic

#include "llvm/AsmParser/Parser.h"
// using llvm::parseAssemblyString

#include "llvm/IR/Verifier.h"
// using llvm::verifyModule

#include "llvm/Support/ErrorHandling.h"
// using llvm::report_fatal_error

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_string_ostream

#include "gtest/gtest.h"
// using testing::Test

#include "boost/variant.hpp"
// using boost::variant

#include "AnnotateLoops.hpp"

namespace icsa {
namespace {

using test_result_t = boost::variant<bool, unsigned int>;
using test_result_map = std::map<std::string, test_result_t>;

struct test_result_visitor : public boost::static_visitor<unsigned int> {
  unsigned int operator()(bool b) const { return b ? 1 : 0; }
  unsigned int operator()(unsigned int i) const { return i; }
};

class TestAnnotateLoops : public testing::Test {
public:
  enum struct AssemblyHolderType : int { FILE_TYPE, STRING_TYPE };

  TestAnnotateLoops() : m_Module{nullptr}, m_TestDataDir{"./unittests/data/"} {}

  void ParseAssembly(
      const char *AssemblyHolder,
      const AssemblyHolderType asmHolder = AssemblyHolderType::FILE_TYPE) {
    llvm::SMDiagnostic err;

    if (AssemblyHolderType::FILE_TYPE == asmHolder) {
      std::string fullFilename{m_TestDataDir};
      fullFilename += AssemblyHolder;

      m_Module =
          llvm::parseAssemblyFile(fullFilename, err, llvm::getGlobalContext());

    } else {
      m_Module = llvm::parseAssemblyString(AssemblyHolder, err,
                                           llvm::getGlobalContext());
    }

    std::string errMsg;
    llvm::raw_string_ostream os(errMsg);
    err.print("", os);

    if (llvm::verifyModule(*m_Module, &(llvm::errs())))
      llvm::report_fatal_error("module verification failed\n");

    if (!m_Module)
      llvm::report_fatal_error(os.str().c_str());

    return;
  }

  void ExpectTestPass(const test_result_map &trm) {
    static char ID;

    struct UtilityPass : public llvm::FunctionPass {
      UtilityPass(const test_result_map &trm)
          : llvm::FunctionPass(ID), m_trm(trm) {}

      static int initialize() {
        auto *registry = llvm::PassRegistry::getPassRegistry();

        auto *PI = new llvm::PassInfo("Utility pass for unit tests", "", &ID,
                                      nullptr, true, true);

        registry->registerPass(*PI, false);
        llvm::initializeLoopInfoWrapperPassPass(*registry);

        return 0;
      }

      void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.setPreservesAll();
        AU.addRequiredTransitive<llvm::LoopInfoWrapperPass>();

        return;
      }

      bool runOnFunction(llvm::Function &F) override {
        if (!F.hasName() || !F.getName().startswith("test"))
          return false;

        auto &LI = getAnalysis<llvm::LoopInfoWrapperPass>().getLoopInfo();
        // LI.print(llvm::outs());

        auto *CurLoop = *LI.begin();
        assert(CurLoop && "Loop ptr is invalid");

        test_result_map::const_iterator found;

        AnnotateLoops al1{1, 2, 3};
        AnnotateLoops al2{2, 2, 3};

        // subcase
        found = lookup("has loop id prior to annotation");
        if (std::end(m_trm) != found) {
          const auto &rv = al1.hasAnnotatedId(*CurLoop);
          const auto &ev =
              boost::apply_visitor(test_result_visitor(), found->second);
          EXPECT_EQ(ev, rv) << found->first;
        }

        al1.annotateWithId(LI);
        const auto annotateId = al1.getAnnotatedId(*CurLoop);

        al2.annotateWithId(LI);

        // subcase
        found = lookup("has loop id after annotation");
        if (std::end(m_trm) != found) {
          const auto &rv = al1.hasAnnotatedId(*CurLoop);
          const auto &ev =
              boost::apply_visitor(test_result_visitor(), found->second);
          EXPECT_EQ(ev, rv) << found->first;
        }

        // subcase
        found = lookup("single loop id next annotation");
        if (std::end(m_trm) != found) {
          const auto &rv = al1.getId();
          const auto &ev =
              boost::apply_visitor(test_result_visitor(), found->second);
          EXPECT_EQ(ev, rv) << found->first;
        }

        // subcase
        found = lookup("inner loop id next annotation");
        if (std::end(m_trm) != found) {
          const auto &rv = al2.getId();
          const auto &ev =
              boost::apply_visitor(test_result_visitor(), found->second);
          EXPECT_EQ(ev, rv) << found->first;
        }

        // subcase
        found = lookup("top loop id current annotation");
        if (std::end(m_trm) != found) {
          const auto &ev =
              boost::apply_visitor(test_result_visitor(), found->second);
          EXPECT_EQ(ev, annotateId) << found->first;
        }

        return false;
      }

      test_result_map::const_iterator lookup(const std::string &subcase,
                                             bool fatalIfMissing = false) {
        auto found = m_trm.find(subcase);
        if (fatalIfMissing && m_trm.end() == found) {
          llvm::errs() << "subcase: " << subcase << " test data not found\n";
          std::abort();
        }

        return found;
      }

      const test_result_map &m_trm;
    };

    static int init = UtilityPass::initialize();
    (void)init; // do not optimize away

    auto *P = new UtilityPass(trm);
    llvm::legacy::PassManager PM;

    PM.add(P);
    PM.run(*m_Module);

    return;
  }

protected:
  std::unique_ptr<llvm::Module> m_Module;
  const char *m_TestDataDir;
};

TEST_F(TestAnnotateLoops, RegularLoop) {
  ParseAssembly("test01.ll");

  test_result_map trm;

  trm.insert({"has loop id prior to annotation", false});
  trm.insert({"has loop id after annotation", true});
  trm.insert({"single loop id next annotation", 5u});
  trm.insert({"inner loop id next annotation", 5u});
  trm.insert({"top loop current id annotation", 2u});
  ExpectTestPass(trm);
}

TEST_F(TestAnnotateLoops, RegularInnerLoop) {
  ParseAssembly("test02.ll");

  test_result_map trm;

  trm.insert({"has loop id prior to annotation", false});
  trm.insert({"has loop id after annotation", true});
  trm.insert({"single loop id next annotation", 5u});
  trm.insert({"inner loop id next annotation", 8u});
  trm.insert({"top loop current id annotation", 2u});
  ExpectTestPass(trm);
}

TEST_F(TestAnnotateLoops, ExitCallLoop) {
  ParseAssembly("test03.ll");

  test_result_map trm;

  trm.insert({"has loop id prior to annotation", false});
  trm.insert({"has loop id after annotation", true});
  trm.insert({"single loop id next annotation", 5u});
  trm.insert({"inner loop id next annotation", 5u});
  trm.insert({"top loop current id annotation", 2u});
  ExpectTestPass(trm);
}

} // namespace anonymous end
} // namespace icsa end
