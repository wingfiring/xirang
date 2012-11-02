#ifndef XIRANG_XRFWD_H
#define XIRANG_XRFWD_H

#include <aio/xirang/xrbase.h>
#include <iterator>

namespace xirang
{
  class TypeMethods;
  extern TypeMethods& DefaultMethods();

  
  //handle of a type member item owned by Type.
  class TypeItem;
  typedef RangeT<aio::const_itr_traits<TypeItem> > TypeItemRange;

  class TypeArg;
  typedef RangeT<aio::const_itr_traits<TypeArg> > TypeArgRange;

  //handle of a type owned by Xirang
  class Type;
  typedef RangeT<aio::const_itr_traits<Type> >  TypeRange;

  class TypeAlias;
  typedef RangeT<aio::const_itr_traits<TypeAlias> > TypeAliasRange;

  class Namespace;
  typedef RangeT<aio::const_itr_traits<Namespace> > NamespaceRange;

  //handle of a subobject of object woned by Xirang.
  class SubObject;
  typedef RangeT<aio::const_itr_traits<SubObject> > SubObjRange;

  //handle of an object owned by Xirang
  class CommonObject;

  struct ConstNameValuePair;
  struct NameValuePair;

  typedef RangeT<aio::const_itr_traits<NameValuePair> > ObjectRange;

  class ConstSubObject;
  typedef RangeT<aio::const_itr_traits<ConstSubObject> > ConstSubObjRange;

  class ConstCommonObject;
  typedef RangeT<aio::const_itr_traits<ConstCommonObject> > ConstObjectRange;

  class Xirang;			//runtime

  typedef int ErrorCode;

  class TypeBuilder;
  class NamespaceBuilder;
  class TypeAliasBuilder;

  template<typename Imp>
  class ImpAccessor;

  inline int comparePtr(const void* p1, const void* p2) {
      size_t d = reinterpret_cast<const char*>(p1) - reinterpret_cast<const char*>(p2);
      if (d < 0)
          return -1;
      if (d > 0)
          return 1;
      return 0;
  }
}



#define DEFINE_COMPARE(type)\
inline bool operator==(const type& lhs, const type& rhs)\
{ return lhs.compare(rhs) == 0;}\
inline bool operator!=(const type& lhs, const type& rhs)\
{ return lhs.compare(rhs) != 0;}\
inline bool operator<(const type& lhs, const type& rhs)\
{ return lhs.compare(rhs) < 0;}\
inline bool operator<=(const type& lhs, const type& rhs)\
{ return lhs.compare(rhs) <= 0;}\
inline bool operator>(const type& lhs, const type& rhs)\
{ return lhs.compare(rhs) > 0;}\
inline bool operator>=(const type& lhs, const type& rhs)\
{ return lhs.compare(rhs) >= 0;}

#endif				//end XIRANG_XRFWD_H
