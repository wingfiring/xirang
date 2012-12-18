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
namespace aio { namespace io{

	enum open_flag
	{
		of_open,
		of_create,
		of_create_or_open,

        of_low_mask = 0xffff,

        /// depends on implementation capability. intends to be used to create a tempfile
        of_remove_on_close = 1 << 16        
	};

    enum io_option
    {
        ao_default = 0,

        ao_user = 1 << 16
    };

	const long_size_t unknow_size = -1;

	struct AIO_INTERFACE reader
	{
		typedef byte* iterator;
		virtual range<iterator> read(const range<iterator>& buf) = 0;
		virtual bool readable() const = 0;

		virtual ~reader();
	};
	template<typename CoClass> struct reader_co : public reader
	{
		virtual range<iterator> read(const range<iterator>& buf){
			return get_cobj<CoClass>(this).read(buf);
		}
		virtual bool readable() const{
			return get_cobj<CoClass>(this).readable();
		}
	};
	template<typename CoClass>
	reader_co<CoClass> get_interface_map(reader*, CoClass*);

	struct AIO_INTERFACE writer
	{
		typedef const byte* const_iterator;
		typedef const byte* iterator;
		virtual range<iterator> write(const range<const_iterator>& r) = 0;
		virtual bool writable() const = 0;
		virtual void sync() = 0;

		virtual ~writer();
	};
	template<typename CoClass> struct writer_co : public writer
	{
		virtual range<iterator> write(const range<const_iterator>& r){
			return get_cobj<CoClass>(this).write(r);
		}
		virtual bool writable() const{
			return get_cobj<CoClass>(this).writable();
		}
		virtual void sync(){
			get_cobj<CoClass>(this).sync();
		}
	};
	template<typename CoClass>
	writer_co<CoClass> get_interface_map(writer*, CoClass*);

	struct AIO_INTERFACE ioctrl{
		virtual long_size_t truncate(long_size_t size) = 0;
		virtual long_size_t size() const = 0;
		virtual ~ioctrl(){}
	};
	template<typename CoClass> struct ioctrl_co : public ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			return get_cobj<CoClass>(this).truncate(size);
		}
		virtual long_size_t size() const{
			return get_cobj<CoClass>(this).size();
		}
	};
	template<typename CoClass>
	ioctrl_co<CoClass> get_interface_map(ioctrl*, CoClass*);


	struct AIO_INTERFACE sequence
	{

		virtual long_size_t offset() const = 0;
		virtual long_size_t size() const = 0;
		virtual ~sequence();
	};
	template<typename CoClass> struct sequence_co : public sequence
	{
		virtual long_size_t offset() const{
			return get_cobj<CoClass>(this).offset();
		}
		virtual long_size_t size() const{
			return get_cobj<CoClass>(this).size();
		}
	};
	template<typename CoClass>
	sequence_co<CoClass> get_interface_map(sequence*, CoClass*);

	struct AIO_INTERFACE forward : sequence
	{
		virtual long_size_t offset() const = 0;
		virtual long_size_t size() const = 0;
		virtual long_size_t seek(long_size_t off) = 0;
		virtual ~forward();
	};
	template<typename CoClass> struct forward_co : public forward
	{
		virtual long_size_t offset() const{
			return get_cobj<CoClass>(this).offset();
		}
		virtual long_size_t size() const{
			return get_cobj<CoClass>(this).size();
		}
		virtual long_size_t seek(long_size_t off){
			return get_cobj<CoClass>(this).seek(off);
		}
	};
	template<typename CoClass>
	forward_co<CoClass> get_interface_map(forward*, CoClass*);

	struct AIO_INTERFACE random : forward
	{
		virtual long_size_t offset() const = 0;
		virtual long_size_t size() const = 0;
		virtual long_size_t seek(long_size_t offset) = 0;
		virtual ~random();
	};
	template<typename CoClass> struct random_co : public random
	{
		virtual long_size_t offset() const{
			return get_cobj<CoClass>(this).offset();
		}
		virtual long_size_t size() const{
			return get_cobj<CoClass>(this).size();
		}
		virtual long_size_t seek(long_size_t off){
			return get_cobj<CoClass>(this).seek(off);
		}
	};
	template<typename CoClass>
	random_co<CoClass> get_interface_map(random*, CoClass*);

	struct AIO_INTERFACE options{
		virtual any getopt(int id, const any & optdata = any() ) const = 0;
		virtual any setopt(int id, const any & optdata,  const any & indata= any()) = 0;
		virtual ~options();
	};
	template<typename CoClass> struct options_co : public options
	{
		virtual any getopt(int id, const any & optdata){
			return get_cobj<CoClass>(this).getopt(id, optdata);
		}
		virtual any setopt(int id, const any & optdata,  const any & indata){
			return get_cobj<CoClass>(this).setopt(id, optdata, indata);
		}
	};
	template<typename CoClass>
	options_co<CoClass> get_interface_map(options*, CoClass*);

	struct AIO_INTERFACE read_view
	{
		virtual range<const byte*> address() const =0;
		virtual ~read_view();
	};
	template<typename CoClass> struct read_view_co : public read_view
	{
		virtual range<const byte*> address() const{
			return get_cobj<CoClass>(this).address();
		}
	};
	template<typename CoClass>
	read_view_co<CoClass> get_interface_map(read_view*, CoClass*);

	struct AIO_INTERFACE write_view
	{
		virtual range<byte*> address() const = 0;
		virtual ~write_view();
	};
	template<typename CoClass> struct write_view_co : public write_view
	{
		virtual range<byte*> address() const {
			return get_cobj<CoClass>(this).address();
		}
	};
	template<typename CoClass>
	write_view_co<CoClass> get_interface_map(write_view*, CoClass*);

	struct AIO_INTERFACE read_map
	{
		virtual unique_ptr<read_view> view_rd(ext_heap::handle h) = 0;
		virtual long_size_t size() const = 0;
		virtual ~read_map();
	};
	template<typename CoClass> struct read_map_co : public read_map
	{
		virtual unique_ptr<read_view> view_rd(ext_heap::handle h){
			return get_cobj<CoClass>(this).view_rd(h);
		}
		virtual long_size_t size() const{
			return get_cobj<CoClass>(this).size();
		}
	};
	template<typename CoClass>
	read_map_co<CoClass> get_interface_map(read_map*, CoClass*);

	struct AIO_INTERFACE write_map
	{
		virtual unique_ptr<write_view> view_wr(ext_heap::handle h) = 0;
		virtual long_size_t size() const = 0;
		virtual void sync() = 0;
		virtual ~write_map();
	};
	template<typename CoClass> struct write_map_co : public write_map
	{
		virtual unique_ptr<write_view> view_wr(ext_heap::handle h){
			return get_cobj<CoClass>(this).view_wr(h);
		}
		virtual long_size_t size() const{
			return get_cobj<CoClass>(this).size();
		}
		virtual void sync(){
			get_cobj<CoClass>(this).sync();
		}
	};
	template<typename CoClass>
	write_map_co<CoClass> get_interface_map(write_map*, CoClass*);


	extern range<reader::iterator> block_read(reader& rd, const range<reader::iterator>& buf);
	extern range<writer::iterator> block_write(writer& wr, const range<writer::iterator>& buf);
	extern long_size_t copy_data(reader& rd, writer& wr, long_size_t max_size  = ~0 );

	AIO_EXCEPTION_TYPE(read_failed);
	AIO_EXCEPTION_TYPE(write_failed);
	AIO_EXCEPTION_TYPE(seek_failed);
    AIO_EXCEPTION_TYPE(create_failed);

}

namespace sio{
	template<typename Ar, typename T>
		typename std::enable_if<std::is_pod<T>::value, Ar&>::type 
		save( Ar& wt, const T& t)
		{
			const byte* first = reinterpret_cast<const byte*>(&t);
			const byte* last = reinterpret_cast<const byte*>(&t + 1);

			io::block_write(wt, make_range(first, last));

			return wt;
		}

	template<typename T, typename Ar>
		typename std::enable_if<std::is_pod<T>::value, Ar&>::type 
		load(Ar& rd, T& t)
		{
			byte* first = reinterpret_cast<byte*>(&t);
			byte* last = reinterpret_cast<byte*>(&t + 1);

			if (!io::block_read(rd, make_range(first, last)).empty() )
				AIO_THROW(io::read_failed);

			return rd;
		}

	template<typename T, typename Ar>
		typename std::enable_if<std::is_base_of<io::reader, Ar>::value, T>::type 
		load(Ar& rd)
		{
			T t;
			load(rd, t);
			return std::move(t);
		}

	template<typename Ar, typename T> Ar& save(Ar& ar, const buffer<T>& buf)
	{
		save(ar, buf.size());
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
		size_t size = load<size_t>(ar);
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

