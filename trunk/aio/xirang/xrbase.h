#ifndef XIRANG_XRBASE_H
#define XIRANG_XRBASE_H

#include <aio/common/range.h>
#include <aio/common/string.h>
#include <aio/common/iterator.h>
#include <aio/common/buffer.h>
#include <aio/common/context_except.h>

namespace xirang
{
  using aio::string;
  using aio::heap;
  using aio::ext_heap;
  typedef aio::ext_heap::handle handle;
  using aio::byte;

  template<typename T>
  struct BiRangeT : public  aio::range<aio::bidir_iterator<T> >
  {	
	  typedef aio::range<aio::bidir_iterator<T> > base;
	  typedef typename base::iterator iterator;
      BiRangeT() {};
	  BiRangeT(const iterator& first, const iterator& last) : base(first, last){}

  };
}

#endif //end XIRANG_XRBASE_H

