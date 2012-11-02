//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_BUFFER_H
#define AIO_BUFFER_H

#include <aio/common/memory.h>
#include <aio/common/assert.h>
#include <aio/common/range.h>

//STL
#include <algorithm>
#include <type_traits>

namespace aio
{
	template<typename T>
	class buffer
	{
		static_assert(std::is_pod<T>::value, "T must be pod type");

		public:
		typedef heap heap_type;
		typedef T value_type;
		typedef abi_allocator<T> allocator_type;
		typedef typename allocator_type::size_type size_type;
		typedef typename allocator_type::difference_type difference_type;

		typedef typename allocator_type::reference reference;
		typedef typename allocator_type::const_reference const_reference;
		typedef typename allocator_type::pointer pointer;
		typedef typename allocator_type::const_pointer const_pointer;

		typedef pointer iterator;
		typedef const_pointer const_iterator;

		explicit buffer()
			: m_heap(&memory::get_global_heap())
			, m_capacity(0)
			, m_size(0)
			, m_data(0)
		{}

		explicit buffer(heap& h)
			: m_heap(&h)
			, m_capacity(0)
			, m_size(0)
			, m_data(0)
		{}

		buffer(buffer&& rhs)
			: m_heap(rhs.m_heap)
			, m_capacity(rhs.m_capacity)
			, m_size(rhs.m_size)
			, m_data(rhs.m_data)
		{
			rhs.m_capacity = 0;
			rhs.m_size = 0;
			rhs.m_data = 0;
		}
		buffer(const buffer& rhs)
			: m_heap(rhs.m_heap)
			, m_capacity(0)
			, m_size(rhs.m_size)
			, m_data(0)
		{
			if (m_size > 0)
			{
				AIO_PRE_CONDITION(rhs.m_data != 0);

				m_capacity = m_size;
				m_data = malloc_(m_capacity);
				std::copy(rhs.m_data, rhs.m_data + m_size, m_data);
			}
		}

		buffer(const buffer& rhs
				, heap& h)
			: m_heap(&h)
			, m_capacity(0)
			, m_size(rhs.size)
			, m_data(0)
		{
			if (m_size > 0)
			{
				AIO_PRE_CONDITION(rhs.m_data != 0);

				m_capacity = m_size;
				m_data = malloc_(m_capacity);
				std::copy(rhs.m_data, rhs.m_data + m_size, m_data);
			}
		}

		buffer(size_type count, T ch
				, heap& h = memory::get_global_heap())
			: m_heap(&h)
			, m_capacity(0)
			, m_size(count)
			, m_data(0)
		{
			if (m_size > 0)
			{
				m_capacity = m_size;
				m_data = malloc_(m_capacity);
				std::fill(m_data, m_data + m_size, ch);
			}
		}
		template<typename ForwardIterator>
		buffer(const range<ForwardIterator>& r
				, heap& h = memory::get_global_heap())
			: m_heap(&h)
			, m_capacity(0)
			, m_size(0)
			, m_data(0)
		{
			difference_type len = std::distance(r.begin(), r.end());
			AIO_PRE_CONDITION(len >= 0);
			if (len >0)
			{
				m_size = (size_type)len;
				m_capacity = m_size;
				m_data = malloc_(m_capacity);
				std::copy(r.begin(), r.end(), m_data);
			}
		}
		
		//dtor
		~buffer() {
			if (m_data)
				free_(m_data, m_capacity);
		}

		//assignment
		buffer& operator=(const buffer& rhs)
		{
			if (this != &rhs)
				buffer(rhs).swap(*this);
			return *this;
		}
		buffer& operator=(buffer&& rhs)
		{
			buffer(std::move(rhs)).swap(*this);
			return *this;
		}
		buffer& assign(size_type count, T ch)
		{
			buffer(count, ch, *m_heap).swap(*this);
			return *this;
		}

		template<typename ForwardIterator>
		buffer& assign(const range<ForwardIterator>& r)
		{
			buffer(r, *m_heap).swap(*this);
			return *this;
		}
	
		//memory
		size_type capacity() const	{ return m_capacity;}
		size_type size() const	{ return m_size;}
		void reserve(size_type n) 
		{
			if (n > m_capacity)
			{
				pointer np = malloc_(n);
				if (m_data != 0)
				{
					std::copy(m_data, m_data + m_size, np);
					free_(m_data, m_capacity);
				}
				m_data = np;
				m_capacity = n;
			}
		}
		void resize(size_type n)
		{
			if (n > m_capacity)
			{
				reserve(new_cap_(n));
			}
			m_size = n;
		}
		void resize(size_type n, T ch)
		{
			if (n < m_size)
			{
				m_size = n;
			}
			else if (n > m_size)
			{
				if (n > m_capacity)
					reserve(new_cap_(n));
				std::fill(m_data + m_size, m_data + n, ch);
				m_size = n;
			}
		}
		//insert
		template<typename ForwardIterator>
		buffer& insert(iterator pos, const range<ForwardIterator>& r)
		{
			difference_type len = std::distance(r.begin(), r.end());
			AIO_PRE_CONDITION(len >= 0);
			if (m_size + len > m_capacity)
			{
				size_type ncap = new_cap_(m_size + len);

				pointer np = malloc_(ncap);
				pointer ni = std::copy(begin(), pos, np);
				ni = std::copy(r.begin(), r.end(), ni);
				std::copy(pos, end(), ni);
				if (m_data)
					free_(m_data, m_capacity);
				m_capacity = ncap;
				m_data = np;
			}
			else
			{
				std::copy_backward(pos, end(),  m_data + m_size + len);
				std::copy(r.begin(), r.end(),  pos );
			}
			m_size += len;
			return *this;
		}

		buffer& insert(iterator pos, size_type n, T ch)
		{
			if (m_size + n > m_capacity)
			{
				size_type ncap = new_cap_(m_size + n + 1);

				pointer np = malloc_(ncap);
				pointer ni = std::copy(begin(), pos, np);
				std::fill_n(ni, n, ch);
                ni += n;
				std::copy(pos, end(), ni);
				if (m_data)
					free_(m_data, m_capacity);
				m_capacity = ncap;
				m_data = np;
			}
			else
			{
				std::copy_backward(pos, end(),  m_data + m_size + n);
				std::fill_n(pos, n, ch);
			}
			m_size += n;
			return *this;
		}

		//append
		buffer& append(size_type count, T ch)
		{
			return insert(end(), count, ch);
		}

		template<typename ForwardIterator>
		buffer& append(const range<ForwardIterator>& r)
		{
			return insert(end(), r);
		}

		buffer& push_back(T ch)
		{
			if (m_size < m_capacity)
			{
				m_data[m_size++] = ch;
			}
			else
				append(1, ch);
			return *this;
		}
	
		//erase
		iterator erase(iterator p)
		{
			AIO_PRE_CONDITION(p >= begin() && p < end());
			std::copy(p + 1, end(), p);
			--m_size;
			return p;
		}

		iterator erase(const range<iterator>& r)
		{
			AIO_PRE_CONDITION(r.begin() <= r.end());
			AIO_PRE_CONDITION(r.begin() >= begin() && r.begin() <= end());
			AIO_PRE_CONDITION(r.end() >= begin() && r.end() <= end());
			if (!empty())
			{
				std::copy(r.end(), end(), r.begin());
				m_size -= std::distance(r.begin(), r.end());
			}
			return r.begin();
		}

		template<typename ForwardIterator>
		buffer& replace(const range<iterator>& r,
				const range<ForwardIterator>& f)
		{
			AIO_PRE_CONDITION(r.begin() <= r.end());
			AIO_PRE_CONDITION(r.begin() >= begin() && r.begin() <= end());
			AIO_PRE_CONDITION(r.end() >= begin() && r.end() <= end());

			difference_type len1 = std::distance(r.begin(), r.end());
			difference_type len2 = std::distance(f.begin(), f.end());
			if (m_size - len1 + len2 < m_capacity)
			{
				if (len1 < len2) //insert
					std::copy_backward(r.end(), end(), end() - len1 + len2);
				else if (len1 > len2)
					std::copy(r.end(), end(), r.begin() + len2);
				std::copy(f.begin(), f.end(), r.begin());
				m_size = m_size + len2 - len1;
			}
			else //insert
			{
				size_type ncap = new_cap_(m_size - len1 + len2 + 1);
				pointer np = malloc_(ncap);
				pointer ni = std::copy(begin(), r.begin(), np);
				ni = std::copy(f.begin(), f.end(), ni);
				std::copy(r.end(), end(), ni);

				free_(m_data, m_capacity);
				m_data = np;
				m_capacity = ncap;
				m_size = m_size + len2 - len1;
			}
			 return *this;
		}

		buffer& sub(const range<iterator>& r)
		{
			AIO_PRE_CONDITION(r.begin() <= r.end());
			AIO_PRE_CONDITION(r.begin() >= begin() && r.begin() <= end());
			AIO_PRE_CONDITION(r.end() >= begin() && r.end() <= end());

			if (!empty())
			{
				std::copy(r.begin(), r.end(), begin());
				m_size = std::distance(r.begin(), r.end());
			}
			return *this;
		}
		//methods
		void clear()
		{
			if (!empty())
			{
				m_size = 0;
			}
		}
		void swap(buffer& rhs)
		{
			using std::swap;
			swap(m_heap, rhs.m_heap);
			swap(m_capacity, rhs.m_capacity);
			swap(m_size, rhs.m_size);
			swap(m_data, rhs.m_data);
		}


		//queries
		bool empty() const
		{
			return m_size == 0;
		}

		//iterator
		iterator begin()
		{
			return m_data;
		}

		iterator end()
		{
			return  m_data + m_size;
		}

		const_iterator begin() const
		{
			return  m_data;
		}

		const_iterator end() const
		{
			return  m_data + m_size;
		}

		//content access
		pointer data() 
		{
			return m_data;
		}
		//content access
		const_pointer data() const
		{
			return m_data;
		}

		reference operator[](size_type n)
		{
			AIO_PRE_CONDITION(m_data > 0 &&  n < m_size);
			return m_data[n];
		}

		const_reference operator[](size_type n) const
		{
			AIO_PRE_CONDITION(m_data > 0 &&  n < m_size);
			return m_data[n];
		}

		heap& get_heap() const { return *m_heap; }

        bool operator == (const buffer<T>& rhs) const{
            return size() == rhs.size()
                && std::equal(begin(), end(), rhs.begin());
        }
        bool operator != (const buffer<T>& rhs) const{
            return !(*this == rhs);
        }
        bool operator < (const buffer<T>& rhs) const{
            return lexicographical_compare(to_range(*this), to_range(rhs));
        }
        bool operator <= (const buffer<T>& rhs) const{
            return !(*this > rhs);
        }
        bool operator > (const buffer<T>& rhs) const{
            return rhs < *this;
        }
        bool operator >= (const buffer<T>& rhs) const{
            return !(*this < rhs);
        }

	private:		
		pointer malloc_(size_type ncap)
		{
			return reinterpret_cast<pointer>(m_heap->malloc(ncap * sizeof(T), sizeof(T), 0));
		}

		void free_(pointer p, size_type size)
		{
			m_heap->free(p, sizeof(T) * size, sizeof(T));
		}

		size_type new_cap_(size_type n) const
		{
			return
				n < m_capacity
				? m_capacity
				: (n < 2 * m_capacity)
				 ? 2 * m_capacity
				 : n + n / 2;
		}

		heap* m_heap;
		size_type m_capacity;
		size_type m_size;
		T* m_data;
	};

	typedef unsigned char byte;

    typedef buffer<byte> byte_buffer;
}

#endif //end AIO_BUFFER_H

