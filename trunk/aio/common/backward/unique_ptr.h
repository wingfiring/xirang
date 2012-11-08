#ifndef AIO_UNIQUE_PTR_H
#define AIO_UNIQUE_PTR_H

#include <aio/common/assert.h>
#include <type_traits>

namespace aio
{
    /// Primary template, default_delete.
    template<typename T>
    struct default_delete
    {
        default_delete(){}
		template<typename Up, typename = typename
			std::enable_if<std::is_convertible<Up*, T*>::value>::type>
			default_delete(const default_delete<Up>&) { }

        void operator()(T* ptr_) const
        {
            static_assert(sizeof(T)>0, "can't delete pointer to incomplete type");
            delete ptr_;
        }
    };

  /// Specialization, default_delete.
  template<typename T>
    struct default_delete<T[]>
    { 
      void operator()(T* ptr_) const
      {
        static_assert(sizeof(T)>0,                      "can't delete pointer to incomplete type");
        delete [] ptr_;
      }

    private:
      template<typename U> void operator()(U*) const;
    };

    /// 20.7.12.2 unique_ptr for single objects.
    template <typename T, typename Dp = default_delete<T> >
    class unique_ptr
    {   
        T* ptr;
        Dp dp;

    public:
        typedef T*   pointer;
        typedef T                       element_type;
        typedef Dp                       deleter_type;

        explicit unique_ptr(pointer p_ = 0) 
            : ptr(p_), dp()
        {}

        unique_ptr(pointer p_, const deleter_type& d_)
            : ptr(p_), dp(d_) { }

        unique_ptr(unique_ptr&& u)
            : ptr(u.ptr), dp(u.dp)
        {
            u.ptr = 0;
        }

		template<typename Up, typename Ep, typename = typename
			std::enable_if
			<std::is_convertible<typename unique_ptr<Up, Ep>::pointer,
			pointer>::value
				&& ((std::is_reference<Dp>::value
							&& std::is_same<Ep, Dp>::value)
						|| (!std::is_reference<Dp>::value
							&& std::is_convertible<Ep, Dp>::value))>
				::type>
				unique_ptr(unique_ptr<Up, Ep>&& u) 
				: ptr(u.release()), dp(std::forward<Ep>(u.get_deleter()))
				{ }

		// Destructor.
		~unique_ptr()  { reset(); }

		// Assignment.
		unique_ptr& operator=(unique_ptr&& u_) 
		{
			reset(u_.release());
			get_deleter() = u_.get_deleter();
			return *this;
		}

		// Observers.
		T& operator*() const
		{
			AIO_PRE_CONDITION(get() != pointer());
			return *get();
		}

		pointer operator->() const 
		{
			AIO_PRE_CONDITION(get() != pointer());
			return get();
		}

		pointer get() const 
		{ return ptr; }

		deleter_type& get_deleter() 
		{ return dp; }

		const deleter_type& get_deleter() const 
		{ return dp; }

		operator bool() const 
		{ return get() != pointer(); }

		// Modifiers.
		pointer release() 
		{
			pointer p_ = get();
			ptr = pointer();
			return p_;
		}

		void reset(pointer p_ = pointer()) 
		{
			using std::swap;
			if (ptr != pointer())
				get_deleter()(ptr);

			swap(ptr, p_);
		}

		void swap(unique_ptr& u_) 
		{
			using std::swap;
			swap(ptr, u_.ptr);
		}
		// Disable copy from lvalue.
	private:
		unique_ptr(const unique_ptr&);
		unique_ptr& operator=(const unique_ptr&);
	};

	//disable
	template<typename T, typename Dp>
		class unique_ptr<T[], Dp>;

	template<typename T, typename Dp>
		inline void swap(unique_ptr<T, Dp>& x_,
				unique_ptr<T, Dp>& y_) 
		{ x_.swap(y_); }

	template<typename T, typename Dp,
		typename U, typename Ep>
			inline bool
			operator==(const unique_ptr<T, Dp>& x_,
					const unique_ptr<U, Ep>& y_)
			{ return x_.get() == y_.get(); }

	template<typename T, typename Dp,
		typename U, typename Ep>
			inline bool
			operator!=(const unique_ptr<T, Dp>& x_,
					const unique_ptr<U, Ep>& y_)
			{ return x_.get() != y_.get(); }

	template<typename T, typename Dp>
		inline bool
		operator<(const unique_ptr<T, Dp>& x_,
				const unique_ptr<T, Dp>& y_)
		{
			return x_.get() < y_.get();
		}

	template<typename T, typename Dp,
		typename U, typename Ep>
			inline bool
			operator<=(const unique_ptr<T, Dp>& x_,
					const unique_ptr<U, Ep>& y_)
			{ return !(y_ < x_); }


	template<typename T, typename Dp>
		inline bool
		operator>(const unique_ptr<T, Dp>& x_,
				const unique_ptr<T, Dp>& y_)
		{ return (y_ < x_); }


	template<typename T, typename Dp>
		inline bool
		operator>=(const unique_ptr<T, Dp>& x_,
				const unique_ptr<T, Dp>& y_)
		{ return !(x_ < y_); }

}



#endif //end AIO_UNIQUE_PTR_H

