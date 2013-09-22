#ifndef XIRANG_SRC_XIRANG_IMP_ACCESSOR_H
#define XIRANG_SRC_XIRANG_IMP_ACCESSOR_H
#include <xirang/xrfwd.h>

namespace xirang{
  template<typename Imp>
  class ImpAccessor
  {
  public:
      template<typename T>
      static Imp* const & getImp(const T& t) { return t.m_imp;}
  };

}
#endif
