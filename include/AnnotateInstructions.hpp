//
//
//

#ifndef ANNOTATEINSTRUCTIONS_HPP
#define ANNOTATEINSTRUCTIONS_HPP

#include "Config.hpp"

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include <cstdint>
// using std::uint32_t

namespace llvm {
class Instruction;
class Metadata;
class MDTuple;
} // namespace llvm end

namespace icsa {

struct AnnotateInstructions {
  using InstructionIDTy = std::uint32_t;

  AnnotateInstructions(
      InstructionIDTy StartID = 1, InstructionIDTy IDInterval = 1,
      llvm::StringRef IDKey = "icsa.dynapar.instruction.id") noexcept
      : CurrentID(StartID),
        IDInterval(IDInterval),
        IDKey(IDKey.str()) {}

  AnnotateInstructions(const AnnotateInstructions &) = delete;
  AnnotateInstructions(AnnotateInstructions &&) = delete;

  void annotate(llvm::Instruction &CurInstruction) noexcept;
  bool has(const llvm::Instruction &CurInstruction) const noexcept;
  InstructionIDTy get(const llvm::Instruction &CurInstruction) const noexcept;
  InstructionIDTy current() const noexcept { return CurrentID; }
  llvm::StringRef key() const noexcept { return IDKey; }

private:
  void next() noexcept { CurrentID += IDInterval; }

  InstructionIDTy CurrentID;
  const InstructionIDTy IDInterval;
  const std::string IDKey;
};

} // namespace icsa

#endif // header
