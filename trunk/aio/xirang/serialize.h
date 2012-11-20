#ifndef AIO_XIRANG_SERIALIZE_H_
#define AIO_XIRANG_SERIALIZE_H_

#include <aio/common/iarchive.h>
#include <aio/common/archive/string.h>

namespace xirang{ namespace serialize{
	using aio::io::reader;
	using aio::io::writer;
	using namespace aio::lio;
	
	//overide buffer
	using aio::buffer;

	extern uint32_t numeric_uint32_cast(size_t n);

	template<typename Ar, typename T> Ar& save(Ar& ar, const buffer<T>& buf)
	{
		uint32_t size = numeric_uint32_cast(buf.size());
		save(ar, size);
		if (!buf.empty())
		{
			const byte* first = reinterpret_cast<const byte*>(buf.data());
			const byte* last = first + sizeof(T) * buf.size();
			ar.write(make_range(first, last));
		}
		return ar;
	}

	template<typename Ar, typename T> Ar& load(Ar& ar, buffer<T>& buf)
	{
		uint32_t size = load<uint32_t>(ar);
		buf.resize(size);
		if (size > 0)
		{   
			byte* first = reinterpret_cast<byte*>(buf.data());
			byte* last = first + sizeof(T) * size;
			ar.read(make_range(first, last));
		}
		return ar;
	}

	//overide string
	using aio::basic_string;
	using aio::basic_string_builder;

	template<typename Archive, typename T, typename = 
		typename std::enable_if< std::is_convertible<Archive&, reader&>::value , Archive&>::type>
	Archive& load(Archive& ar, basic_string<T>& buf)
	{
        buf.clear();

		uint32_t size = load<uint32_t>(ar);
		if (size > 0)
		{
			buffer<byte> rbuf;
			rbuf.resize(size * sizeof(T));
			block_read(ar, make_range(rbuf.begin(), rbuf.end()));
			
			T* first = (T*)rbuf.begin();
			T* last = first + size;
			buf = make_range(first, last);
		}

		return ar;
	}

	template<typename Archive, typename T, typename =
		typename std::enable_if< std::is_convertible<Archive&, writer&>::value , Archive&>::type>
	Archive& save(Archive& ar, const basic_string<T>& buf)
	{
		ar & buf.size();
		if (!buf.empty())
		{
			const byte* first = reinterpret_cast<const byte*>(buf.data());
			const byte* last = first + sizeof(T) * buf.size();
			block_write(ar, make_range(first, last));
		}
		return ar;
	}


	// default imp of type method
	template<typename T> struct serializer{
		static aio::io::writer& apply(aio::io::writer& wt, ConstCommonObject obj){
			return wt & uncheckBind<T>(obj);
		}
	};

	template<typename T> struct deserializer{
		static aio::io::reader& apply(aio::io::reader& rd, CommonObject obj, heap& inner, ext_heap& ext){
			return rd & uncheckBind<T>(obj);
		}
	};

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
	template<typename T> struct hasher<buffer<T>> {
		static size_t apply(ConstCommonObject obj){
			auto& data = uncheckBind<buffer<T>>(obj);
			size_t hash = 2166136261U;
			size_t stride = 1 + size / 10;

			for(size_t first = 0; first < size; first += stride)
				hash = 16777619U * hash ^ (size_t)data[first];

		}
	};
	// specialize for basic_string
	template<typename CharT> struct hasher<basic_string<CharT>> {
		static size_t apply(ConstCommonObject obj){
			auto& data = uncheckBind<basic_string<CharT>>(obj);
			return data.hash();
		}
	};
}

}}
#endif //end AIO_XIRANG_SERIALIZE_H_

