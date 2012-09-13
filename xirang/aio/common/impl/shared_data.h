#ifndef AIO_IMP_SHARED_DATA_H
#define AIO_IMP_SHARED_DATA_H

#include <aio/common/config.h>
#include <aio/common/atomic.h>

namespace aio{ namespace private_{
	template<typename T> struct shared_data
	{
		atomic::atomic_t<std::size_t> counter;
        std::size_t hash;
		std::size_t size;
		T data[1];

		std::size_t addref(){
			std::size_t old = sync_fetch_add(counter, std::size_t(1));
			AIO_PRE_CONDITION(old != 0);
			return old + 1;
		}
		std::size_t release(){
			std::size_t old = sync_fetch_sub(counter, std::size_t(1));
			AIO_PRE_CONDITION(old != 0);
			return old - 1;
		}
		std::size_t count(){
			return sync_get(counter);
		}

        void hash_self(){
            hash = 2166136261U;
            size_t stride = 1 + size / 10;

            for(size_t first = 0; first < size; first += stride)
                hash = 16777619U * hash ^ (size_t)data[first];
        }
	};
}}

#endif //end AIO_IMP_SHARED_DATA_H
