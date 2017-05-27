
#ifndef ANNOTATELOOPS_HPP
#define ANNOTATELOOPS_HPP

namespace llvm {
class Loop;
class LoopInfo;
} // namespace llvm end

namespace icsa {

struct AnnotateLoops {
  AnnotateLoops(unsigned int loopDepthThreshold = 1, unsigned int startId = 1,
                unsigned int idInterval = 1)
      : m_loopDepthThreshold(loopDepthThreshold), m_currentId(startId),
        m_idInterval(idInterval) {}

  void annotateWithId(llvm::Loop &CurLoop);
  void annotateWithId(llvm::LoopInfo &LI);

  unsigned int getId() const { return m_currentId; }

private:
  unsigned int m_currentId;
  const unsigned int m_idInterval;
  const unsigned int m_loopDepthThreshold;
};

} // namespace icsa end

#endif // ANNOTATELOOPS_HPP
