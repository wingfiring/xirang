#include <xirang/heap.h>
#include <xirang/backward/atomic.h>

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
}
