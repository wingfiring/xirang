#ifndef XIRANG_TYPE_BINDER_H
#define XIRANG_TYPE_BINDER_H

#include <aio/xirang/itypebinder.h>

#include <aio/xirang/binder.h>
#include <aio/xirang/serialize.h>

#include <typeinfo>

namespace xirang
{
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


	template<typename T> serialize::constructor<T> get_constructor(T*) { return serialize::constructor<T>();}
	template<typename T> serialize::destructor<T> get_destructor(T*) { return serialize::destructor<T>();}
	template<typename T> serialize::assigner<T> get_assigner(T*) { return serialize::assigner<T>();}
	template<typename T> serialize::deserializer<T> get_deserializer(T*) { return serialize::deserializer<T>();}
	template<typename T> serialize::serializer<T> get_serializer(T*) { return serialize::serializer<T>();}
	template<typename T> serialize::layout<T> get_layout(T*) { return serialize::layout<T>();}
	template<typename T> serialize::extendMethods<T> get_extendMethods(T*) { return serialize::extendMethods<T>();}

	template<typename T>
		struct PrimitiveMethods : public TypeMethods
	{
		virtual void construct(CommonObject obj, heap&  al , ext_heap& ext) const
		{
			get_constructor((T*)0).apply(obj, al, ext);
		}

		virtual void destruct(CommonObject obj) const
		{
			get_destructor((T*)0).apply(obj);
		}
		virtual void assign(ConstCommonObject src, CommonObject dest) const{
			get_assigner((T*)0).apply(src, dest);
		}

		virtual void deserialize(aio::io::reader& rd, CommonObject obj, heap& inner, ext_heap& ext) const
		{
			AIO_PRE_CONDITION(obj.valid() );
			get_deserializer((T*)0).apply(rd, obj, inner, ext);
		}

		virtual void serialize(aio::io::writer& wr, ConstCommonObject obj) const
		{
			AIO_PRE_CONDITION(obj.valid() );
			get_serializer((T*)0).apply(wr, obj);
		}

		virtual void beginLayout(std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
		{
			get_layout((T*)0).begin(payload, offset, align, pod);
		}

		virtual void nextLayout(TypeItem& item, std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
		{
			get_layout((T*)0).next(item, payload, offset, align, pod);
		}

		virtual const TypeInfoHandle& typeinfo() const
		{
			return typeinfo_;
		}

		virtual const MethodsExtension* extension() const
		{
			return get_extendMethods((T*)0).value();
		}

		private:

		static const TypeInfo<T> typeinfo_;
	};

	template<typename T>
		const TypeInfo<T>	PrimitiveMethods<T>::typeinfo_;

}
#endif //end XIRANG_TYPE_BINDER_H
