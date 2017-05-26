
#ifndef ANNOTATELOOPS_HPP
#define ANNOTATELOOPS_HPP

namespace llvm {
class Loop;
class LoopInfo;
} // namespace llvm end

namespace icsa {

struct AnnotateLoops {
  AnnotateLoops(unsigned int startId = 1, unsigned int idInterval = 1)
      : m_currentId(startId), m_idInterval(idInterval) {}

  void annotateWithId(llvm::Loop &CurLoop);
  void annotateWithId(llvm::LoopInfo &LI);

  unsigned int getId() const { return m_currentId; }

private:
  unsigned int m_currentId;
  unsigned int m_idInterval;
};

} // namespace icsa end

#endif // ANNOTATELOOPS_HPP
