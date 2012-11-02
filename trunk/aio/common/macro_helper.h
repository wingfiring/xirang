//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_MACRO_HELPER_H
#define AIO_MACRO_HELPER_H

#define AIO_JOIN( X, Y ) AIO_DO_JOIN( X, Y )
#define AIO_DO_JOIN( X, Y ) AIO_DO_JOIN2(X,Y)
#define AIO_DO_JOIN2( X, Y ) X##Y

#ifndef AIO_STRING
#  define AIO_STRING_(x) #x
#  define AIO_STRING(x) AIO_STRING_(x) 
#endif

#ifndef AIO_WIDE_STRING
#  define AIO_WIDE_STRING_(str) L ## str
#  define AIO_WIDE_STRING(str) AIO_WIDE_STRING_(str)
#endif

#define AIO_MP(x) AIO_WIDE_STRING(AIO_STRING(x)), x

#ifndef AIO_EXCEPTION_SPEC_ENABLED

#  define AIO_COMPATIBLE_NOTHROW() throw()
#  define AIO_NOTHROW() 
#  define AIO_THROW0() 
#  define AIO_THROW1(e1)
#  define AIO_THROW2(e1, e2)
#  define AIO_THROW3(e1, e2, e3)
#  define AIO_THROW4(e1, e2, e3, e4)
#  define AIO_THROW5(e1, e2, e3, e4, e5)
#  define AIO_THROW6(e1, e2, e3, e4, e5, e6)
#  define AIO_THROW7(e1, e2, e3, e4, e5, e6, e7)
#  define AIO_THROW8(e1, e2, e3, e4, e5, e6, e7, e8)
#  define AIO_THROW9(e1, e2, e3, e4, e5, e6, e7, e8, e9)

#else

#  define AIO_COMPATIBLE_NOTHROW() throw()
#  define AIO_NOTHROW() throw()
#  define AIO_THROW0() throw()
#  define AIO_THROW1(e1) throw (e1)
#  define AIO_THROW2(e1, e2) throw (e1, e2)
#  define AIO_THROW3(e1, e2, e3) throw (e1, e2, e3)
#  define AIO_THROW4(e1, e2, e3, e4) throw (e1, e2, e3, e4)
#  define AIO_THROW5(e1, e2, e3, e4, e5) throw (e1, e2, e3, e4, e5)
#  define AIO_THROW6(e1, e2, e3, e4, e5, e6) throw (e1, e2, e3, e4, e5, e6)
#  define AIO_THROW7(e1, e2, e3, e4, e5, e6, e7) throw (e1, e2, e3, e4, e5, e6, e7)
#  define AIO_THROW8(e1, e2, e3, e4, e5, e6, e7, e8) throw (e1, e2, e3, e4, e5, e6, e7, e8)
#  define AIO_THROW9(e1, e2, e3, e4, e5, e6, e7, e8, e9) throw (e1, e2, e3, e4, e5, e6, e7, e8, e9)

#endif //end AIO_EXCEPTION_SPEC_ENABLED

#define DISABLE_CLONE(type)\
	private:\
	type(const type&);\
	type& operator=(const type&)

#endif //AIO_MACRO_HELPER_H

