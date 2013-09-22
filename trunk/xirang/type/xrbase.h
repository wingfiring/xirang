#ifndef XIRANG_XRBASE_H
#define XIRANG_XRBASE_H

#include <xirang/range.h>
#include <xirang/string.h>
#include <xirang/iterator.h>
#include <xirang/buffer.h>
#include <xirang/context_except.h>

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

