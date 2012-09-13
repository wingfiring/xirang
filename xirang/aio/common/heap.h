#ifndef AIO_COMMON_HEAP_H
#define AIO_COMMON_HEAP_H

#include <aio/common/memory.h>
#include <aio/common/string.h>

namespace aio
{
	/// plain heap. it forward the malloc and free to platform call directly.
	struct AIO_COMM_API plain_heap : heap
	{
		/// ctor 
		/// \param thp this parameter is ignored.
		explicit plain_heap( memory::thread_policy thp);

		virtual ~plain_heap();

		/// call platform malloc. the parameter alignment and hint are ignored.
		virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint);

		/// call platform free. the parameter alignment is ignored.
		virtual void free(void* p, std::size_t size, std::size_t alignment);

		/// return null for always.
		virtual heap* underling() ;

		virtual bool equal_to(const heap& rhs) const ;

	};

	struct AIO_COMM_API pool_heap : heap
	{
		pool_heap(heap* under, memory::thread_policy thp);
		virtual ~pool_heap();

		virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint) ;
		virtual void free(void* p, std::size_t size, std::size_t alignment);
		virtual heap* underling();

		virtual bool equal_to(const heap& rhs) const;

		virtual void prepare_destroy();
	};


}

#endif //end AIO_COMMON_HEAP_H

