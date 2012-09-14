//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_INTRUSIVE_PTR_H
#define AIO_COMMON_INTRUSIVE_PTR_H

#include <aio/common/config.h>
#include <aio/common/assert.h>
#include <aio/common/backward/unique_ptr.h> //for aio::rv

namespace aio{

//
//  intrusive_ptr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on unqualified calls to
//  
//      void intrusive_ptr_add_ref(T & p);
//      void intrusive_ptr_release(T & p);
//
//  The object is responsible for destroying itself.
//

template<class T> class intrusive_ptr
{
private:

    typedef intrusive_ptr this_type;

public:

    typedef T element_type;

    intrusive_ptr(): px( 0 )
    {
    }

    intrusive_ptr( T * p, bool add_ref = true ): px( p )
    {
        if( px != 0 && add_ref ) intrusive_ptr_add_ref( *px );
    }


    template<class U>
    intrusive_ptr( intrusive_ptr<U> const & rhs, 
        typename aio::enable_if<STDTR1::is_convertible<U, T>::value, void*>::type = 0)
        : px( rhs.get() )
    {
        if( px != 0 ) intrusive_ptr_add_ref(*px );
    }


    intrusive_ptr(intrusive_ptr const & rhs): px( rhs.px )
    {
        if( px != 0 ) intrusive_ptr_add_ref(*px );
    }

    ~intrusive_ptr()
    {
        if( px != 0 ) intrusive_ptr_release( *px );
    }

    template<class U> intrusive_ptr & operator=(intrusive_ptr<U> const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

// Move support
    rv<intrusive_ptr>& move()
    {
        return *reinterpret_cast< rv<intrusive_ptr>* >(this);  
    }
    intrusive_ptr(rv<intrusive_ptr>& rhs): px( rhs.px )
    {
        rhs.px = 0;
    }
    
    intrusive_ptr & operator=(rv<intrusive_ptr>& rhs)
    {
        this_type(  rhs.move() ).swap(*this);
        return *this;
    }

    intrusive_ptr & operator=(intrusive_ptr const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr & operator=(T * rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    void reset()
    {
        this_type().swap( *this );
    }

    void reset( T * rhs )
    {
        this_type( rhs ).swap( *this );
    }

    T * get() const
    {
        return px;
    }

    T & operator*() const
    {
        AIO_PRE_CONDITION( px != 0 );
        return *px;
    }

    T * operator->() const
    {
        AIO_PRE_CONDITION( px != 0 );
        return px;
    }

    typedef T * this_type::*unspecified_bool_type;

    operator unspecified_bool_type() const // never throws
    {
        return px == 0? 0: &this_type::px;
    }

    void swap(intrusive_ptr & rhs)
    {
        T * tmp = px;
        px = rhs.px;
        rhs.px = tmp;
    }

private:

    T * px;
};

template<class T, class U> inline bool operator==(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() != b.get();
}

template<class T, class U> inline bool operator==(intrusive_ptr<T> const & a, U * b)
{
    return a.get() == b;
}

template<class T, class U> inline bool operator!=(intrusive_ptr<T> const & a, U * b)
{
    return a.get() != b;
}

template<class T, class U> inline bool operator==(T * a, intrusive_ptr<U> const & b)
{
    return a == b.get();
}

template<class T, class U> inline bool operator!=(T * a, intrusive_ptr<U> const & b)
{
    return a != b.get();
}

template<class T> inline bool operator<(intrusive_ptr<T> const & a, intrusive_ptr<T> const & b)
{
    return a.get() < b.get();
}

template<class T> void swap(intrusive_ptr<T> & lhs, intrusive_ptr<T> & rhs)
{
    lhs.swap(rhs);
}

// mem_fn support

template<class T> T * get_pointer(intrusive_ptr<T> const & p)
{
    return p.get();
}

template<class T, class U> intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U> const & p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U> intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U> const & p)
{
    return const_cast<T *>(p.get());
}

template<class T, class U> intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U> const & p)
{
    return dynamic_cast<T *>(p.get());
}

}
#endif //end AIO_COMMON_INTRUSIVE_PTR_H
