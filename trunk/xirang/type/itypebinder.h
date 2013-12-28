#ifndef AIO_XIRANG_ITYPE_BINDER_H
#define AIO_XIRANG_ITYPE_BINDER_H

#include <xirang/type/object.h>
#include <typeinfo>
#include <xirang/versiontype.h>
#include <xirang/sha1/process_value.h>


namespace xirang {namespace type {

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

		/// serialize object. it alwasy save as exchangable binary format.
		/// \param wr	writer
		///	\param obj	object to seriaze
		/// \pre obj.valid()
		virtual void serialize(io::writer& wr, ConstCommonObject obj);

		/// deserialize object. it always loads from exchangable binary format.
		///	\param rd	reader
		///	\param obj	deserialize to obj
		/// \pre obj.valid() && obj is allocated but not constructed
		virtual void deserialize(io::reader& rd, CommonObject obj, heap& inner, ext_heap& outer);

		/// calculate object version
		///\pre &t.method() == this;
		virtual version_type getTypeVersion(Type t) const;

		/// calculate object version
		///\pre &t.method() == this;
		virtual void calculateTypeSha(sha1& sha, Type t) const;

        /// get the optional methods
		virtual const MethodsExtension* extension() const;

		/// internal id, intends to be used to identify each primitive type;
		virtual const string& internalID() const;

		virtual ~TypeMethods();
	};

    /// get the defalut methods, default method support compound type.
	extern TypeMethods& DefaultMethods();

	// for intermediary type, for example array<string> will be placed to /var/run/type/
	// the type name would be "array", the version is the result of  array<string>
	// If a content save the type, it should save all referenced types in  /var/run/type
	///\note This save method will save type reference only.
	template<typename Ar> Ar& save(Ar& ar, Type t){
		ar & uint8_t(t.valid()? 1 : 0);
		if (t.valid()){
			ar & t.fullName() & t.version();
		}
		return ar;
	}

	template<typename T, int Cateory = 0> struct TypeVersionOf{
		static const version_type value;
	};
#define XIRANG_DEFINE_TYPE_VERSION_OF(id, t, default_path)\
	template<int Category> struct TypeVersionOf<t, Category>{\
		static const version_type value;\
	};\
	template<int Category> const version_type TypeVersionOf<t, Category>::value = version_type(literal(id))

	template<typename Ar> Ar& save(Ar& ar, ConstCommonObject obj){
		AIO_PRE_CONDITION(obj.valid());
		for (auto& i : obj.members()){
			i.type().methods().serialize(ar, i.asCommonObject());
		}
		return ar;
	}

	extern version_type calculateObjVersion(ConstCommonObject obj);
}}

#endif //end AIO_XIRANG_ITYPE_BINDER_H
