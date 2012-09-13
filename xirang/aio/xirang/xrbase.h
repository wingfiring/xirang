#ifndef XIRANG_XRBASE_H
#define XIRANG_XRBASE_H

#include <aio/common/range.h>
#include <aio/common/string.h>
#include <aio/common/iterator.h>
#include <aio/common/buffer.h>

namespace xirang
{
  using aio::string;
  using aio::heap;
  using aio::ext_heap;
  typedef aio::ext_heap::handle handle;
  typedef aio::byte byte;

  template<typename T>
  struct RangeT : public  aio::range<aio::IteratorT<T> >
  {	
	  typedef aio::range<aio::IteratorT<T> > base;
	  typedef typename base::iterator iterator;
      RangeT() {};
	  RangeT(const iterator& first, const iterator& last) : base(first, last){}

  };
}

#endif //end XIRANG_XRBASE_H

