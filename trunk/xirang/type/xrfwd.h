#ifndef XIRANG_XRFWD_H
#define XIRANG_XRFWD_H

#include <xirang/type/xrbase.h>
#include <iterator>

namespace xirang { namespace type{
  class TypeMethods;
  extern TypeMethods& DefaultMethods();

  struct Serializer;
  struct DeSerializer;


  //handle of a type member item owned by Type.
  class TypeItem;
  typedef BiRangeT<const_itr_traits<TypeItem> > TypeItemRange;

  class TypeArg;
  typedef BiRangeT<const_itr_traits<TypeArg> > TypeArgRange;

  //handle of a type owned by Xirang
  class Type;
  typedef BiRangeT<const_itr_traits<Type> >  TypeRange;

  class TypeAlias;
  typedef BiRangeT<const_itr_traits<TypeAlias> > TypeAliasRange;

  class Namespace;
  typedef BiRangeT<const_itr_traits<Namespace> > NamespaceRange;

  //handle of a subobject of object woned by Xirang.
  class SubObject;
  typedef BiRangeT<const_itr_traits<SubObject> > SubObjRange;

  //handle of an object owned by Xirang
  class CommonObject;

  struct ConstNameValuePair;
  struct NameValuePair;

  typedef BiRangeT<const_itr_traits<NameValuePair> > ObjectRange;

  class ConstSubObject;
  typedef BiRangeT<const_itr_traits<ConstSubObject> > ConstSubObjRange;

  class ConstCommonObject;
  typedef BiRangeT<const_itr_traits<ConstCommonObject> > ConstObjectRange;

  class Xirang;			//runtime

  typedef int ErrorCode;

  class TypeBuilder;
  class NamespaceBuilder;
  class TypeAliasBuilder;

  template<typename Imp>
  class ImpAccessor;

  inline int comparePtr(const void* p1, const void* p2) {
	  if (p1 < p2)
          return -1;
	  if (p1 > p2)
          return 1;
      return 0;
  }

  template<typename T> struct assigner;
	template<typename T> struct constructor;
	template<typename T> struct destructor;
	template<typename T> struct comparison;
	template<typename T> struct hasher;
	template<typename T> struct layout;
	template<typename T> struct extendMethods;

	template<typename T> constructor<T> get_constructor(T*);
	template<typename T> destructor<T> get_destructor(T*);
	template<typename T> assigner<T> get_assigner(T*);
	template<typename T> layout<T> get_layout(T*);
	template<typename T> extendMethods<T> get_extendMethods(T*);
}}



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
