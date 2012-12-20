#ifndef AIO_COMMON_XIRANG_BINDER_H
#define AIO_COMMON_XIRANG_BINDER_H

#include <aio/xirang/object.h>
#include <aio/xirang/itypebinder.h>

#include <aio/common/context_except.h>

namespace xirang
{
    ///  exception for type mismatch
	AIO_EXCEPTION_TYPE(TypeMismatchException);
    /// Bind T* to const CommonObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T > T * bind (const CommonObject * obj) 
	{
		AIO_PRE_CONDITION(obj && obj->valid());

		Type t = obj->type();
		AIO_PRE_CONDITION(t.valid());
        
		const static TypeInfo<T> info;
		return info.equal(t.methods().typeinfo())
			? reinterpret_cast<T*>(obj->data())
			: 0;
	}

    /// Bind T& to const CommonObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T > T & bind (const CommonObject & obj) 
	{
		AIO_PRE_CONDITION(obj.valid());

		T* p = bind<T>(&obj);

		if (!p)
			AIO_THROW(TypeMismatchException);
		return *p;
	}


    /// Bind T* to const CommonObject pointer. no type check.
    /// \pre obj && obj->valid()
	template < typename T > T * uncheckBind (const CommonObject * obj) 
	{
		AIO_PRE_CONDITION(obj && obj->valid());

		return reinterpret_cast<T*>(obj->data());
	}

    /// Bind T& to const CommonObject&. no type check.
    /// \pre obj->valid()
	template < typename T > T & uncheckBind (const CommonObject & obj) 
	{
		AIO_PRE_CONDITION(obj.valid());

		return *reinterpret_cast<T*>(obj.data());
	}

    /// Bind T* to const SubObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T > T * bind (const SubObject * obj) 
	{
		AIO_PRE_CONDITION(obj && obj->valid());

		Type t = obj->type();
		AIO_PRE_CONDITION(t.valid());

		const static TypeInfo<T> info;
		return info.equal(t.methods().typeinfo())
			? reinterpret_cast<T*>(obj->data())
			: 0;
	}

    /// Bind T& to const SubObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T > T & bind (const SubObject & obj) 
	{
		AIO_PRE_CONDITION(obj.valid());

		T* p = bind<T>(&obj);

		if (!p)
		{
			AIO_THROW(TypeMismatchException);
		}
		return *p;
	}

    /// Bind T* to const SubObject pointer. no type check.
    /// \pre obj && obj->valid()
	template < typename T > T * uncheckBind (const SubObject * obj) 
	{
		AIO_PRE_CONDITION(obj && obj->valid());

		return reinterpret_cast<T*>(obj->data());
	}

    /// Bind T& to const SubObject&. no type check.
    /// \pre obj->valid()
	template < typename T > T & uncheckBind (const SubObject & obj) 
	{
		AIO_PRE_CONDITION(obj.valid());

		return *reinterpret_cast<T*>(obj.data());
	}


    /// Bind const T* to const ConstCommonObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T > const T * bind (const ConstCommonObject * obj) 
	{
		AIO_PRE_CONDITION(obj && obj->valid());

		Type t = obj->type();
		AIO_PRE_CONDITION(t.valid());

		const static TypeInfo<T> info;
		return info.equal(t.methods().typeinfo())
			? reinterpret_cast<const T*>(obj->data())
			: 0;
	}

    /// Bind const T& to const ConstCommonObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T > const T & bind (const ConstCommonObject & obj) 
	{
		AIO_PRE_CONDITION(obj.valid());

		const T* p = bind<T>(&obj);

		if (!p)
		{
			AIO_THROW(TypeMismatchException);
		}
		return *p;
	}

    /// Bind const T* to const ConstCommonObject pointer. no type check.
    /// \pre obj && obj->valid()
	template < typename T > const T * uncheckBind (const ConstCommonObject * obj) 
	{
		AIO_PRE_CONDITION(obj && obj->valid());

		return reinterpret_cast<const T*>(obj->data());
	}

    /// Bind const T& to const ConstCommonObject&. no type check.
    /// \pre obj->valid()
	template < typename T > const T & uncheckBind (const ConstCommonObject & obj) 
	{
		AIO_PRE_CONDITION(obj.valid());

		return *reinterpret_cast<const T*>(obj.data());
	}

    /// Bind const T* to const ConstSubObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T > const T * bind (const ConstSubObject * obj) 
	{
		AIO_PRE_CONDITION(obj && obj->valid());

		Type t = obj->type();
		AIO_PRE_CONDITION(t.valid());

		const static TypeInfo<T> info;
		return info.equal(t.methods().typeinfo())
			? reinterpret_cast<const T*>(obj->data())
			: 0;
	}

    /// Bind const T& to const ConstSubObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T > const T & bind (const ConstSubObject & obj) 
	{
		AIO_PRE_CONDITION(obj.valid());

		const T* p = bind<T>(&obj);

		if (!p)
		{
			AIO_THROW(TypeMismatchException);
		}
		return *p;
	}

    /// Bind const T* to const ConstSubObject pointer. no type check.
    /// \pre obj && obj->valid()
	template < typename T > const T * uncheckBind (const ConstSubObject * obj) 
	{
		AIO_PRE_CONDITION(obj && obj->valid());

		return reinterpret_cast<const T*>(obj->data());
	}

    /// Bind T& to const SubObject&. no type check.
    /// \pre obj->valid()
	template < typename T > const T & uncheckBind (const ConstSubObject & obj) 
	{
		AIO_PRE_CONDITION(obj.valid());

		return *reinterpret_cast<const T*>(obj.data());
	}

#ifdef _DEBUG
#define SELECTED_BIND_FUNCTION bind
#else
#define SELECTED_BIND_FUNCTION uncheckBind
#endif
    /// Bind T* to const CommonObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T, typename ObjType > T * fastBind (const ObjType * obj) 
	{
        return SELECTED_BIND_FUNCTION(obj);
	}

    /// Bind T& to const CommonObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T, typename ObjType >  T & fastBind (const ObjType & obj) 
	{
        return SELECTED_BIND_FUNCTION(obj);
	}
#undef SELECTED_BIND_FUNCTION

}
#endif				//end AIO_COMMON_XIRANG_BINDER_H
