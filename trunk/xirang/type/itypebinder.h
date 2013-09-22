#ifndef AIO_XIRANG_ITYPE_BINDER_H
#define AIO_XIRANG_ITYPE_BINDER_H

#include <xirang/object.h>
#include <typeinfo>

namespace xirang {namespace type {
    /// this class design a portable C++ type info
	struct TypeInfoHandle
	{
        /// compare two type info, return true for same underlying C++ type
		virtual bool equal(const TypeInfoHandle& other) const = 0;
	protected:
		virtual ~TypeInfoHandle();
	};

	template<typename T>
	struct TypeInfo : TypeInfoHandle
	{
        TypeInfo():TypeInfoHandle(){}
		virtual bool equal(const TypeInfoHandle& other) const 
		{
			return this == &other
				|| typeid(*this) == typeid(other); 
		}
	};


    /// defines optional type methods 
	struct MethodsExtension
	{
        /// compare two ConstCommonObject, return 0 if equal, > 0 for great than and <0 for less than
		int (*compare)(ConstCommonObject lhs,ConstCommonObject rhs);

        /// return the hash code for given object
		std::size_t (*hash)(ConstCommonObject lhs);
	};

    /// define general methods to manipulate object
	class TypeMethods
	{
    public:

        /// exception tag to identify the type with unresolved parameters
		AIO_EXCEPTION_TYPE(unresolved_type);

        /// construct given memory block
        /// \pre obj.valid() && obj.data() must point to uninitialized memory block
        /// \throw it can throw any exception. even exception thrown, it still need to clear resources.
		virtual void construct(CommonObject obj, heap& inner, ext_heap& outer) const;

        /// destroy given object
        /// \pre obj.valid() && obj.data() must point to a constructed object
        /// \throw nothrow
		virtual void destruct(CommonObject obj) const;

        /// assign the pointed object from src to  dest
        /// \pre src.valid() && dest.valid() && both are initialized
		virtual void assign(ConstCommonObject src, CommonObject dest) const;

        /// init the type metadata, all parameters are output. caller 
        /// \param payload hold the size of type
        /// \param offset hold the offset of member,
        /// \note for primitive type, 
		virtual void beginLayout(std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const;

        /// caculate the metadata against given item. all paramters are in/out parameters.
        /// \param payload 
        /// \pre item.valid()
		virtual void nextLayout(TypeItem& item, std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const;

        /// get the info of current type
		virtual const TypeInfoHandle& typeinfo() const;

        /// get the optional methods
		virtual const MethodsExtension* extension() const;

		virtual ~TypeMethods();
	};

    /// get the defalut methods, default method support compound type.
	extern TypeMethods& DefaultMethods();
}}

#endif //end AIO_XIRANG_ITYPE_BINDER_H
