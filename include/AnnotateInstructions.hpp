//
//
//

#ifndef ANNOTATEINSTRUCTIONS_HPP
#define ANNOTATEINSTRUCTIONS_HPP

#include "Config.hpp"

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include <string>
// using std::string

#include <cstdint>
// using std::uint32_t

namespace llvm {
class Instruction;
class Metadata;
class MDTuple;
} // namespace llvm end

namespace icsa {
namespace AnnotateInstructions {

using InstructionIDTy = std::uint32_t;
using IDKeyTy = std::string;

const std::string DefaultKey{"icsa.dynapar.instruction.id"};

//

struct ReaderWriterBase {
  ReaderWriterBase(llvm::StringRef IDKey) noexcept : IDKey(IDKey.str()) {}
  llvm::StringRef key() const noexcept { return IDKey; }

protected:
  IDKeyTy IDKey;
};

//

struct Reader : ReaderWriterBase {
  Reader(llvm::StringRef IDKey = DefaultKey) noexcept
      : ReaderWriterBase(IDKey) {}

  bool has(const llvm::Instruction &CurInstruction) const noexcept;
  InstructionIDTy get(const llvm::Instruction &CurInstruction) const noexcept;
};

//

struct Writer : ReaderWriterBase {
  Writer(InstructionIDTy StartID = 1, InstructionIDTy IDInterval = 1,
         llvm::StringRef IDKey = DefaultKey) noexcept : ReaderWriterBase(IDKey),
                                                        CurrentID(StartID),
                                                        IDInterval(IDInterval) {
  }

  Writer(const Writer &) = delete;
  Writer(Writer &&) = default;

  InstructionIDTy put(llvm::Instruction &CurInstruction) noexcept;
  InstructionIDTy current() const noexcept { return CurrentID; }

private:
  void next() noexcept { CurrentID += IDInterval; }

  InstructionIDTy CurrentID;
  const InstructionIDTy IDInterval;
};

} // namespace AnnotateInstructions
} // namespace icsa

#endif // header
