
#ifndef ANNOTATELOOPS_HPP
#define ANNOTATELOOPS_HPP

#include <cstdint>
// using std::uint32_t

namespace llvm {
class Loop;
class LoopInfo;
class Metadata;
class MDTuple;
} // namespace llvm end

namespace icsa {

struct AnnotateLoops {
  using LoopID_t = std::uint32_t;

  AnnotateLoops(unsigned int loopDepthThreshold = 1, LoopID_t startId = 1,
                LoopID_t idInterval = 1)
      : m_loopDepthThreshold(loopDepthThreshold), m_currentId(startId),
        m_idInterval(idInterval) {}

  void annotateWithId(llvm::Loop &CurLoop);
  void annotateWithId(llvm::LoopInfo &LI);

  bool hasAnnotatedId(const llvm::Loop &CurLoop) const;
  LoopID_t getAnnotatedId(const llvm::Loop &CurLoop) const;
  LoopID_t getId() const { return m_currentId; }

private:
  const llvm::Metadata *getAnnotatedIdNode(const llvm::Metadata *node) const;
  const llvm::MDTuple *getAnnotatedIdNode(const llvm::Loop &CurLoop) const;

  LoopID_t m_currentId;
  const LoopID_t m_idInterval;
  const unsigned int m_loopDepthThreshold;
  const char *m_idKey = "icsa.dynapar.loop.id";
};

} // namespace icsa end

#endif // ANNOTATELOOPS_HPP
