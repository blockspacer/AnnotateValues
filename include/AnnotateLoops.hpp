//
//
//

#ifndef ANNOTATELOOPS_HPP
#define ANNOTATELOOPS_HPP

#include "Config.hpp"

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include <string>
// using std::string

#include <cstdint>
// using std::uint32_t

namespace llvm {
class Loop;
class Metadata;
class MDTuple;
} // namespace llvm end

namespace icsa {

struct AnnotateLoops {
  using LoopIDTy = std::uint32_t;

  AnnotateLoops(LoopIDTy StartID = 1, LoopIDTy IDInterval = 1,
                llvm::StringRef IDKey = "icsa.dynapar.loop.id")
      : CurrentID(StartID), IDInterval(IDInterval), IDKey(IDKey.str()) {}

  LoopIDTy annotate(llvm::Loop &CurLoop);

  bool has(const llvm::Loop &CurLoop) const;
  LoopIDTy get(const llvm::Loop &CurLoop) const;
  LoopIDTy current() const { return CurrentID; }
  llvm::StringRef key() const noexcept { return IDKey; }

private:
  void next() noexcept { CurrentID += IDInterval; }

  const llvm::Metadata *getNode(const llvm::Metadata *node) const;
  const llvm::MDTuple *getNode(const llvm::Loop &CurLoop) const;

  LoopIDTy CurrentID;
  const LoopIDTy IDInterval;
  const std::string IDKey;
};

} // namespace icsa

#endif // header
