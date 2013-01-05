#include <aio/common/heap.h>
#include <aio/common/atomic.h>

#include <typeinfo>
#include <new>
namespace aio
{
	heap::~heap()
	{}

	ext_heap::~ext_heap()
	{ }

	plain_heap::plain_heap( memory::thread_policy )
	{}

	plain_heap::~plain_heap()
	{}

	/// call platform malloc. the parameter alignment and hint are ignored.
	void* plain_heap::malloc(std::size_t size, std::size_t /* alignment */, const void* /* hint */)
	{
		return ::operator new(size);
	}

	/// call platform free. the parameter alignment is ignored.
	void plain_heap::free(void* p, std::size_t /* size */, std::size_t /* alignment */ )
	{
		::operator delete(p);
	}

	/// return null for always.
	heap* plain_heap::underling()
	{
		return 0;
	}

	bool plain_heap::equal_to(const heap& rhs) const 
	{
		return typeid(*this) == typeid(rhs);
	}
	ext_heap::handle::handle() : m_begin(0), m_end(0){}
	ext_heap::handle::handle(long_offset_t b, long_offset_t e) : m_begin(b), m_end(e){
		AIO_PRE_CONDITION(b <= e);
	}
	long_size_t ext_heap::handle::size() const { return m_end - m_begin;}
	bool ext_heap::handle::empty() const { return m_begin == m_end; }
	long_offset_t ext_heap::handle::begin() const { return m_begin;}
	long_offset_t ext_heap::handle::end() const { return m_end;}

	bool ext_heap::handle::in(const ext_heap::handle& h) const { 
		return m_begin >= h.begin()
			&& m_end <= h.end();
	}
	bool ext_heap::handle::contains(const ext_heap::handle& h) const { 
		return h.in(*this);
	}
}
