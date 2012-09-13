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
	ext_heap::handle::handle() : begin(-1), end(-1){}
	ext_heap::handle::handle(long_offset_t b, long_offset_t e) : begin(b), end(e){}
	long_size_t ext_heap::handle::size() const { return end - begin;}
	void ext_heap::handle::clear() { begin = -1; end = -1;}
	bool ext_heap::handle::valid() const { return 0 <= begin && begin < end; }
	ext_heap::handle::operator bool() const { return valid();}
}
