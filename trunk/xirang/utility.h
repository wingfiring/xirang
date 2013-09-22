//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_UTILITY_H
#define AIO_UTILITY_H

#include <xirang/config.h>
#include <xirang/assert.h>

namespace aio
{
	//use to surppress unused variable warning.
	template<typename T>
	void unused(const T& ) {};

	struct indirect_less
	{
		template<typename T>
		bool operator()(const T& lhs, const T& rhs)const
		{
			return *lhs < *rhs;
		}
	};

	template<typename T>
	class init_once
	{
		static int refcount;
	public:
		init_once()
		{
			if (refcount++ == 0)
			  	T::init();
		}
		~init_once()
		{
			if (refcount == 0)
				T::uninit();
		}
	};

	template<typename T>
	int init_once<T>::refcount = 0;

#define AIO_INIT_ONCE(T)\
	namespace { \
	::aio::init_once<T> g_init_once;}

	template<typename T>
	struct init_ptr
	{
		explicit init_ptr(T* p = 0)
		{
			if (m_p)
				check_delete(p);
			else
				m_p = p;

		}

		~init_ptr()
		{
			if (m_p)
				check_delete(m_p);
			m_p = 0;
		}

		T& operator*() const{
			AIO_PRE_CONDITION(m_p != 0);
			return *m_p;
		}

		T* operator->() const{
			AIO_PRE_CONDITION(m_p != 0);
			return m_p;
		}

		T* get() const { return m_p; }

		bool isnull() const { return m_p == 0;}

		void  reset(T* p) 
		{
			AIO_PRE_CONDITION(isnull());
			m_p = p;
		}
	private:
		T* m_p;
		//disable clone
		init_ptr(const init_ptr&);
		init_ptr& operator=(const init_ptr&);
	};
};


#endif //end AIO_UTILITY_H
