#ifndef XIRANG_ARRAY_H
#define XIRANG_ARRAY_H

#include <aio/xirang/type.h>
#include <aio/xirang/object.h>
#include <aio/xirang/typebinder.h>
#include <aio/xirang/xrbase.h>

//AIO
#include <aio/common/iarchive.h>

//STL
#include <iterator>

namespace xirang
{
	typedef aio::byte byte;

	class ArrayImp;

    /// this class implements an array data struct, and the element type is parameterized
	class Array
	{
	public:

        /// Implements stl const iterator concept
		struct const_iterator
		{
            /// stl inter-operate support @stl::iterator_traits
			typedef std::iterator<std::random_access_iterator_tag, ConstCommonObject> base_type;
			typedef base_type::iterator_category iterator_category;
			typedef base_type::value_type value_type;
			typedef base_type::difference_type difference_type;
            typedef const base_type::pointer pointer;
			typedef const base_type::reference reference;


            /// default ctor
			const_iterator() {}

            /// construct the iterator with given type and offset
            /// \param t given type, t.valid() can be true or false
            /// \param p given address. p should be null if !t.valid()            
			const_iterator(Type t, const byte* p) {
                if (p)
                    m_obj = CommonObject(t, const_cast<byte*>(p));
            }

            /// dereference operator
            /// \pre valid()
            /// \return ConstCommonObject
			reference operator*() const
			{
				AIO_PRE_CONDITION(valid());
				return m_obj;
			}

            /// member access operator
            /// \pre valid()
            /// \return ConstCommonObject*
			pointer operator->() const
			{
				AIO_PRE_CONDITION(valid());
				return &m_obj;
			}

            /// prefix increase iterator
            /// \pre valid()
			const_iterator& operator++()
			{
				AIO_PRE_CONDITION(valid());
                Type t = m_obj.type();
                m_obj = CommonObject(m_obj.type(), reinterpret_cast<byte*>(m_obj.data()) + t.payload());
				return *this;
			}

            /// postfix increase iterator
            /// \pre valid()            
			const_iterator operator++(int)
			{
				AIO_PRE_CONDITION(valid());
				const_iterator tmp = *this;
				++*this;
				return tmp;
			}

            /// decrease iterator, prefix
            /// \pre valid()
			const_iterator& operator--()
			{
				AIO_PRE_CONDITION(valid());
				Type t = m_obj.type();
                m_obj = CommonObject(t, reinterpret_cast<byte*>(m_obj.data()) - t.payload());
				return *this;
			}

            /// decrease iterator, postfix
            /// \pre valid()
			const_iterator operator--(int)
			{
				AIO_PRE_CONDITION(valid());
				const_iterator tmp = *this;
				--*this;
				return tmp;
			}

            /// subscript accessor
            /// \pre valid() && n is not out of bound
			value_type operator[](difference_type n) const
			{
				AIO_PRE_CONDITION(valid());
                Type t = m_obj.type();
                return ConstCommonObject(t, reinterpret_cast<byte*>(m_obj.data()) + n * t.payload());
			}

            /// Increase iterator with n steps
            /// \pre valid() && n is not out of bound
			const_iterator& operator+=(difference_type n)
			{
				AIO_PRE_CONDITION(valid());
				Type t = m_obj.type();
                m_obj = CommonObject(t, reinterpret_cast<byte*>(m_obj.data()) + n * t.payload());
				return *this;
			}

            /// Increase iterator with n steps
            /// \pre valid() && n is not out of bound
			const_iterator operator+(difference_type n) const
			{
                if (n == 0)
                    return *this;

				AIO_PRE_CONDITION(valid());
				const_iterator tmp =*this;
				tmp += n;
				return tmp;
			}

            /// decrease iterator with n steps
            /// \pre valid() && n is not out of bound
			const_iterator& operator-=(difference_type n)
			{
				AIO_PRE_CONDITION(valid());
				Type t = m_obj.type();
                m_obj = CommonObject(t, reinterpret_cast<byte*>(m_obj.data()) - n * t.payload());
				return *this;
			}

            /// decrease iterator with n steps
            /// \pre valid() && n is not out of bound
			const_iterator operator-(difference_type n) const
			{
				AIO_PRE_CONDITION(valid());
				const_iterator tmp =*this;
				tmp -= n;
				return tmp;
			}

            /// get data pointer of referenced object
			const byte* data() const { 
                return reinterpret_cast<const byte*>(m_obj.data());
            }

            /// get Type of referenced object, returned type can be invalid.            
            Type type() const{
                return m_obj.type();
            }

            /// return true if hold a valid object
            bool valid() const { return m_obj.valid();}

        protected:
			mutable CommonObject m_obj;
		};

        // Implements stl iterator concept
		struct iterator : public const_iterator
		{
			typedef std::iterator<std::random_access_iterator_tag, CommonObject> base_type;
			typedef base_type::iterator_category iterator_category;
			typedef base_type::value_type value_type;
			typedef base_type::difference_type difference_type;
			typedef base_type::pointer pointer;
			typedef base_type::reference reference;

            /// default ctor
			iterator() : const_iterator(){}

            /// construct the iterator with given type and offset
            /// \param t given type, t.valid() can be true or false
            /// \param p given address. p should be null if !t.valid() 
			iterator(Type t, byte* p) : const_iterator(t, p){}

            /// dereference operator
            /// \pre valid()
            /// \return CommonObject
			reference operator*() const
			{
				AIO_PRE_CONDITION(valid());
				return m_obj;

			}

            /// member access operator
            /// \pre valid()
            /// \return CommonObject*
			pointer operator->() const
			{
				AIO_PRE_CONDITION(valid());
				return &m_obj;
			}

            /// prefix increase iterator
            /// \pre valid()
			iterator& operator++()
			{
				AIO_PRE_CONDITION(valid());
				const_iterator* this_ = this;
                ++(*this_);
				return *this;
			}

            /// postfix increase iterator
            /// \pre valid()
			iterator operator++(int)
			{
				AIO_PRE_CONDITION(valid());
				iterator tmp = *this;
				++*this;
				return tmp;
			}

            /// decrease iterator, prefix
            /// \pre valid()
			iterator& operator--()
			{
				AIO_PRE_CONDITION(valid());
				const_iterator* this_ = this;
                --(*this_);
				return *this;
			}

            /// decrease iterator, postfix
            /// \pre valid()
			iterator operator--(int)
			{
				AIO_PRE_CONDITION(valid());
				iterator tmp = *this;
				--*this;
				return tmp;
			}

            /// subscript accessor
            /// \pre valid() && n is not out of bound
			value_type operator[](difference_type n) const
			{
				AIO_PRE_CONDITION(valid());
				Type t = m_obj.type();
                return CommonObject(t, reinterpret_cast<byte*>(m_obj.data()) + n * t.payload());
			}

            /// Increase iterator with n steps
            /// \pre valid() && n is not out of bound
			iterator& operator+=(difference_type n)
			{
				AIO_PRE_CONDITION(valid());
				Type t = m_obj.type();
                m_obj = CommonObject(t, reinterpret_cast<byte*>(m_obj.data()) + n * t.payload());
				return *this;
			}

            /// Increase iterator with n steps
            /// \pre valid() && n is not out of bound
			iterator operator+(difference_type n) const
			{
				AIO_PRE_CONDITION(valid());
				iterator tmp =*this;
				tmp += n;
				return tmp;
			}

            /// decrease iterator with n steps
            /// \pre valid() && n is not out of bound
			iterator& operator-=(difference_type n)
			{
				AIO_PRE_CONDITION(valid());
				Type t = m_obj.type();
                m_obj = CommonObject(t, reinterpret_cast<byte*>(m_obj.data()) - n * t.payload());
				return *this;
			}

            /// decrease iterator with n steps
            /// \pre valid() && n is not out of bound
			iterator operator-(difference_type n) const
			{
				AIO_PRE_CONDITION(valid());
				iterator tmp =*this;
				tmp -= n;
				return tmp;
			}

            /// get data pointer of referenced object
            /// \pre valid()
			byte* data() const { 
                AIO_PRE_CONDITION(valid());
                return reinterpret_cast<byte*>(m_obj.data());
            }
		};

        /// default ctor
        /// \post !valid()
		Array();


        /// ctor with given heap, ext_heap and type
        /// \pre t.valid()
        /// \post !valid()
		Array(heap& h, ext_heap& eh, Type t );

        /// copy ctor
        /// \post *this == other
		Array(const Array& other);

        /// dtor
		~Array();
		
        /// assign other to this
        /// \post *this == other
		void assign(const Array& other);

        /// assignment
        /// \post *this == other
		Array& operator=(const Array& other);

        /// swap the elements
        /// \throw nothrow
		void swap(Array& other);

        /// \return true if it doesn't specify the element type.
		bool valid() const;

        /// \return element type
		Type type() const;

        /// \pre valid()
		heap& get_heap() const;

        /// \pre valid()
		ext_heap& get_ext_heap() const;

        /// \pre valid()
        /// \return the number of elements
		std::size_t size() const;

        /// \pre valid()
        /// \return size() == 0
		bool empty() const;

        /// \pre valid()
		const_iterator begin() const;

        /// \pre valid()
		iterator begin() ;

        /// \pre valid()
		const_iterator end() const;

        /// \pre valid()
		iterator end() ;

        /// get the first element
        /// \pre valid() && !empty()
        /// \return *begin()
		ConstCommonObject front() const;

        /// get the first element
        /// \pre valid() && !empty()
        /// \return *begin()
		CommonObject front() ;

        /// get the last element
        /// \pre valid() && !empty()
        /// \return *--end()
		ConstCommonObject back() const;

        /// get the last element
        /// \pre valid() && !empty()
        /// \return *--end()
		CommonObject back();

        /// subscript accessor
        /// \pre valid() && idx >= 0 && idx < size()
		CommonObject operator[](std::size_t idx) ;

        /// subscript accessor
        /// \pre valid() && idx >= 0 && idx < size()
		ConstCommonObject operator[](std::size_t idx) const;

        /// append an item at the end
        /// \pre valid() && obj.type() == type()
        /// \post size() increase 1
		void push_back(ConstCommonObject obj);

        /// append an item at the end
        /// \pre valid() && !empty()
        /// \post size() decrease 1
		void pop_back();


        /// insert the obj before position pos
        /// \pre valid() && obj.type() == type()
        /// \post size() increase 1
		void insert(iterator pos, ConstCommonObject obj);

        /// erase the element by given pos, and move forward the rest elements.
        /// \pre valid() && pos >= begin() && pos < end()
        /// \post size() decrease 1
        /// \return returned iterator has the same index as input pos
		iterator erase(iterator pos);

        /// set the array with given size.
        /// if size == size(), no action. 
        ///         > size(), append size - size() element at the end.
        ///         < size(), erase the elements which index great than or equal to size
        /// \pre valid()
        /// \post size() == size
        /// \note if appended elements, each element must be constructed even the type().isPos() is true.
		void resize(std::size_t size);

        /// reserve enough memory to store given elements
        /// if size <= capacity(), no action.
        /// \pre valid()
        /// \post capacity() >= size
		void reserve(std::size_t size);

        /// get the memory capacity. it returns number of elements, not bytes.
        /// \pre valid()
		std::size_t capacity() const;

        /// release all elements
        /// \pre valid()
        /// \post size() == 0
		void clear();

        int compare(const Array& rhs) const;

	private:
		ArrayImp* m_imp;
	};
    DEFINE_COMPARE (Array);

    inline bool operator==(const Array::const_iterator& lhs, const Array::const_iterator& rhs)
    { 
		AIO_PRE_CONDITION(lhs.type() == rhs.type());
		return lhs.data() == rhs.data(); 
	}

    inline bool operator!=(const Array::const_iterator& lhs, const Array::const_iterator& rhs)
    { 
		AIO_PRE_CONDITION(lhs.type() == rhs.type());
		return lhs.data() != rhs.data(); 
	}

    inline bool operator<(const Array::const_iterator& lhs, const Array::const_iterator& rhs)
    { 
		AIO_PRE_CONDITION(lhs.type() == rhs.type());
		return lhs.data() < rhs.data(); 
	}

    inline bool operator>(const Array::const_iterator& lhs, const Array::const_iterator& rhs)
    { 
		AIO_PRE_CONDITION(lhs.type() == rhs.type());
		return lhs.data() > rhs.data(); 
	}

    inline bool operator<=(const Array::const_iterator& lhs, const Array::const_iterator& rhs)
    { 
		AIO_PRE_CONDITION(lhs.type() == rhs.type());
		return lhs.data() <= rhs.data(); 
	}

    inline bool operator>=(const Array::const_iterator& lhs, const Array::const_iterator& rhs)
    { 
		AIO_PRE_CONDITION(lhs.type() == rhs.type());
		return lhs.data() >= rhs.data(); 
	}

	inline Array::const_iterator::difference_type operator-(const Array::const_iterator& lhs,
	      const Array::const_iterator& rhs)
    {
		AIO_PRE_CONDITION(lhs.type() == rhs.type());
		return (lhs.data() - rhs.data()) / lhs.type().payload(); 
	}

    inline Array::const_iterator operator+(Array::const_iterator::difference_type n, const Array::const_iterator& i)
    { 
		return i + n; 
	}

    inline Array::iterator operator+(Array::iterator::difference_type n, const Array::iterator& i)
    { 
		return i + n; 
	}

	namespace serialize{
		template<> struct constructor<Array>{
			static void apply(CommonObject obj, heap& hp, ext_heap& ehp);
		};

		template<> struct serializer<Array> {
			static aio::io::writer& apply(aio::io::writer& ar, ConstCommonObject obj);
		};

		template<> struct deserializer<Array> {
			static aio::io::reader& apply(aio::io::reader& rd, CommonObject obj, heap& inner, ext_heap& ext);
		};

		template<> struct hasher<Array> {
			static size_t apply(ConstCommonObject obj);
		};

		template<> struct comparison<Array> {
			static int apply(ConstCommonObject lhs,ConstCommonObject rhs) ;
		};
	}
}

#endif //end XIRANG_ARRAY_H
