#ifndef AIO_COMMON_IO_S11N_BASE_H
#define AIO_COMMON_IO_S11N_BASE_H

#include <xirang/io.h>

namespace xirang{ namespace io{ namespace s11n{
	struct serializer_tag{};
	struct deserializer_tag{};
	struct context_serializer_tag : serializer_tag{};
	struct context_deserializer_tag : deserializer_tag{};

	template<typename Ar>
	struct is_serializer : public std::is_convertible<typename Ar::tag_type, serializer_tag>{ };
	template<typename Ar>
	struct is_deserializer : public std::is_convertible<typename Ar::tag_type, deserializer_tag>{ };

	template<typename Ar>
	struct is_context_serializer : public std::is_convertible<typename Ar::tag_type, context_serializer_tag>{ };
	template<typename Ar>
	struct is_context_deserializer : public std::is_convertible<typename Ar::tag_type, context_deserializer_tag>{ };

	template<typename Ar>
	struct serializer_base {
		typedef serializer_tag  tag_type;

		explicit serializer_base(Ar& ar) : m_archive(&ar){};
		operator Ar& () const {
			return get();
		}
		Ar& get() const{ return *m_archive;}
		private:
		Ar* m_archive;
	};

	template<typename Ar>
	struct deserializer_base {
		typedef deserializer_tag  tag_type;

		explicit deserializer_base(Ar& ar) : m_archive(&ar){};
		operator Ar& () const {
			return get();
		}
		Ar& get() const{ return *m_archive;}
		private:
		Ar* m_archive;
	};

	template<typename Ar, typename ContextType>
		struct context_serializer : public Ar{
			static_assert(is_serializer<Ar>::value, "context_serializer require a serializer");
			typedef context_serializer  tag_type;
			context_serializer(Ar& ar, ContextType& c) : context(&c){}
			ContextType* context;
		};
	template<typename Ar, typename ContextType>
		context_serializer<Ar, ContextType> make_context_serializer(Ar& ar, ContextType& c){
			return context_serializer<Ar, ContextType>(ar, c);
		}
	template<typename Ar, typename ContextType>
		struct context_deserializer : public Ar{
			static_assert(is_deserializer<Ar>::value, "context_deserializer require a deserializer");
			typedef context_deserializer  tag_type;
			context_deserializer(Ar& ar, ContextType& c) : context(&c){}
			ContextType* context;
		};
	template<typename Ar, typename ContextType>
		context_deserializer<Ar, ContextType> make_context_deserializer(Ar& ar, ContextType& c){
			return context_deserializer<Ar, ContextType>(ar, c);
		}

}
	template<typename Ar, typename T,
		typename = typename std::enable_if<s11n::is_deserializer<Ar>::value, Ar>::type>
		Ar& operator&(Ar& ar, T& t){	//load
			return load(ar, t);
		}

	template<typename Ar, size_t N,
		typename = typename std::enable_if<s11n::is_deserializer<Ar>::value, Ar>::type>
		Ar& operator&(Ar& ar, skip_n<N> t){       //load
			return load(ar, t);
		}


	template<typename Ar, typename T,
		typename = typename std::enable_if<s11n::is_serializer<Ar>::value, Ar>::type>
		Ar& operator&(Ar& ar, const T& t){	//save
			return save(ar, t);
		}

	template<typename Ar, size_t N,
		typename = typename std::enable_if<s11n::is_serializer<Ar>::value>::type>
		Ar& save(Ar& wt, skip_n<N> t)
		{
			auto& seeker = get_interface<io::sequence>(wt.get());
			seeker.seek(seeker.offset() + N);
			return wt;
		}

	template<typename Ar, size_t N,
		typename = typename std::enable_if<s11n::is_deserializer<Ar>::value>::type>
		Ar& load(Ar& rd, skip_n<N> t)
		{
			auto& seeker = get_interface<io::sequence>(rd.get());
			seeker.seek(seeker.offset() + N);
			return rd;
		}

	template<typename T, typename Ar,
		typename = typename std::enable_if<s11n::is_deserializer<Ar>::value>::type>
		T load(Ar& rd)
	{
		T t;
		load(rd, t);
		return std::move(t);
	}

}}
#endif //end AIO_COMMON_IO_S11N_BASE_H

