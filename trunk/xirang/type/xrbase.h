#ifndef XIRANG_XRBASE_H
#define XIRANG_XRBASE_H

#include <xirang/range.h>
#include <xirang/string.h>
#include <xirang/iterator.h>
#include <xirang/buffer.h>
#include <xirang/context_except.h>

namespace xirang{
  typedef ext_heap::handle handle;

  template<typename T>
  struct BiRangeT : public  range<bidir_iterator<T> >
  {
	  typedef range<bidir_iterator<T> > base;
	  typedef typename base::iterator iterator;
      BiRangeT() {};
	  BiRangeT(const iterator& first, const iterator& last) : base(first, last){}

  };
}

#endif //end XIRANG_XRBASE_H

