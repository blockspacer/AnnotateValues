
#ifndef ANNOTATELOOPS_HPP
#define ANNOTATELOOPS_HPP

namespace llvm {
class Loop;
class LoopInfo;
class Metadata;
class MDTuple;
} // namespace llvm end

namespace icsa {

struct AnnotateLoops {
  AnnotateLoops(unsigned int loopDepthThreshold = 1, unsigned int startId = 1,
                unsigned int idInterval = 1)
      : m_loopDepthThreshold(loopDepthThreshold), m_currentId(startId),
        m_idInterval(idInterval) {}

  void annotateWithId(llvm::Loop &CurLoop);
  void annotateWithId(llvm::LoopInfo &LI);

  bool hasAnnotatedId(const llvm::Loop &CurLoop) const;
  unsigned int getAnnotatedId(const llvm::Loop &CurLoop) const;
  unsigned int getId() const { return m_currentId; }

private:
  const llvm::Metadata *getAnnotatedIdNode(const llvm::Metadata *node) const;
  const llvm::MDTuple *getAnnotatedIdNode(const llvm::Loop &CurLoop) const;

  unsigned int m_currentId;
  const unsigned int m_idInterval;
  const unsigned int m_loopDepthThreshold;
  const char *m_idKey = "icsa.dynapar.loop.id";
};

} // namespace icsa end

#endif // ANNOTATELOOPS_HPP
