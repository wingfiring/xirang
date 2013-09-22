#ifndef AIO_COMMON_IO_S11N_BASE_H
#define AIO_COMMON_IO_S11N_BASE_H

#include <xirang/io.h>

namespace aio{ namespace io{ namespace s11n{
	struct serializer_tag{};
	struct deserializer_tag{};

	template<typename Ar>
	struct is_serializer : public std::is_same<typename Ar::tag_type, serializer_tag>{
	};
	template<typename Ar>
	struct is_deserializer : public std::is_same<typename Ar::tag_type, deserializer_tag>{
	};

	template<typename Ar, typename Tag>
	struct de_serializer_base {
		typedef Tag  tag_type;

		explicit de_serializer_base(Ar& ar) : m_archive(&ar){};
		operator Ar& () const {
			return get();
		}
		Ar& get() const{ return *m_archive;}
		private:
		Ar* m_archive;
	};

	template<typename Ar>
	using deserializer_base =  de_serializer_base<Ar, deserializer_tag> ;
	template<typename Ar>
	using serializer_base =  de_serializer_base<Ar, serializer_tag> ;

}
	template<typename Ar, typename T,  
		typename = typename std::enable_if<s11n::is_deserializer<Ar>::value, Ar>::type>
		Ar operator&(Ar ar, T& t){	//load
			return load(ar, t);
		}

	template<typename Ar, typename T,
		typename = typename std::enable_if<s11n::is_serializer<Ar>::value, Ar>::type>
		Ar operator&(Ar ar, const T& t){	//save
			return save(ar, t);
		}

	template<typename Ar, size_t N,
		typename = typename std::enable_if<s11n::is_serializer<Ar>::value>::type>
		Ar save(Ar wt, skip_n<N> t)
		{
			auto& seeker = get_interface<io::sequence>(wt.get());
			seeker.seek(seeker.offset() + N);
			return wt;
		}

	template<typename Ar, size_t N,
		typename = typename std::enable_if<s11n::is_deserializer<Ar>::value>::type>
		Ar load(Ar rd, skip_n<N> t)
		{
			auto& seeker = get_interface<io::sequence>(rd.get());
			seeker.seek(seeker.offset() + N);
			return rd;
		}

	template<typename T, typename Ar, 
		typename = typename std::enable_if<s11n::is_deserializer<Ar>::value>::type>
		T load(Ar rd)
	{
		T t;
		load(rd, t);
		return std::move(t);
	}

}}
#endif //end AIO_COMMON_IO_S11N_BASE_H

