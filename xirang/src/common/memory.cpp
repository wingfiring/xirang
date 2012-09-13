#include <aio/common/memory.h>

#include <aio/common/heap.h>
#include <aio/common/atomic.h>

#include <typeinfo>

namespace aio
{
	namespace
	{
		atomic::atomic_t<heap*> g_global_heap = { 0};
		plain_heap default_global_heap(memory::multi_thread);

        class dummy_extern_heap : public ext_heap
        {

            virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint ) {
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual void free(void* p, std::size_t size, std::size_t alignment ){
                AIO_PRE_CONDITION(false && "not implemented");
            }

            virtual heap* underling(){
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual bool equal_to(const heap& rhs) const {
                return typeid(*this) == typeid(rhs);
            }

            virtual heap* hook(){ return 0;}

            virtual heap* hook(heap* newhook){
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

        public:
            virtual handle allocate(std::size_t size, std::size_t alignment, handle hint) {
                AIO_PRE_CONDITION(false && "not implemented");
                return handle();
            }

            virtual void deallocate(handle p) {
                AIO_PRE_CONDITION(false && "not implemented");
            }

            virtual void* track_pin(handle h) {
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }
            virtual void* pin(handle h) {
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual int track_pin_count(handle h) const{
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }
            virtual int view_pin_count(handle h) const{
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual int track_unpin(void* h){
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }
            virtual int unpin(void* h){
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual std::size_t write(handle h, const void* src, std::size_t n){
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual std::size_t read(handle, void* dest, std::size_t){
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual void sync(handle h){
                AIO_PRE_CONDITION(false && "not implemented");
            }
        };

        dummy_extern_heap default_dummy_extern_heap;
	}

	namespace memory
	{

		AIO_COMM_API void set_global_heap(heap& h) {sync_set(g_global_heap, &h);}

		AIO_COMM_API heap& get_global_heap() { 
			heap * global_heap = sync_get(g_global_heap);
			if (global_heap == 0){
				init_global_heap_once();
				global_heap = sync_get(g_global_heap);
			}
			AIO_PRE_CONDITION(global_heap != 0);
			return *global_heap;
		}

        AIO_COMM_API ext_heap& get_global_ext_heap() { 
            return default_dummy_extern_heap;
		}

		AIO_COMM_API void init_global_heap_once()
		{
			if (atomic::sync_cas<heap*>(g_global_heap, 0, &default_global_heap))
            {
                // it's safe to constrct default_global_heap multi times.
                new (&default_global_heap) plain_heap(memory::multi_thread);
            }
		}
		AIO_COMM_API extern std::size_t max_alignment()
		{
			return 16;
		}
	}


}

