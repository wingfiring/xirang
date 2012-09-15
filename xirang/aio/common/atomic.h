//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_ATOMIC_H
#define AIO_ATOMIC_H

#include <aio/common/config.h>

namespace aio{ namespace atomic{

	struct atomic_flag{ 
#ifdef GNUC_COMPILER_	
	 __attribute__((aligned(sizeof(int))))
#elif defined(MSVC_COMPILER_)
	 __declspec(align(4))
#else
	 alignas(int)
#endif	
		int value;
	};

	template<typename T> struct atomic_t {
#ifdef GNUC_COMPILER_	
	 __attribute__((aligned(sizeof(int))))
#elif defined(MSVC_COMPILER_)
	 __declspec(align(4))
# else
	 alignas(const_max(alignof(int), alignof(T)))
#endif	
		T value;
	};
}}

namespace aio{ namespace atomic{ namespace private_{

#ifdef GNUC_COMPILER_
	inline bool cas_( atomic_t<bool>& target, bool expect, bool update)
	{
		return __sync_bool_compare_and_swap(&target.value, expect, update);
	}

	template<typename T> inline bool cas_( atomic_t<T>& target, T expect, T update)
	{
		return __sync_val_compare_and_swap(&target.value, expect, update) == expect;
	}

	inline void sync_fence_()
	{
		__sync_synchronize();
	}

	inline bool sync_acquire_(atomic_flag& target)
	{
		return __sync_lock_test_and_set(&target.value, 1) == 0;
	}

	inline void sync_release_(atomic_flag& target)
	{
		__sync_lock_release(&target.value);
	}

	template<typename T> inline T sync_add_fetch_(atomic_t<T>& target, T value)
	{
		return __sync_add_and_fetch(&target.value, value);
	}
	template<typename T> inline T sync_sub_fetch_(atomic_t<T>& target, T value)
	{
		return __sync_sub_and_fetch(&target.value, value);
	}
	template<typename T> inline T sync_or_fetch_(atomic_t<T>& target, T value)
	{
		return __sync_or_and_fetch(&target.value, value);
	}
	template<typename T> inline T sync_and_fetch_(atomic_t<T>& target, T value)
	{
		return __sync_and_and_fetch(&target.value, value);
	}
	template<typename T> inline T sync_xor_fetch_(atomic_t<T>& target, T value)
	{
		return __sync_xor_and_fetch(&target.value, value);
	}
	template<typename T> inline T sync_nand_fetch_(atomic_t<T>& target, T value)
	{
		return __sync_nand_and_fetch(&target.value, value);
	}

	template<typename T> inline T sync_fetch_add_(atomic_t<T>& target, T value)
	{
		return __sync_fetch_and_add(&target.value, value);
	}
	template<typename T> inline T sync_fetch_sub_(atomic_t<T>& target, T value)
	{
		return __sync_fetch_and_sub(&target.value, value);
	}
	template<typename T> inline T sync_fetch_or_(atomic_t<T>& target, T value)
	{
		return __sync_fetch_and_or(&target.value, value);
	}
	template<typename T> inline T sync_fetch_and_(atomic_t<T>& target, T value)
	{
		return __sync_fetch_and_and(&target.value, value);
	}
	template<typename T> inline T sync_fetch_xor_(atomic_t<T>& target, T value)
	{
		return __sync_fetch_and_xor(&target.value, value);
	}
	template<typename T> inline T sync_fetch_nand_(atomic_t<T>& target, T value)
	{
		return __sync_fetch_and_nand(&target.value, value);
	}

	template<typename T> inline T sync_fetch_(atomic_t<T>& target)
    {
        return sync_add_fetch_(target, T());
    }

#elif defined(MSVC_COMPILER_)
    
	template<typename T> inline bool cas_( atomic_t<T>& target, T expect, T update)
	{
        return InterlockedCompareExchange((LONG volatile*)&target.value, (LONG)update, (LONG)expect) == (LONG)expect;
	}

	inline void sync_fence_()
	{
		MemoryBarrier();
	}

    template<typename T> inline T sync_fetch_(atomic_t<T>& target)
    {
        return (T) InterlockedCompareExchange((LONG volatile*)&target.value, (LONG)0, (LONG)0);
    }
    inline bool sync_fetch_(atomic_t<bool>& target)
    {
        return InterlockedCompareExchange((LONG volatile*)&target.value, (LONG)0, (LONG)0) != 0;
    }

	inline bool sync_acquire_(atomic_flag& target)
	{
        return InterlockedExchange((LONG volatile*)&target.value, (LONG)1) == 0;
	}

	inline void sync_release_(atomic_flag& target)
	{
        InterlockedExchange((LONG volatile*)&target.value, (LONG)0);
	}

	template<typename T> inline T sync_add_fetch_(atomic_t<T>& target, T value)
	{
        T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = expect + value;
        } while(!cas_(target, expect, update));

        return update;
	}
	template<typename T> inline T sync_sub_fetch_(atomic_t<T>& target, T value)
	{
		return sync_add_fetch_(target, -value);
	}
	template<typename T> inline T sync_or_fetch_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = expect | value;
        }
        while(!cas_(target, expect, update));
        return update;
	}
	template<typename T> inline T sync_and_fetch_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = expect & value;
        }
        while(!cas_(target, expect, update));
        return update;
	}
	template<typename T> inline T sync_xor_fetch_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = expect ^ value;
        }
        while(!cas_(target, expect, update));
        return update;
	}
	template<typename T> inline T sync_nand_fetch_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = ~expect & value;
        }
        while(!cas_(target, expect, update));
        return update;
	}

	template<typename T> inline T sync_fetch_add_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = expect + value;
        }
        while(!cas_(target, expect, update));
        return expect;
	}
	template<typename T> inline T sync_fetch_sub_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = expect - value;
        }
        while(!cas_(target, expect, update));
        return expect;
	}
	template<typename T> inline T sync_fetch_or_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = expect | value;
        }
        while(!cas_(target, expect, update));
        return expect;;
	}
	template<typename T> inline T sync_fetch_and_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = expect & value;
        }
        while(!cas_(target, expect, update));
        return expect;
	}
	template<typename T> inline T sync_fetch_xor_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = expect ^ value;
        }
        while(!cas_(target, expect, update));
        return expect;
	}
	template<typename T> inline T sync_fetch_nand_(atomic_t<T>& target, T value)
	{
		T expect, update;
        do
        {
            expect = sync_fetch_(target);
            update = ~expect & value;
        }
        while(!cas_(target, expect, update));
        return expect;
	}
#else
#error "Unsupport compiler!"
#endif
}}}

namespace aio{ namespace atomic{

	template<typename T> inline bool sync_cas( atomic_t<T>& p, T expect, T update)
	{
		return private_::cas_(p, expect, update);
	}

	inline void sync_fence()
	{
		private_::sync_fence_();
	}

	inline bool sync_acquire(atomic_flag& target)
	{
		return private_::sync_acquire_(target);
	}

	inline void sync_release(atomic_flag& target)
	{
		private_::sync_release_(target);
	}

	// { *ptr op= value; return *ptr; }
	// { *ptr = ~(*ptr & value); return *ptr; }   // nand
    
	template<typename T> inline T sync_add_fetch(atomic_t<T>& target, T value)
	{
		return private_::sync_add_fetch_(target, value);
	}
	template<typename T> inline T sync_sub_fetch(atomic_t<T>& target, T value)
	{
		return private_::sync_sub_fetch_(target, value);
	}
	template<typename T> inline T sync_or_fetch(atomic_t<T>& target, T value)
	{
		return private_::sync_or_fetch_(target, value);
	}
	template<typename T> inline T sync_and_fetch(atomic_t<T>& target, T value)
	{
		return private_::sync_and_fetch_(target, value);
	}
	template<typename T> inline T sync_xor_fetch(atomic_t<T>& target, T value)
	{
		return private_::sync_xor_fetch_(target, value);
	}
	template<typename T> inline T sync_nand_fetch(atomic_t<T>& target, T value)
	{
		return private_::sync_nand_fetch_(target, value);
	}

	// { tmp = *ptr; *ptr op= value; return tmp; }
	// { tmp = *ptr; *ptr = ~(tmp & value); return tmp; }   // nand
	template<typename T> inline T sync_fetch_add(atomic_t<T>& target, T value)
	{
		return private_::sync_fetch_add_(target, value);
	}
	template<typename T> inline T sync_fetch_sub(atomic_t<T>& target, T value)
	{
		return private_::sync_fetch_sub_(target, value);
	}
	template<typename T> inline T sync_fetch_or(atomic_t<T>& target, T value)
	{
		return private_::sync_fetch_or_(target, value);
	}
	template<typename T> inline T sync_fetch_and(atomic_t<T>& target, T value)
	{
		return private_::sync_fetch_and_(target, value);
	}
	template<typename T> inline T sync_fetch_xor(atomic_t<T>& target, T value)
	{
		return private_::sync_fetch_xor_(target, value);
	}
	template<typename T> inline T sync_fetch_nand(atomic_t<T>& target, T value)
	{
		return private_::sync_fetch_nand_(target, value);
	}

	template<typename T> inline T sync_get(atomic_t<T>& target)
	{
		return private_::sync_fetch_(target);
	}

	template<typename T> inline T sync_set(atomic_t<T>& target, T value)
	{
		T tmp = sync_get(target);
		while(!sync_cas(target, tmp, value))
		{
			tmp = sync_get(target);
		}
		return tmp;
	}

}}
#endif //end AIO_ATOMIC_H
