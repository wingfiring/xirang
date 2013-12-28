#include <xirang/memory.h>

#include <xirang/heap.h>
#include <xirang/backward/atomic.h>

#include <typeinfo>

namespace xirang
{
	namespace
	{
		atomic::atomic_t<heap*> g_global_heap = { 0};
		void* default_global_heap = 0;
		static_assert(sizeof (default_global_heap) == sizeof(plain_heap), "plain_heap size is wrong");

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
            virtual offset_range allocate(std::size_t size, std::size_t alignment, offset_range hint) {
                AIO_PRE_CONDITION(false && "not implemented");
                return offset_range();
            }

            virtual void deallocate(offset_range p) {
                AIO_PRE_CONDITION(false && "not implemented");
            }

            virtual void* track_pin(offset_range h) {
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }
            virtual void* pin(offset_range h) {
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual int track_pin_count(offset_range h) const{
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }
            virtual int view_pin_count(offset_range h) const{
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

            virtual std::size_t write(offset_range h, const void* src, std::size_t n){
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual std::size_t read(offset_range, void* dest, std::size_t){
                AIO_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual void sync(offset_range h){
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
			if (atomic::sync_cas<heap*>(g_global_heap, 0, (heap*)&default_global_heap))
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

	offset_range::offset_range() : m_begin(0), m_end(0){}
	offset_range::offset_range(long_offset_t b, long_offset_t e) : m_begin(b), m_end(e){
		AIO_PRE_CONDITION(b <= e);
	}
	long_size_t offset_range::size() const { return m_end - m_begin;}
	bool offset_range::empty() const { return m_begin == m_end; }
	offset_range::operator bool() const { return !empty(); }
	long_offset_t offset_range::begin() const { return m_begin;}
	long_offset_t offset_range::end() const { return m_end;}

	bool offset_range::in(const offset_range& h) const {
		return m_begin >= h.begin()
			&& m_end <= h.end();
	}
	bool offset_range::contains(const offset_range& h) const {
		return h.in(*this);
	}

}

