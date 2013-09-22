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
#include <aio/common/interface.h>
namespace aio { namespace io{
	namespace private_{
		template<typename ...T> struct total_size_of;
		template<> struct total_size_of<> {
			static const size_t value = 0;
		};

		template<typename T, typename ... U>
			struct total_size_of<T, U...> {
				static_assert(std::is_scalar<T>::value, "don't known how to skip a non-pod type (due to padding issue).");
				static const size_t value = sizeof(T) + total_size_of<U...>::value;
			};
	}

	template<size_t N> struct skip_n { };
	template<typename ... T> struct skip_scalar : skip_n<private_::total_size_of<T...>::value> { };
	template<typename T> struct skip_t { 
		typedef T type;
	};

	//TODO: clean block & non-block behavior



	template<typename Interface, typename Ar> Ar& get_interface(Ar& ar){ return ar;}
	template<typename Interface, typename ...Io> Interface& get_interface(const iref<Io...>& ar)
	{ return ar.template get<Interface>();}
	template<typename Interface, typename ...Io> Interface& get_interface(const iauto<Io...>& ar)
	{ return ar.template get<Interface>();}

	/// describe how to open an archive
	enum open_flag
	{
		of_open,	///< just open an existing archive
		of_create,	///< just create a non-exsting archive
		of_create_or_open,	///< as name

        of_low_mask = 0xffff,

        /// depends on implementation capability. intends to be used to create a tempfile
        of_remove_on_close = 1 << 16    ///< remove archive on close.     
	};

	/// option id
    enum io_option
    {
        ao_default = 0,

        ao_user = 1 << 16
    };

	const long_size_t unknow_size = -1;

	//reader
	struct AIO_INTERFACE reader
	{
		typedef byte* iterator;

		/// \param buf should be memory continuous
		/// \return return rest part of given buf
		virtual range<iterator> read(const range<iterator>& buf) = 0;

		/// \return not at the end of archive
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

	//writer
	struct AIO_INTERFACE writer
	{
		typedef const byte* const_iterator;
		typedef const byte* iterator;
		/// \param buf should be memory continuous
		/// \return return rest part of given buffer
		virtual range<const_iterator> write(const range<const_iterator>& r) = 0;

		virtual bool writable() const = 0;

		/// sync buffer, implementationi defines.
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

	//ioctrl
	struct AIO_INTERFACE ioctrl{
		/// set archive length with given size
		/// \return the given size or unknow_size if failed 
		virtual long_size_t truncate(long_size_t size) = 0;
		virtual ~ioctrl(){}
	};
	template<typename CoClass> struct ioctrl_co : public ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			return get_cobj<CoClass>(this).truncate(size);
		}
	};
	template<typename CoClass>
	ioctrl_co<CoClass> get_interface_map(ioctrl*, CoClass*);

	//ioinfo
	struct AIO_INTERFACE ioinfo {
		/// \return archive length
		virtual long_size_t size() const = 0;
		virtual ~ioinfo(){}
	};
	template<typename CoClass> struct ioinfo_co : public ioinfo{
		virtual long_size_t size() const{
			return get_cobj<CoClass>(this).size();
		}
	};
	template<typename CoClass>
	ioinfo_co<CoClass> get_interface_map(ioinfo*, CoClass*);

	//sequence
	struct AIO_INTERFACE sequence
	{
		/// \return current seek position.
		virtual long_size_t offset() const = 0;
		/// \return archive length, may return unknow_size.
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

	//forward
	struct AIO_INTERFACE forward : sequence
	{
		/// \return current seek position.
		virtual long_size_t offset() const = 0;
		/// \return archive length, may return unknow_size.
		virtual long_size_t size() const = 0;
		/// seek to given position from beginning, and return the given offset or unknow_size if failed
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

	//random
	struct AIO_INTERFACE random : forward
	{
		/// \return current seek position.
		virtual long_size_t offset() const = 0;
		/// \return archive length, may return unknow_size.
		virtual long_size_t size() const = 0;
		/// seek to given position from beginning, and return the given offset
		/// the off may pass over the end.
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

	//options
	struct AIO_INTERFACE options{
		/// get an option with given id and option data
		virtual any getopt(int id, const any & optdata = any() ) const = 0;
		/// set an option with given id, option data and option data content
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

	//read_view
	struct AIO_INTERFACE read_view
	{
		/// \return memory continuous block, readonly
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

	struct empty_read_view : read_view
	{
		virtual range<const byte*> address() const {
			return range<const byte*>();
		}
	};
	//write_view
	struct AIO_INTERFACE write_view
	{
		/// \return memory continuous block, read write
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

	struct empty_write_view : write_view
	{
		virtual range<byte*> address() const {
			return range<byte*>();
		}
	};
	//read_map
	struct AIO_INTERFACE read_map
	{
		/// get a read_view with given handle, the result should large enough or empty.
		/// \note implementation should deal with alignment issues
		virtual iauto<read_view> view_rd(ext_heap::handle h) = 0;
		/// \return the whole size of the map
		virtual long_size_t size() const = 0;
		virtual ~read_map();
	};
	template<typename CoClass> struct read_map_co : public read_map
	{
		virtual iauto<read_view> view_rd(ext_heap::handle h){
			return get_cobj<CoClass>(this).view_rd(h);
		}
		virtual long_size_t size() const{
			return get_cobj<CoClass>(this).size();
		}
	};
	template<typename CoClass>
	read_map_co<CoClass> get_interface_map(read_map*, CoClass*);

	//write_map
	struct AIO_INTERFACE write_map
	{
		/// get a write_view with given handle, the result should large enough or empty.
		/// if h is out of range, implementation can decide increase automatically or not.
		/// \note implementation should deal with alignment issues
		virtual iauto<write_view> view_wr(ext_heap::handle h) = 0;

		/// \return the current lenght of write map
		virtual long_size_t size() const = 0;

		/// flush buffer, implementation defines
		virtual void sync() = 0;

		virtual ~write_map();
	};
	template<typename CoClass> struct write_map_co : public write_map
	{
		virtual iauto<write_view> view_wr(ext_heap::handle h){
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


	/// read full buf or reach the end of rd
	/// \return rest part of given buf
	extern range<reader::iterator> block_read(reader& rd, const range<reader::iterator>& buf);
	template<typename Ar>
	typename std::enable_if<!std::is_convertible<Ar&, reader&>::value, range<reader::iterator>>::type
	block_read(Ar& rd, const range<reader::iterator>& buf){
		iref<reader> iar(rd);
		return block_read(get_interface<reader>(iar), buf);
	}

	/// write all buf or until wr is not writable
	/// \return rest part of given buf
	extern range<writer::iterator> block_write(writer& wr, const range<writer::iterator>& buf);
	template<typename Ar>
	typename std::enable_if<!std::is_convertible<Ar&, writer&>::value, range<writer::iterator>>::type
	block_write(Ar& wr, const range<writer::iterator>& buf){
		iref<writer> iar(wr);
		return block_write(get_interface<writer>(iar), buf);
	}

	/// copy max_size bytes from rd to wr, or !rd.readable() || !wr.writable,
	/// \return the real copies bytes
	extern long_size_t copy_data(reader& rd, writer& wr, long_size_t max_size  = ~0 );

	extern long_size_t copy_data(reader& rd, write_map& wr, long_size_t max_size  = ~0 );
	extern long_size_t copy_data(read_map& rd, writer& wr, long_size_t max_size  = ~0 );
	extern long_size_t copy_data(read_map& rd, write_map& wr, long_size_t max_size  = ~0 );

	template<typename Input, typename Output, typename InAr, typename OutAr>
	long_size_t copy_data(InAr& rd, OutAr& wr, long_size_t max_size  = ~0 ){
		iref<Input> in(rd);
		iref<Output> out(rd);
		return copy_data(get_interface<Input>(in), get_interface<Output>(out), max_size);
	}

	AIO_EXCEPTION_TYPE(read_failed);
	AIO_EXCEPTION_TYPE(write_failed);
	AIO_EXCEPTION_TYPE(seek_failed);
    AIO_EXCEPTION_TYPE(create_failed);


}}
#endif //end AIO_COMMON_IARCHIVE_H_

