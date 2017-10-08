
# LLVM Annotate Loops opt pass

This is a LLVM opt pass for annotating loops with numerical ID's.


## External Dependencies

None.


## How to Build

- make sure the required environment variables are exported (see `utils/scripts/build` directory):
  - compiler selection is catered by the `exports_local_*` scripts using the `CC` and `CXX` variables for my current 
   machine, so adjust appropriately for your setup.
  - export one of the `exports_deps_*` scripts, depending on the kind of setup you are interested in.
- `mkdir my-build-dir`
- optionally `mkdir my-install-dir`
- `[path to repo]/utils/build.sh [path torepo] [path to installation dir]`
- `cd my-build-dir`
- `make`
- optionally `make install`


## How to execute

### Using opt

- make sure LLVM's opt is in your `$PATH`
- `opt -load [path to plugin]/libLLVMAnnotateLoopsPass.so -annotate-loops foo.bc -o foo.out.bc`

### Using clang

- make sure LLVM's clang is in your `$PATH`
- `clang -Xclang -load -Xclang [path to plugin]/libLLVMAnnotateLoopsPass.so foo.c -o foo`


## Requirements

- Built and executed with:
  - LLVM 3.7.0
  - LLVM 3.8.0


## Notes

- When the build script uses LLVM's cmake utility functions the `lib` shared library prefix is omitted
