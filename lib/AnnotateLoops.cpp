//
//
//

#include "llvm/IR/Type.h"
// using llvm::Module

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Constants.h"
// using llvm::Constant;
// using llvm::ConstantInt;

#include "llvm/IR/LLVMContext.h"
// using llvm::LLVMContext;

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass;
// using llvm::LoopInfo;
// using llvm::Loop;

#include "llvm/IR/Metadata.h"
// using llvm::Metadata
// using llvm::MDNode

#include "llvm/IR/MDBuilder.h"
// using llvm::MDBuilder

#include "llvm/IR/Constants.h"
// using llvm::ConstantInt

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/Debug.h"
// using DEBUG macro
// using llvm::dbgs

#include "AnnotateLoops.hpp"


//bool AnnotateLoopsPass::runOnModule(llvm::Module &CurModule) {
  //llvm::MDBuilder LoopMDBuilder(CurModule.getContext());

  //for (auto &CurFunc : CurModule) {
    //if (CurFunc.isDeclaration())
      //continue;

    //const auto &LIPass = Pass::getAnalysis<llvm::LoopInfoWrapperPass>(CurFunc);
    //const auto &LI = LIPass.getLoopInfo();
    //for (const auto *CurLoop : LI) {
      //llvm::SmallVector<llvm::Metadata *, 2> LoopIDVals;

      //LoopIDVals.push_back(LoopMDBuilder.createString("icsa.dynapar.loop.id"));
      //auto *IntType = llvm::Type::getInt32Ty(CurModule.getContext());
      //LoopIDVals.push_back(LoopMDBuilder.createConstant(
          //llvm::ConstantInt::get(IntType, m_LoopID)));

      //auto *const LoopIDMD =
          //llvm::MDNode::get(CurModule.getContext(), LoopIDVals);

      //llvm::SmallVector<llvm::Metadata *, 4> MDs;
      //MDs.push_back(nullptr); // reserve the first position for self
      //MDs.push_back(LoopIDMD);

      //auto *LoopMD = CurLoop->getLoopID();

      //if (LoopMD) {
        //for (auto i = 0; i < LoopMD->getNumOperands(); ++i)
          //MDs.push_back(LoopMD->getOperand(i));
      //}

      //auto NewLoopIDMD = llvm::MDNode::get(CurModule.getContext(), MDs);
      //NewLoopIDMD->replaceOperandWith(0, NewLoopIDMD);

      //CurLoop->setLoopID(NewLoopIDMD);
    //} // loopinfo loop end
  //}   // func loop end

  //return false;
//}

