#ifndef AIO_COMMON_ARCHIVE_ADAPTOR_H
#define AIO_COMMON_ARCHIVE_ADAPTOR_H

#include <aio/common/iarchive.h>

namespace aio{ namespace io{

    template<typename Base >
		struct adaptor_deletor : Base
	{
		explicit adaptor_deletor(iarchive* par = 0) : Base(par){}
		virtual void destroy() {
			delete this;
		}
	};

	template<typename ... Base>
		struct proxy_base : archiveT<Base...>
	{
		explicit proxy_base(iarchive* par = 0) : m_ar(par){}

		protected:
		reader& reader_() const {return *m_ar->query_reader();}
		writer& writer_() const {return *m_ar->query_writer();}
		sequence& sequence_() const {return *m_ar->query_sequence();}
		forward& forward_() const {return *m_ar->query_forward();}
		random& random_() const {return *m_ar->query_random();}

		iarchive* m_ar;
	};

	template<typename Base = proxy_base<reader> >
		struct proxy_reader : Base
	{
		typedef reader::iterator iterator;
		explicit proxy_reader(iarchive* par = 0) : Base(par){}

		virtual iterator read(const aio::range<iterator>& buf)
		{
			return Base::reader_().read(buf);
		}

		virtual bool readable() const
		{
			return Base::reader_().readable();
		}

		virtual const_view view_rd(ext_heap::handle h) const {
			return Base::reader_().view_rd(h);
		}

		virtual bool viewable() const{
			return Base::reader_().viewable();
		}
	};

	template<typename Base = proxy_base<writer> >
		struct proxy_writer : Base
	{
		typedef writer::const_iterator const_iterator;
		explicit proxy_writer(iarchive* par = 0) : Base(par){}

		virtual const_iterator write(const aio::range<const_iterator>& r)
		{
			return Base::writer_().write(r);
		}
		virtual long_size_t truncate(long_size_t size)
		{
			return Base::writer_().truncate(size);
		}
		virtual bool writable() const
		{
			return Base::writer_().writable();
		}
		virtual void sync() 
		{
			return Base::writer_().sync();
		}

		virtual view view_wr(ext_heap::handle h) {
			return Base::writer_().view_wr(h);
		}

		virtual bool viewable() const{
			return Base::writer_().viewable();
		}
	};


	template<typename Base = proxy_base<sequence> >
		struct proxy_sequence : Base
	{
		explicit proxy_sequence(iarchive* par = 0) : Base(par){}

		virtual long_size_t offset() const
		{
			return Base::sequence_().offset();
		}

		virtual long_size_t size() const
		{
			return Base::sequence_().size();
		}
	};

	template<typename Base = proxy_base<forward> >
		struct proxy_forward : Base
	{
		explicit proxy_forward(iarchive* par = 0) : Base(par){}

		virtual long_size_t offset() const
		{
			return Base::forward_().offset();
		}

		virtual long_size_t size() const
		{
			return Base::forward_().size();
		}

		virtual long_size_t seek(long_size_t offset)
		{
			return Base::forward_().seek(offset);
		}
	};

	template<typename Base = proxy_base<random> >
		struct proxy_random : Base
	{
		explicit proxy_random(iarchive* par = 0) : Base(par){}

		virtual long_size_t offset() const
		{
			return Base::random_().offset();
		}

		virtual long_size_t size() const
		{
			return Base::random_().size();
		}

		virtual long_size_t seek(long_size_t offset)
		{
			return Base::random_().seek(offset);
		}
	};

    template<typename Base >
    struct multiplex_deletor : Base
    {
        explicit multiplex_deletor(iarchive* par) : Base(par){}
        virtual void destroy() {
            delete this;
        }
    };

    template<typename Base >
    struct multiplex_owner : Base
    {
        archive_ptr m_ar;
        explicit multiplex_owner(archive_ptr&& par) : Base(par.get())
        {
            m_ar = std::move(par);
        }
    };

	template<typename ... Base>
		struct multiplex_base : archiveT<Base...>
	{
		explicit multiplex_base(iarchive* par) : m_ar(par), m_random(0), m_offset(0)
        {
            m_random = m_ar ?
                m_ar->query_random() : 0;
            AIO_PRE_CONDITION(!m_ar || m_random);
        }

		protected:
		reader& reader_() const {return *m_ar->query_reader();}
		writer& writer_() const {return *m_ar->query_writer();}

		iarchive* m_ar;
		random* m_random;
        mutable long_size_t m_offset;
	};

    struct TieOffset
    {
        random& m_rnd;
        long_size_t& m_off;
        TieOffset(random& rnd, long_size_t& off)
            : m_rnd(rnd), m_off(off)
        {
            rnd.seek(off);
        }
        ~TieOffset()
        {
            m_off = m_rnd.offset();
        }
    };

	template<typename Base = multiplex_base<reader> >
		struct multiplex_reader : Base
	{
		typedef reader::iterator iterator;
		explicit multiplex_reader(iarchive* par) : Base(par){}

		virtual iterator read(const aio::range<iterator>& buf)
		{
            TieOffset holder(*Base::m_random, Base::m_offset);
			return Base::reader_().read(buf);
		}

		virtual bool readable() const
		{
            TieOffset holder(*Base::m_random, Base::m_offset);
			return Base::reader_().readable();
		}

		virtual const_view view_rd(ext_heap::handle h) const {
            TieOffset holder(*Base::m_random, Base::m_offset);
			return Base::reader_().view_rd(h);
		}

		virtual bool viewable() const{
			return Base::reader_().viewable();
		}
	};

	template<typename Base = multiplex_base<writer> >
		struct multiplex_writer : Base
	{
		typedef writer::const_iterator const_iterator;
		explicit multiplex_writer(iarchive* par) : Base(par){}

		virtual const_iterator write(const aio::range<const_iterator>& r)
		{
            TieOffset holder(*Base::m_random, Base::m_offset);
			return Base::writer_().write(r);
		}
		virtual long_size_t truncate(long_size_t size)
		{
            TieOffset holder(*Base::m_random, Base::m_offset);
			return Base::writer_().truncate(size);
		}
		virtual bool writable() const
		{
            TieOffset holder(*Base::m_random, Base::m_offset);
			return Base::writer_().writable();
		}
		virtual void sync() 
		{
			return Base::writer_().sync();
		}

		virtual view view_wr(ext_heap::handle h) {
            TieOffset holder(*Base::m_random, Base::m_offset);
			return Base::writer_().view_wr(h);
		}

		virtual bool viewable() const{
			return Base::writer_().viewable();
		}
	};

	template<typename Base = multiplex_base<random> >
		struct multiplex_random : Base
	{
		explicit multiplex_random(iarchive* par) : Base(par){}

		virtual long_size_t offset() const
		{
			return Base::m_offset;
		}

		virtual long_size_t size() const
		{
			return Base::m_random->size();
		}

		virtual long_size_t seek(long_size_t offset)
		{
			Base::m_offset = Base::m_random->seek(offset);
            return Base::m_offset;
		}
	};	
}}
#endif //end AIO_COMMON_ARCHIVE_ADAPTOR_H

