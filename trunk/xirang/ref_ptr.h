//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_REFERENCE_PTR_H
#define AIO_REFERENCE_PTR_H
#include <xirang/config.h>

#include <xirang/config/abi_prefix.h>
namespace aio
{
///handler ptr, it's a value semantic
    template<typename T>
    class ref_ptr
    {
    public:

		///ctor
		template<typename U>
        explicit ref_ptr(U &p) : m_ptr(&p){}

		///ctor
		template<typename U>
        ref_ptr(const ref_ptr<U>& p) : m_ptr(&p.get()){}

		///get hold pointer
        T& get() const {
            return *m_ptr;
        }
        
		///member access
		///@pre !is_nullptr(*this)
        T* get_pointer() const{
            return m_ptr;
        }
        
		 
		///deference
		///@pre !is_nullptr(*this)
        operator T&() const{
            return *m_ptr;
        }

		///swap
		ref_ptr& swap(ref_ptr& rhs)
        {
            std::swap(m_ptr, rhs.m_ptr);
            return *this;
        }

    private:        
        T* m_ptr;
    };

	//swap
    template<typename T>
    void swap(ref_ptr<T>& lhs, ref_ptr<T>& rhs)
    {
        lhs.swap(rhs);
    }		

	template<class T> inline ref_ptr<T> ref(T & t)
	{ 
		return ref_ptr<T>(t);
	}

	template<class T> inline ref_ptr<T const> cref(T const & t)
	{
		return ref_ptr<T const>(t);
	}
}
#include <xirang/config/abi_suffix.h>
#endif //AIO_REFERENCE_PTR_H
