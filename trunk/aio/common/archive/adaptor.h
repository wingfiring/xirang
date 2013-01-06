#ifndef AIO_COMMON_ARCHIVE_ADAPTOR_H
#define AIO_COMMON_ARCHIVE_ADAPTOR_H

#include <aio/common/iarchive.h>

namespace aio{ namespace io{

	/// \param ArchiveData	archive type, intends to be iref type
	/// \param ArchiveData	archive type, intends to be iref type
	template<typename ArchiveData, template<typename> class ... PartialInterfaces>
		struct combinator : public ArchiveData, 
		public PartialInterfaces<combinator<ArchiveData, PartialInterfaces...> >...
	{
		typedef typename ArchiveData::archive_type archive_type;
		template<typename ... Args>
		combinator(Args... args) : ArchiveData(std::forward<Args>(args)...){}
		combinator(){}
	};

	template<typename ArchiveData, template<typename>class... PartialInterfaces, typename T>
		static combinator<ArchiveData, PartialInterfaces...> combine(T& ar){
			return combinator<ArchiveData, PartialInterfaces...>(ar);
		}

	template<typename ArchiveType> struct proxy_archive
	{
		typedef ArchiveType	archive_type;

		proxy_archive(): m_underlying(){}
		proxy_archive(archive_type ar): m_underlying(ar){}
		
		const archive_type& underlying() const{ 
			AIO_PRE_CONDITION(valid());
			return m_underlying;
		}
		bool valid() const{
			return static_cast<bool>(m_underlying);
		}

		archive_type m_underlying;
	};

	template<typename Derive> struct proxy_reader_p : reader
	{
		typedef typename reader::iterator iterator;

		virtual range<iterator> read(const range<iterator>& buf) {
			return underlying_<reader>().read(buf);
		}

		virtual bool readable() const {
			return underlying_<reader>().readable();
		}

		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_writer_p : writer
	{
		typedef typename writer::iterator iterator;
		typedef typename writer::const_iterator const_iterator;

		virtual range<const_iterator> write(const range<const_iterator>& r){
			return underlying_<writer>().write(r);
		}
		virtual bool writable() const{
			return underlying_<writer>().writable();
		}
		virtual void sync(){
			underlying_<writer>().sync();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_ioctrl_p : ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			return underlying_<ioctrl>().truncate(size);
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_ioinfo_p : ioinfo 
	{
		virtual long_size_t size() const{
			return underlying_<ioinfo>().size();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_sequence_p : sequence 
	{
		virtual long_size_t offset() const{
			return underlying_<sequence>().offset();
		}
		virtual long_size_t size() const{
			return underlying_<sequence>().size();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_forward_p : forward 
	{
		virtual long_size_t offset() const{
			return underlying_<forward>().offset();
		}
		virtual long_size_t size() const{
			return underlying_<forward>().size();
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<forward>().seek(off);
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_random_p : random
	{
		virtual long_size_t offset() const{
			return underlying_<random>().offset();
		}
		virtual long_size_t size() const{
			return underlying_<random>().size();
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<random>().seek(off);
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_options_p : options
	{
		virtual any getopt(int id, const any & optdata = any() ) const {
			return underlying_<options>().getopt(id, optdata);
		}
		virtual any setopt(int id, const any & optdata,  const any & indata= any()){
			return underlying_<options>().setopt(id, optdata, indata);
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_read_view_p : read_view
	{
		virtual range<const byte*> address() const{
			return underlying_<read_view>().address();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_write_view_p : write_view
	{
		virtual range<byte*> address() const{
			return underlying_<write_view>().address();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_read_map_p : read_map
	{
		virtual iauto<read_view> view_rd(ext_heap::handle h){
			return underlying_<read_map>().view_rd(h);
		}
		virtual long_size_t size() const{
			return underlying_<read_map>().size();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct proxy_write_map_p : write_map
	{
		virtual iauto<write_view> view_wr(ext_heap::handle h) {
			return underlying_<write_map>().view_wr(h);
		}
		virtual long_size_t size() const{
			return underlying_<write_map>().size();
		}
		virtual void sync() {
			return underlying_<write_map>().sync();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};
	
	template<typename ArchiveType>
	struct multiplex_archive : public proxy_archive<ArchiveType>
	{
		typedef proxy_archive<ArchiveType> base;
		multiplex_archive(): current(0){}
		multiplex_archive(ArchiveType& ar, long_size_t off)
			: base(ar), current(off)
		{}
		
		long_size_t current;
	};


	template<typename Archive>
    struct multiplex_offset_tracker
    {
        Archive& m_ar;

        multiplex_offset_tracker(Archive& ar) : m_ar(ar) {
            m_ar.underlying().seek(m_ar.current);
        }
        ~multiplex_offset_tracker()
        {
            m_ar.current = m_ar.underlying().offset();
        }
    };

	template<typename Derive> struct multiplex_reader_p : reader
	{
		typedef reader::iterator iterator;

		virtual range<iterator> read(const range<iterator>& buf) {
            multiplex_offset_tracker<Derive> tracker(derive_());
			return underlying_<reader>().read(buf);
		}

		virtual bool readable() const {
			underlying_<random>().seek(derive_().current);
			return underlying_<reader>().readable();
		}

		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct multiplex_writer_p : writer
	{
		typedef typename writer::iterator iterator;
		typedef typename writer::const_iterator const_iterator;

		virtual range<const_iterator> write(const range<const_iterator>& r){
            multiplex_offset_tracker<Derive> tracker(derive_());
			return underlying_<writer>().write(r);
		}
		virtual bool writable() const{
			underlying_<random>().seek(derive_().current);
			return underlying_<writer>().writable();
		}
		virtual void sync(){
			underlying_<writer>().sync();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct multiplex_random_p : random
	{
		virtual long_size_t offset() const {
			return derive_().current;
		}
		virtual long_size_t size() const {
			return underlying_<random>().size();
		}
		virtual long_size_t seek(long_size_t current) {
			return (derive_().current = underlying_<random>().seek(current));
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		Derive& derive_() { return static_cast<Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct multiplex_ioctrl_p : ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			auto new_size = underlying_<ioctrl>().truncate(size);
			derive_().current = std::min(
					derive_().current,
					new_size);
			return new_size;
		}
		private:
		Derive& derive_() { return static_cast<Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> using multiplex_options_p = proxy_options_p<Derive>;
	template<typename Derive> using multiplex_ioinfo_p = proxy_ioinfo_p<Derive>;
	template<typename Derive> using multiplex_read_map_p = proxy_read_map_p<Derive>;
	template<typename Derive> using multiplex_write_map_p = proxy_write_map_p<Derive>;

	// TODO: remove multiplex dependency
	template<typename ArchiveType> struct sub_archive: public proxy_archive<ArchiveType>
	{
		typedef proxy_archive<ArchiveType> base;
		sub_archive(): first(0), last(0){}
		sub_archive(ArchiveType& ar, long_size_t first_, long_size_t last_)
			: base(ar)
			  , first(first_), last(last_)
		{
			AIO_PRE_CONDITION(first <= last);
			AIO_PRE_CONDITION(this.underlying().template get<sequence>().offset() == first);
		}

		long_size_t first;
		long_size_t last;
	};

	template<typename Derive> struct sub_reader_p : reader
	{
		typedef typename reader::iterator iterator;

		virtual range<iterator> read(const range<iterator>& buf) {
			auto current = underlying_<sequence>().offset();
			if (current >= derive_().first && current < derive_().last){
				auto min_size = std::min(buf.size(), derive_().last - current);
				range<iterator> sub_buf(buf.begin(), buf.begin() + min_size);
				return underlying_<reader>().read(sub_buf);
			}
			return buf;
		}

		virtual bool readable() const {
			auto current = underlying_<sequence>().offset();
			return underlying_<reader>().readable()
					&& current >= derive_().first
					&& current < derive_().last;
		}

		private:
		Derive& derive_() { return static_cast<Derive&>(*this);}
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct sub_writer_p : writer
	{
		typedef typename writer::iterator iterator;
		typedef typename writer::const_iterator const_iterator;

		virtual range<const_iterator> write(const range<const_iterator>& buf){
			auto current = underlying_<sequence>().offset();
			if (current >= derive_().first && current < derive_().last){
				auto min_size = std::min(buf.size(), derive_().last - current);
				range<iterator> sub_buf(buf.begin(), buf.begin() + min_size);
				return underlying_<writer>().write(sub_buf);
			}
			return buf;
		}
		virtual bool writable() const{
			auto current = underlying_<sequence>().offset();
			return underlying_<writer>().writable()
					&& current >= derive_().first
					&& current < derive_().last;
		}
		virtual void sync(){
			underlying_<writer>().sync();
		}
		private:
		Derive& derive_() { return static_cast<Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct sub_ioctrl_p : ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			return unknow_size;
		}
	};

	template<typename Derive> struct sub_ioinfo_p : ioinfo 
	{
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
	};

	template<typename Derive> struct sub_sequence_p : sequence 
	{
		typedef typename Derive::archive_type archive_type;

		virtual long_size_t offset() const{
			return underlying_<sequence>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};
	
	template<typename Derive> struct sub_forward_p : forward 
	{
		typedef typename Derive::archive_type archive_type;

		virtual long_size_t offset() const{
			return underlying_<forward>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<forward>().seek(derive_().first + off);
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct sub_random_p : random 
	{
		virtual long_size_t offset() const{
			return underlying_<random>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<random>().seek(derive_().first + off);
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct sub_read_map_p : read_map
	{
		virtual iauto<read_view> view_rd(ext_heap::handle h){
			h = ext_heap::handle(h.begin() + derive_().first, h.end() + derive_().first);

			if (h.begin() >= derive_().last)
				h = ext_heap::handle();
			else if(h.end() >= derive_().last)
				h = ext_heap::handle(h.begin(), derive_().last);

			return underlying_<read_map>().view_rd(h);
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};
	
	template<typename Derive> struct sub_write_map_p : write_map
	{
		virtual iauto<write_view> view_rd(ext_heap::handle h){
			h = ext_heap::handle(h.begin() + derive_().first, h.end() + derive_().first);

			if (h.begin() >= derive_().last)
				h = ext_heap::handle();
			else if(h.end() >= derive_().last)
				h = ext_heap::handle(h.begin(), derive_().last);

			return underlying_<write_map>().view_wr(h);
		}
		virtual long_size_t size() const{
			return derive_().last - derive_().first;
		}
		virtual void sync() {
			return underlying_<write_map>().sync();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename ArchiveType>
	struct tail_archive : public proxy_archive<ArchiveType>
	{
		typedef proxy_archive<ArchiveType> base;
		tail_archive(): first(0){}
		tail_archive(ArchiveType& ar, long_size_t first_)
			: base(ar) , first(first_)
		{
			AIO_PRE_CONDITION(this.underlying().template get<sequence>().offset() == first);
		}

		long_size_t first;
	};
	template<typename Derive>
		using tail_reader_p = proxy_reader_p<Derive>;
	template<typename Derive>
		using tail_writer_p = proxy_writer_p<Derive>;

	template<typename Derive> struct tail_ioctrl_p : ioctrl
	{
		virtual long_size_t truncate(long_size_t size){
			return underlying_<ioctrl>().truncate(size + derive_().first);
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct tail_ioinfo_p : ioinfo 
	{
		virtual long_size_t size() const{
			auto real_size = underlying_<ioinfo>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct tail_sequence_p : sequence 
	{
		virtual long_size_t offset() const{
			return underlying_<sequence>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<sequence>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};
	
	template<typename Derive> struct tail_forward_p : forward 
	{
		virtual long_size_t offset() const{
			return underlying_<forward>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<forward>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<forward>().seek(off + derive_().first);
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct tail_random_p : random 
	{
		virtual long_size_t offset() const{
			return underlying_<random>().offset() - derive_().first;	//it's ok if current < first, rewind
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<random>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		virtual long_size_t seek(long_size_t off){
			return underlying_<random>().seek(off + derive_().first);
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};

	template<typename Derive> struct tail_read_map_p : read_map
	{
		virtual iauto<read_view> view_rd(ext_heap::handle h){
			h = ext_heap::handle(h.begin() + derive_().first, h.end() + derive_().first);
			return underlying_<read_map>().view_rd(h);
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<read_map>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};
	
	template<typename Derive> struct tail_write_map_p : write_map
	{
		virtual iauto<write_view> view_rd(ext_heap::handle h){
			h = ext_heap::handle(h.begin() + derive_().first, h.end() + derive_().first);

			if (h.begin() >= derive_().last)
				h = ext_heap::handle();
			else if(h.end() >= derive_().last)
				h = ext_heap::handle(h.begin(), derive_().last);

			return underlying_<write_map>().view_wr(h);
		}
		virtual long_size_t size() const{
			auto real_size = underlying_<write_map>().size();
			return real_size < derive_().first
				? 0
				: real_size - derive_().first;
		}
		virtual void sync() {
			return underlying_<write_map>().sync();
		}
		private:
		const Derive& derive_() const{ return static_cast<const Derive&>(*this);}
		template<typename Interface> Interface& underlying_() const{ 
			return derive_().underlying().template get<Interface>(); 
		}
	};
}}
#endif //end AIO_COMMON_ARCHIVE_ADAPTOR_H

