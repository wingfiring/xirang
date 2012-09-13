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

namespace xirang
{
    struct Matrix44
    {
        double data[16];
        int compare (const Matrix44& rhs ) const
        {
            for (int i= 0; i < 16; ++i)
            {
                if (data[i] < rhs.data[i])
                    return -1;
                else if (data[i] > rhs.data[i])
                    return 1;
            }
            return 0;
        }
    };
    DEFINE_COMPARE(Matrix44);

    struct Color3
    {
        double r,g,b;
        int compare (const Color3& rhs ) const
        {
            double result =  r - rhs.r != 0
                ? r - rhs.r
                : g - rhs.g != 0 
                ? g - rhs.g
                : b - rhs.b;
            return result < 0
                ? -1
                : result == 0
                ? 0
                : 1;
        }
    };
    DEFINE_COMPARE(Color3);

    struct Color4
    {
        double r,g,b,a;
        int compare (const Color4& rhs ) const
        {
            double result = r - rhs.r != 0
                ? r - rhs.r
                : g - rhs.g != 0 
                ? g - rhs.g
                : b - rhs.b != 0
                ? b - rhs.b
                : a - rhs.a;

            return result < 0
                ? -1
                : result == 0
                ? 0
                : 1;
        }
    };
    DEFINE_COMPARE(Color4);

    struct UnitsValue
    {
		int units;
        double value;
        int compare (const UnitsValue& rhs ) const
        {
            return value - rhs.value < 0
                ? -1
                : value - rhs.value > 0
                ? 1
                : units - rhs.units;
        }
    };
    DEFINE_COMPARE(UnitsValue);

    struct Vector3
    {
        double data[3];
        int compare (const Vector3& rhs ) const
        {
            for (int i= 0; i < 3; ++i)
            {
                if (data[i] < rhs.data[i])
                    return -1;
                else if (data[i] > rhs.data[i])
                    return 1;
            }
            return 0;
        }
    };
    DEFINE_COMPARE(Vector3);

    struct Vector4
    {
        double data[4];
        int compare (const Vector4& rhs ) const
        {
            for (int i= 0; i < 4; ++i)
            {
                if (data[i] < rhs.data[i])
                    return -1;
                else if (data[i] > rhs.data[i])
                    return 1;
            }
            return 0;
        }
    };
    DEFINE_COMPARE(Vector4);
}

namespace std{
    namespace tr1{
        template<>
        struct is_pod<xirang::Matrix44>
        {
            static const bool value = true;
        };
        
        template<>
        struct is_pod<xirang::Color3>
        {
            static const bool value = true;
        };
        template<>
        struct is_pod<xirang::Color4>
        {
            static const bool value = true;
        };
        template<>
        struct is_pod<xirang::Vector3>
        {
            static const bool value = true;
        };
        template<>
        struct is_pod<xirang::Vector4>
        {
            static const bool value = true;
        };

        
        
    }
}
#endif				//end XIRANG_XRFWD_H
