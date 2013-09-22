#ifndef XIRANG_TYPE_BINDER_H
#define XIRANG_TYPE_BINDER_H

#include <aio/xirang/itypebinder.h>
#include <aio/xirang/binder.h>

namespace xirang
{
	template<typename T> struct assigner{
		static void apply(ConstCommonObject from, CommonObject to){
			uncheckBind<T>(to) = uncheckBind<T>(from);
		}
	};

	template<typename T> struct constructor{
		static void apply(CommonObject obj, heap& /*al */, ext_heap& /*ext*/) {
			new (& uncheckBind<T>(obj)) T();
		}
	};

	template<typename T> struct destructor{
		static void apply(CommonObject obj) {
			(& uncheckBind<T>(obj))->~T();
		}
	};

	template<typename T> struct comparison{
		static int apply(ConstCommonObject lhs,ConstCommonObject rhs) {
			const T& l = *reinterpret_cast<const T*>(lhs.data());
			const T& r = *reinterpret_cast<const T*>(rhs.data());
			return l < r 
				? -1 : r < l ? 1 : 0;
		}
	};

	template<typename T> struct hasher{
		static size_t apply(ConstCommonObject obj) {
			return (size_t)uncheckBind<T>(obj);
		}
	};

	template<> struct hasher<string>{
		static size_t apply(ConstCommonObject obj);
	};

	template<typename T> struct layout{
		static const std::size_t alignment_ = std::alignment_of<T>::value;
		static const bool is_pod_ = std::is_pod<T>::value;
		static const std::size_t size_ = sizeof(T);

		static void begin(std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod)
		{
			payload = size_;
			offset = 0;
			align = alignment_;
			pod = is_pod_;
		}

		static void next(TypeItem& item, std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod)
		{
			AIO_PRE_CONDITION(false && "should no composed item");
		}
	};

	template<typename T> struct extendMethods
	{
		static MethodsExtension* value()
		{
			static MethodsExtension methodsExt = 
			{
				&comparison<T>::apply,
				&hasher<T>::apply
			};
			return &methodsExt;
		}
	};

	// specialize for buffer 
	template<typename T> struct hasher<aio::buffer<T>> {
		static size_t apply(ConstCommonObject obj){
			auto& data = uncheckBind<aio::buffer<T>>(obj);
			auto size = data.size();
			size_t hash = 2166136261U;
			size_t stride = 1 + size / 10;

			for(size_t first = 0; first < size; first += stride)
				hash = 16777619U * hash ^ (size_t)data[first];

			return hash;
		}
	};
	// specialize for basic_string
	template<typename CharT> struct hasher<aio::basic_string<CharT>> {
		static size_t apply(ConstCommonObject obj){
			auto& data = uncheckBind<aio::basic_string<CharT>>(obj);
			return data.hash();
		}
	};

	template<typename T> constructor<T> get_constructor(T*) { return constructor<T>();}
	template<typename T> destructor<T> get_destructor(T*) { return destructor<T>();}
	template<typename T> assigner<T> get_assigner(T*) { return assigner<T>();}
	template<typename T> layout<T> get_layout(T*) { return layout<T>();}
	template<typename T> extendMethods<T> get_extendMethods(T*) { return extendMethods<T>();}

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

	template<typename T> const TypeInfo<T>	PrimitiveMethods<T>::typeinfo_;
}
#endif //end XIRANG_TYPE_BINDER_H
