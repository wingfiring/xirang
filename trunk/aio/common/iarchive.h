//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_IARCHIVE_H_
#define AIO_COMMON_IARCHIVE_H_

#include <aio/common/config.h>
#include <aio/common/buffer.h>
#include <aio/common/context_except.h>

#include <aio/common/ideletor.h>
#include <aio/common/assert.h>
#include <aio/common/memory.h>
#include <aio/common/any.h>
#include <aio/common/backward/unique_ptr.h>
#include <aio/common/interface.h>
namespace aio { namespace archive{

	enum archive_mode 
	{
		mt_sequence = 0,
		mt_forward = 1,
		mt_random = 2,

		mt_read = 4,
		mt_write = 8,
		mt_user = 1<<24
	};

	enum open_flag
	{
		of_open,
		of_create,
		of_create_or_open,

        of_low_mask = 0xffff,

        /// depends on implementation capability. intends to be used to create a tempfile
        of_remove_on_close = 1 << 16        
	};

    enum archive_option
    {
        ao_default = 0,

        ao_user = 1 << 16
    };

	struct view_imp
	{
		virtual void* address() = 0;
		virtual ext_heap::handle handle() const = 0;
		virtual void destroy() = 0;
		protected:
		virtual ~view_imp();
	};

    struct iarchive;
    struct archive_deletor;

	typedef aio::unique_ptr<iarchive, archive_deletor> archive_ptr;

	class const_view
	{
		public:
			const_view(const_view && rhs);
			const_view(view_imp* imp = 0);
			~const_view();
			const_view& operator=(const_view&& rhs);

			EXPLICIT_OPERATOR  operator bool() const;
			bool valid() const;
			const void* address() const;
			ext_heap::handle handle() const;

			void swap(const_view& rhs);


		protected:
			const_view(const const_view& );/* = delete;*/
			const_view& operator=(const const_view&);/* = delete;*/
			view_imp* m_imp;
	};

	class view : public const_view
	{
		public:
			view(view_imp* imp = 0);
			view(view&& other);
			view& operator=(view&& other);
			void* address() const;
			void swap(view& rhs);
		private:
			view (const view&) /*= delete*/;
			view& operator=(const view&) /*= delete*/;
	};

	struct AIO_INTERFACE reader
	{
		typedef buffer<byte>::iterator iterator;
		virtual iterator read(const range<iterator>& buf) = 0;
		virtual bool readable() const = 0;
		virtual const_view view_rd(ext_heap::handle h) const = 0;
		virtual bool viewable() const = 0;

		protected:
		virtual ~reader() = 0;
	};


	struct AIO_INTERFACE writer
	{
		typedef buffer<byte>::const_iterator const_iterator;
		virtual const_iterator write(const range<const_iterator>& r) = 0;
		virtual long_size_t truncate(long_size_t size) = 0;
		virtual bool writable() const = 0;
		virtual void sync() = 0;
		virtual view view_wr(ext_heap::handle h) = 0;
		virtual bool viewable() const = 0;

		protected:
		virtual ~writer() = 0;
	};


	struct AIO_INTERFACE sequence
	{
		const static long_size_t unknow_size = -1;

		virtual long_size_t offset() const = 0;
		virtual long_size_t size() const = 0;
		protected:
		virtual ~sequence()= 0;
	};

	struct AIO_INTERFACE forward : sequence
	{
		virtual long_size_t offset() const = 0;
		virtual long_size_t size() const = 0;
		virtual long_size_t seek(long_size_t offset) = 0;
		protected:
		virtual ~forward()= 0;
	};

	struct AIO_INTERFACE random : forward
	{
		virtual long_size_t offset() const = 0;
		virtual long_size_t size() const = 0;
		virtual long_size_t seek(long_size_t offset) = 0;
		protected:
		virtual ~random()= 0;
	};

	struct AIO_INTERFACE iarchive
	{
		virtual reader* query_reader() = 0;
		virtual writer* query_writer() = 0;
		virtual sequence* query_sequence() = 0;
		virtual forward* query_forward() = 0;
		virtual random* query_random() = 0;
		virtual ideletor* query_deletor() = 0;

        virtual any getopt(int id, const any & optdata = any() ) const ;
        virtual any setopt(int id, const any & optdata,  const any & indata= any());

		protected:
		virtual ~iarchive()= 0;
	};

	template <typename ... Base>
		struct archiveT : iarchive, Base...
	{
		virtual reader* query_reader() { return cast_<reader>();}
		virtual writer* query_writer() { return cast_<writer>();}
		virtual sequence* query_sequence() { return cast_<sequence>();}
		virtual forward* query_forward() { return cast_<forward>();}
		virtual random* query_random() { return cast_<random>();}

		virtual ideletor* query_deletor() { return cast_<ideletor>(); }

    private:
		template<typename T> T* cast_()
		{
			return dynamic_cast<T*>(this);
		}
	};

	extern void default_archive_deletor(iarchive* p);
	struct archive_deletor
	{
		typedef void(*deletor_type)(iarchive*) ;

		archive_deletor() : m_deletor(default_archive_deletor){}
		explicit archive_deletor(deletor_type f) : m_deletor(f){}

		void operator()(iarchive* p) const
		{
			if (m_deletor) m_deletor(p);
		}
		deletor_type m_deletor;
	};

	AIO_EXCEPTION_TYPE(read_failed);
	AIO_EXCEPTION_TYPE(write_failed);
	AIO_EXCEPTION_TYPE(seek_failed);
    AIO_EXCEPTION_TYPE(create_failed);

	extern reader::iterator block_read(reader& rd, const range<reader::iterator>& buf);
	extern writer::const_iterator block_write(writer& wr, const range<writer::const_iterator>& buf);
	extern long_size_t copy_archive(iarchive& from, iarchive& to, long_size_t size = ~0);
	extern long_size_t copy_data(reader& rd, writer& wr, long_size_t max_size  = ~0 );

	template<typename T>
		typename std::enable_if<std::is_pod<T>::value, writer&>::type 
     operator & ( writer& wt, const T& t)
		{
			const byte* first = reinterpret_cast<const byte*>(&t);
			const byte* last = reinterpret_cast<const byte*>(&t + 1);

			block_write(wt, make_range(first, last));

			return wt;
		}

	template<typename T>
		typename std::enable_if<std::is_pod<T>::value, reader&>::type 
    operator & ( reader& rd, T& t)
		{
			byte* first = reinterpret_cast<byte*>(&t);
			byte* last = reinterpret_cast<byte*>(&t + 1);

			if (block_read(rd, make_range(first, last)) != last)
				AIO_THROW(read_failed);

			return rd;
		}

	inline reader& read(reader& rd) { return rd;}
	inline writer& write(writer& wr) { return wr;}

    template<typename T> reader& operator &(reader& ar, buffer<T>& buf)
    {
        uint32_t size;
        ar & size;
        buf.resize(size);
        if (size > 0)
        {   
            byte* first = reinterpret_cast<byte*>(buf.data());
            byte* last = first + sizeof(T) * size;
            ar.read(make_range(first, last));
        }
        return ar;
    }


    template<typename T> writer& operator &(writer& ar, const buffer<T>& buf)
    {
        uint32_t size = static_cast<uint32_t>(buf.size());
        ar & size;
        if (!buf.empty())
        {
            const byte* first = reinterpret_cast<const byte*>(buf.data());
            const byte* last = first + sizeof(T) * buf.size();
            ar.write(make_range(first, last));
        }
        return ar;
    }

}
namespace io{
	enum archive_mode 
	{
		mt_sequence = 0,
		mt_forward = 1,
		mt_random = 2,

		mt_read = 4,
		mt_write = 8,
		mt_user = 1<<24
	};

	enum open_flag
	{
		of_open,
		of_create,
		of_create_or_open,

        of_low_mask = 0xffff,

        /// depends on implementation capability. intends to be used to create a tempfile
        of_remove_on_close = 1 << 16        
	};

    enum archive_option
    {
        ao_default = 0,

        ao_user = 1 << 16
    };

	struct AIO_INTERFACE reader
	{
		typedef byte* iterator;
		virtual range<iterator> read(const range<iterator>& buf) = 0;
		virtual bool readable() const = 0;

		virtual ~reader();
	};
	template<typename Derive> struct reader_co : public reader
	{
		virtual range<iterator> read(const range<iterator>& buf){
			return static_cast<const Derive*>(this)->get_target()->read(buf);
		}
		virtual bool readable() const{
			return static_cast<const Derive*>(this)->get_target()->readable();
		}
	};
	template<typename Derive>
	reader_co<Derive> get_interface_map(Derive*, reader*);

	struct AIO_INTERFACE writer
	{
		typedef const byte* const_iterator;
		typedef const byte* iterator;
		virtual range<iterator> write(const range<const_iterator>& r) = 0;
		virtual long_size_t truncate(long_size_t size) = 0;
		virtual bool writable() const = 0;
		virtual void sync() = 0;

		virtual ~writer();
	};
	template<typename Derive> struct writer_co : public writer
	{
		virtual range<iterator> write(const range<const_iterator>& r){
			return static_cast<const Derive*>(this)->get_target()->write(r);
		}
		virtual long_size_t truncate(long_size_t size){
			return static_cast<const Derive*>(this)->get_target()->truncate(size);
		}
		virtual bool writable() const{
			return static_cast<const Derive*>(this)->get_target()->writable();
		}
		virtual void sync(){
			static_cast<const Derive*>(this)->get_target()->sync();
		}
	};
	template<typename Derive>
	writer_co<Derive> get_interface_map(Derive*, writer*);

	struct AIO_INTERFACE sequence
	{
		const static long_size_t unknow_size = -1;

		virtual long_size_t offset() const = 0;
		virtual long_size_t size() const = 0;
		virtual ~sequence();
	};
	template<typename Derive> struct sequence_co : public sequence
	{
		virtual long_size_t offset() const{
			return static_cast<const Derive*>(this)->get_target()->offset();
		}
		virtual long_size_t size() const{
			return static_cast<const Derive*>(this)->get_target()->size();
		}
	};
	template<typename Derive>
	sequence_co<Derive> get_interface_map(Derive*, sequence*);

	struct AIO_INTERFACE forward : sequence
	{
		virtual long_size_t offset() const = 0;
		virtual long_size_t size() const = 0;
		virtual long_size_t seek(long_size_t off) = 0;
		virtual ~forward();
	};
	template<typename Derive> struct forward_co : public forward
	{
		virtual long_size_t offset() const{
			return static_cast<const Derive*>(this)->get_target()->offset();
		}
		virtual long_size_t size() const{
			return static_cast<const Derive*>(this)->get_target()->size();
		}
		virtual long_size_t seek(long_size_t off){
			return static_cast<const Derive*>(this)->get_target()->seek(off);
		}
	};
	template<typename Derive>
	forward_co<Derive> get_interface_map(Derive*, forward*);

	struct AIO_INTERFACE random : forward
	{
		virtual long_size_t offset() const = 0;
		virtual long_size_t size() const = 0;
		virtual long_size_t seek(long_size_t offset) = 0;
		virtual ~random();
	};
	template<typename Derive> struct random_co : public random
	{
		virtual long_size_t offset() const{
			return static_cast<const Derive*>(this)->get_target()->offset();
		}
		virtual long_size_t size() const{
			return static_cast<const Derive*>(this)->get_target()->size();
		}
		virtual long_size_t seek(long_size_t off){
			return static_cast<const Derive*>(this)->get_target()->seek(off);
		}
	};
	template<typename Derive>
	random_co<Derive> get_interface_map(Derive*, random*);

	struct AIO_INTERFACE options{
		virtual any getopt(int id, const any & optdata = any() ) const = 0;
		virtual any setopt(int id, const any & optdata,  const any & indata= any()) = 0;
		virtual ~options();
	};
	template<typename Derive> struct options_co : public options
	{
		virtual any getopt(int id, const any & optdata){
			return static_cast<const Derive*>(this)->get_target()->getopt(id, optdata);
		}
		virtual any setopt(int id, const any & optdata,  const any & indata){
			return static_cast<const Derive*>(this)->get_target()->setopt(id, optdata, indata);
		}
	};
	template<typename Derive>
	options_co<Derive> get_interface_map(Derive*, options*);

	struct AIO_INTERFACE read_view
	{
		virtual range<const byte*> address() const =0;
		virtual ~read_view();
	};
	template<typename Derive> struct read_view_co : public read_view
	{
		virtual range<const byte*> address() const{
			return static_cast<const Derive*>(this)->get_target()->address();
		}
	};
	template<typename Derive>
	read_view_co<Derive> get_interface_map(Derive*, read_view*);

	struct AIO_INTERFACE write_view
	{
		virtual range<byte*> address() const = 0;
		virtual ~write_view();
	};
	template<typename Derive> struct write_view_co : public write_view
	{
		virtual range<byte*> address() const {
			return static_cast<const Derive*>(this)->get_target()->address();
		}
	};
	template<typename Derive>
	write_view_co<Derive> get_interface_map(Derive*, write_view*);

	struct AIO_INTERFACE read_map
	{
		virtual unique_ptr<read_view> view_rd(ext_heap::handle h) = 0;
		virtual ~read_map();
	};
	template<typename Derive> struct read_map_co : public read_map
	{
		virtual unique_ptr<read_view> view_rd(ext_heap::handle h){
			return static_cast<const Derive*>(this)->get_target()->view_rd(h);
		}
	};
	template<typename Derive>
	read_view_co<Derive> get_interface_map(Derive*, read_view*);

	struct AIO_INTERFACE write_map
	{
		virtual unique_ptr<write_view> view_wr(ext_heap::handle h) = 0;
		virtual ~write_map();
	};
	template<typename Derive> struct write_map_co : public write_map
	{
		virtual unique_ptr<write_view> view_wr(ext_heap::handle h){
			return static_cast<const Derive*>(this)->get_target()->write_rd(h);
		}
	};
	template<typename Derive>
	write_view_co<Derive> get_interface_map(Derive*, write_view*);


	extern range<reader::iterator> block_read(reader& rd, const range<reader::iterator>& buf);
	extern range<writer::iterator> block_write(writer& wr, const range<writer::iterator>& buf);
	extern long_size_t copy_data(reader& rd, writer& wr, long_size_t max_size  = ~0 );

	AIO_EXCEPTION_TYPE(read_failed);
	AIO_EXCEPTION_TYPE(write_failed);
	AIO_EXCEPTION_TYPE(seek_failed);
    AIO_EXCEPTION_TYPE(create_failed);

}

namespace lio{
	template<typename Ar, typename T>
		typename std::enable_if<std::is_pod<T>::value, Ar&>::type 
		save( Ar& wt, const T& t)
		{
			const byte* first = reinterpret_cast<const byte*>(&t);
			const byte* last = reinterpret_cast<const byte*>(&t + 1);

			io::block_write(wt, make_range(first, last));

			return wt;
		}

	template<typename Ar, typename T>
		typename std::enable_if<std::is_pod<T>::value, Ar&>::type 
		load(Ar& rd, T& t)
		{
			byte* first = reinterpret_cast<byte*>(&t);
			byte* last = reinterpret_cast<byte*>(&t + 1);

			if (!io::block_read(rd, make_range(first, last)).empty() )
				AIO_THROW(io::read_failed);

			return rd;
		}

	template<typename Ar, typename T> Ar& save(Ar& ar, const buffer<T>& buf)
	{
		uint32_t size = static_cast<uint32_t>(buf.size());
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
		uint32_t size;
		load(ar, size);
		buf.resize(size);
		if (size > 0)
		{   
			byte* first = reinterpret_cast<byte*>(buf.data());
			byte* last = first + sizeof(T) * size;
			ar.read(make_range(first, last));
		}
		return ar;
	}

	template<typename Ar, typename T>  
		typename std::enable_if<std::is_base_of<io::reader, Ar>::value, Ar&>::type 
		operator&(Ar& ar, T& t){	//load
			return load(ar, t);
		}
	template<typename Ar, typename T>  
		typename std::enable_if<std::is_base_of<io::writer, Ar>::value, Ar&>::type 
		operator&(Ar& ar, const T& t){	//load
			return save(ar, t);
		}
}

}
#endif //end AIO_COMMON_IARCHIVE_H_

