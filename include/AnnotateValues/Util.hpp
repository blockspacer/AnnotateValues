//
//
//

#pragma once

#include "AnnotateValues/Config.hpp"

#include "llvm/IR/InstIterator.h"
// using llvm::inst_begin
// using llvm::inst_end

#include "llvm/ADT/iterator_range.h"
// using llvm::make_range

// preprocessor stringification macros
#define STRINGIFY_UTIL(x) #x
#define STRINGIFY(x) STRINGIFY_UTIL(x)

#define PRJ_CMDLINE_DESC(x) x " (version: " STRINGIFY(VERSION_STRING) ")"

//

namespace icsa {

template <typename T> decltype(auto) make_inst_range(T &&Unit) {
  return llvm::make_range(llvm::inst_begin(std::forward<T>(Unit)),
                          llvm::inst_end(std::forward<T>(Unit)));
}

template <typename T> bool is_range_empty(const T &Range) {
  return Range.begin() == Range.end();
}

} // namespace icsa

