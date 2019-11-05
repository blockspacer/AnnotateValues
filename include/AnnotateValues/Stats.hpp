//
//
//

#pragma once

#include "AnnotateValues/Config.hpp"

#include "AnnotateValues/AnnotateInstructions.hpp"

#include <vector>
// using std::vector

#include <string>
// using std::string

namespace icsa {

struct AnnotateInstructionsStats {
  explicit operator bool() const noexcept { return Functions.size(); }

  void addProcessedFunction(const std::string &FuncName) noexcept(
      noexcept(Functions.emplace_back(FuncName))) {
    Functions.emplace_back(FuncName);
  }

  bool save(const std::string &Filename) const;

private:
  std::vector<std::string> Functions;
};

} // namespace icsa

