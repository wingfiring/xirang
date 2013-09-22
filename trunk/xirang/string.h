//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_STRING_H
#define AIO_STRING_H

#include <xirang/config.h>
#include <xirang/memory.h>
#include <xirang/backward/atomic.h>
#include <xirang/range.h>
#include <xirang/buffer.h>

#include <xirang/string/shared_data.h>
#include <xirang/operators.h>

//STL
#include <iosfwd>
#include <cstddef>
#include <string>		//introduce char_traits and stream

#include <xirang/test_helper.h>
#include <xirang/config/abi_prefix.h>

namespace xirang
{
	//force the signature independs on std::char_traits, 
	//because thre real signature of std::char_traits maybe changed
	template<typename CharT>
	struct char_traits{
		typedef std::char_traits<typename std::remove_cv<CharT>::type> type;
	};
	template<typename T, typename U>
	struct concator;

	template<typename T> struct is_concator{ 
		static const bool value = false;
	};
	template<typename T, typename U> struct is_concator<concator<T, U>>{ 
		static const bool value = true;
	};

	//this class intends to hold literal string
	template< typename CharT >
	class basic_range_string : totally_ordered<basic_range_string<CharT>>
	{
	public:
		typedef typename char_traits<CharT>::type traits_type;	//std::basic_string comptible
		typedef typename traits_type::char_type char_type;
        
		typedef CharT value_type;

		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		typedef value_type& reference;
		typedef value_type* pointer;
		typedef pointer iterator;

		typedef const char_type& const_reference;
		typedef const char_type* const_pointer;
		typedef const_pointer const_iterator;
	
		static const size_type npos = size_type(-1);

		/// \ctor 
		/// \post empty()
		basic_range_string() : m_beg(0), m_end(0){}

		basic_range_string(pointer first, pointer last)
			: m_beg(first), m_end(last)
		{
			AIO_PRE_CONDITION(first <= last);
			AIO_PRE_CONDITION(first == last || first != 0);
		}

		// convert non-const to const 
		template<typename UCharT>
			basic_range_string (const basic_range_string<UCharT>& rhs)
			: m_beg(rhs.begin()), m_end(rhs.end())
			{}


		/// \ctor 
		explicit basic_range_string(pointer str) 
			: m_beg(str), m_end(str + traits_type::length(str))
		{
			AIO_PRE_CONDITION(str != 0);
		}

		/// \ctor
		/// \pre  size == 0 || str != 0
		explicit basic_range_string(pointer str, size_type size)
			: m_beg(str), m_end(str + size)
		{
			AIO_PRE_CONDITION(size == 0 || str != 0);
		}

		/// \ctor
		/// \pre  size == 0 || str != 0
		basic_range_string(const range<pointer>& r)
			: m_beg(r.begin()), m_end(r.end())
		{ }
		template<typename Range, typename Enable = typename std::enable_if<!is_concator<Range>::value, void>::type>
		explicit basic_range_string(const Range& r) 
			: m_beg(0), m_end(0)
		{
			if (!r.empty()){
				m_beg = &*r.begin();
				m_end = m_beg + r.size();
			}
		}
		template<typename Range, typename Enable = typename std::enable_if<!is_concator<Range>::value, void>::type>
		explicit basic_range_string(Range& r) 
			: m_beg(0), m_end(0){
			if (!r.empty()){
				m_beg = &*r.begin();
				m_end = m_beg + r.size();
			}
		}

		/// \return true if size() == 0
		bool empty() const { return m_beg == m_end; }

		size_type size() const { return m_end - m_beg;}

		iterator begin() const { return m_beg;}
		iterator end() const { return m_end;}

		/// \return never return null
		pointer data() const { return  m_beg;}

		/// there is no non-const version
		reference operator[] (size_type index) const{
			AIO_PRE_CONDITION(index >= 0 && index < size());
			return m_beg[index];
		}

		void swap(basic_range_string& rhs)
		{
			std::swap(m_beg, rhs.m_beg);
			std::swap(m_end, rhs.m_end);
		}
		
		friend void swap(basic_range_string& lhs, basic_range_string& rhs) 
		{
			lhs.swap(rhs);
		}

		friend	bool operator < (const basic_range_string<CharT>& lhs
		, const basic_range_string<CharT>& rhs) {

				return (lhs.empty() && !rhs.empty())
					|| (!rhs.empty() && (basic_range_string<CharT>::compare(
						lhs.data(), lhs.size(), rhs.data(), rhs.size()) < 0));
		}

		friend bool operator == (const basic_range_string<CharT>& lhs
				, const basic_range_string<CharT>& rhs)
		{
			return (lhs.empty() && rhs.empty())
			|| (!lhs.empty() && !rhs.empty() && (basic_range_string<CharT>::compare(
					lhs.data(), lhs.size(), rhs.data(), rhs.size()) == 0));
		}

		/// if equal, return 0. if lhs < rhs, return negtive, otherwise return > 0
		static int compare(const_pointer lhs, size_type lhs_size, 
			const_pointer rhs, size_type rhs_size)
		{
            const_pointer lhs_end = lhs + lhs_size;
            const_pointer rhs_end = rhs + rhs_size;
            for (; lhs != lhs_end && rhs != rhs_end; ++lhs, ++rhs)
            {
                int test = *lhs - *rhs;
                if (test != 0) return test;
            }                

            if (lhs != lhs_end) //rhs == rhs_end
                return 1;
            else if(rhs != rhs_end)
                return -1;
            return 0;
		}
	private:
		pointer m_beg;
		pointer m_end;
	};
	template<typename CharT, std::size_t N>
		basic_range_string<CharT> literal(CharT(&src) [N]){
			return basic_range_string<CharT>(src, N-1);
		}

	template<typename CharT, std::size_t N>
		basic_range_string<const CharT> literal(const CharT(&src) [N]){
			return basic_range_string<const CharT>(src, N-1);
		}

	template<typename CharT>
		basic_range_string<CharT> as_range_string(CharT* src){
			AIO_PRE_CONDITION(src);
			return basic_range_string<CharT>(src);
		}

	template<typename CharT>
		basic_range_string<const CharT> as_range_string(const CharT* src){
			return basic_range_string<const CharT>(src);
		}

	typedef basic_range_string<const char> const_range_string;
	typedef basic_range_string<const wchar_t> const_wrange_string;
	typedef basic_range_string<char> range_string;
	typedef basic_range_string<wchar_t> wrange_string;


	///This class intends to be used to hold an immutable string
	template < typename CharT >
	class basic_string : totally_ordered<basic_string<CharT>>
	{
		AIO_ENABLE_TEST;
	public:
		typedef typename char_traits<CharT>::type traits_type;
		typedef typename traits_type::char_type value_type;

		typedef heap heap_type;
		typedef abi_allocator<CharT> allocator_type;
		typedef typename allocator_type::size_type size_type;
		typedef typename allocator_type::difference_type difference_type;

		typedef typename allocator_type::reference reference;
		typedef typename allocator_type::const_reference const_reference;
		typedef typename allocator_type::pointer pointer;
		typedef typename allocator_type::const_pointer const_pointer;

		typedef const_pointer iterator;
		typedef const_pointer const_iterator;

		static const size_type npos = size_type(-1);

	private:
		typedef private_::shared_data<CharT> data_type;
		data_type* m_data;

		static const_pointer empty_string(){
			static value_type empty_(0);
			return &empty_;
		}

		static data_type* new_data2(heap& hp, size_type n)
		{
			AIO_PRE_CONDITION(n > 0);
			data_type* p =  
				reinterpret_cast<data_type* >(hp.malloc(
					sizeof(data_type) +  sizeof(CharT) * n 
				, sizeof(std::size_t), 0));
			p->heap_ptr = &hp;
			p->counter.value = 1;
			p->size = n;
			return p;
		}
		data_type* new_data(heap& hp, const_pointer s, size_type n){
			AIO_PRE_CONDITION(s != 0 && n > 0);

			data_type* p = new_data2(hp, n);
			traits_type::copy(p->data, s, n);
			p->data[n] = CharT();	// for range string, it is not a null-terminate string
            p->hash_self();
			return p;
		}

		struct internal_{};

		basic_string(data_type* buffer, internal_ )
			: m_data(buffer)
		{}

		void add_ref_(){
			m_data->addref();
		}
		void release_ref_(){
			if(0 == m_data->release())
			{
				get_heap().free(m_data, sizeof(data_type) +  sizeof(CharT) * m_data->size, sizeof(std::size_t));
			}
			m_data = 0;
		}
	public:

		basic_string() : m_data(0){}

		basic_string(const basic_range_string<const CharT>& src) 
			: m_data(0)
		{
			m_data = src.size() > 0 ? new_data(get_heap(), src.data(), src.size()) : 0;
		}
		basic_string(const basic_range_string<const CharT>& src, heap& h) 
			: m_data(0)
		{
			m_data = src.size() > 0 ? new_data(h, src.data(), src.size()) : 0;
		}
		
#if 1
		basic_string(const_pointer src) 
			: m_data(0)
		{
			AIO_PRE_CONDITION(src != 0);
			size_type len = traits_type::length(src);
			m_data = len > 0 ? new_data(get_heap(), src, len) : 0;
		}

		basic_string(const_pointer src
					 , heap& h) 
		: m_data(0)
		{
			AIO_PRE_CONDITION(src != 0);
			size_type len = traits_type::length(src);
			m_data = len > 0 ? new_data(h, src, len) : 0;
		}
#endif	
		basic_string(basic_string&& rhs) 
			: m_data(rhs.m_data)
		{
			rhs.m_data = 0;
		}

		basic_string(const basic_string& rhs) 
			: m_data(rhs.m_data)
		{
			if (!empty())
			{
				add_ref_();
			}
		}

		template<typename Range, typename Enable = typename std::enable_if<!is_concator<Range>::value, void>::type>
		explicit basic_string(const Range& r) 
			: m_data(0)
		{
			size_type len = std::distance(r.begin(), r.end());
			if (len > 0)
			{
				m_data  = new_data2(get_heap(), len);
				pointer p = m_data->data;
				for (auto itr = r.begin(); itr != r.end(); ++itr, ++p)
				{
					*p = *itr;
				}
				*p = value_type();
                m_data->hash_self();
			}
		}

		template<typename Range, typename Enable = typename std::enable_if<!is_concator<Range>::value, void>::type>
		explicit basic_string(const Range& r , heap& h) 
		: m_data(0)
		{
			size_type len = std::distance(r.begin(), r.end());
			if (len > 0)
			{
				m_data  = new_data2(h, len);
				pointer p = m_data->data;
				for (auto itr = r.begin(); itr != r.end(); ++itr, ++p)
				{
					*p = *itr;
				}
				*p = value_type();
                m_data->hash_self();
			}
		}
		
		basic_string& operator=(const basic_string& rhs)
		{
			if (this->m_data != rhs.m_data)
				basic_string(rhs).swap(*this);
			return *this;
		}
		basic_string& operator=(basic_string&& rhs) 
		{
			basic_string(std::move(rhs)).swap(*this);
			return *this;
		}
		basic_string& operator=(const basic_range_string<const CharT>& rhs)
		{
			basic_string(rhs, get_heap()).swap(*this);
			return *this;
		}
		template<typename Range, typename Enable = typename std::enable_if<!is_concator<Range>::value, void>::type>
		basic_string& operator=(const Range& r)
		{
			basic_string(r, get_heap()).swap(*this);
			return *this;
		}

		~basic_string()
		{
			if (!empty())
			{
				release_ref_();
			}
		}

		operator basic_range_string<const CharT>() const{
			return range_str();
		}
		basic_range_string<const CharT> range_str() const{
			return empty() 
				? basic_range_string<const CharT>() 
				: basic_range_string<const CharT>(begin(), end());

		}

		void swap(basic_string& rhs)
		{
			using std::swap;
			swap(m_data, rhs.m_data);
		}

		void clear()
		{
			if (!empty())
			{
				release_ref_();
			}
		}

		bool empty() const{
			return m_data == 0;
		}

        size_type hash() const { return m_data ? m_data->hash : 2166136261U;}

		size_type size() const { return empty() ? 0 : m_data->size;}

		const_iterator begin() const { return empty() ? empty_string() : m_data->data;}
		const_iterator end() const { return begin() + size();}
		const_pointer c_str() const { return begin();}
		const_pointer data() const { return begin();}

		value_type operator[] (size_type index) const{
			AIO_PRE_CONDITION(index >= 0 && index < size());
			return m_data->data[index];
		}

		heap& get_heap() const { 
			return empty() ? memory::get_global_heap() : *m_data->heap_ptr;
		}

		template<typename T, typename U>
		friend struct concator;
	};

	template<typename T, typename U>
	struct concator{

		const T* left;
		const U* right;

		template<typename CharT>
		operator basic_string<CharT>() const{
			basic_string<CharT> ret;
			auto ptr = ret.new_data2(ret.get_heap(), size());
			std::size_t pos = 0;
			pos = copy_(ptr->data, 0, this);

			AIO_POST_CONDITION(pos == size());
			ptr->data[pos] = CharT(0);

			ptr->hash_self();
			ret.m_data = ptr;
			return std::move(ret);
		}
		std::size_t size() const{
			return left->size() + right->size();
		}
		private:
		template<typename CT, typename CU>
		friend struct concator;

		template<typename CharT, typename Range>
		static std::size_t copy_(CharT* buf, std::size_t pos, const Range* s){
			std::copy(s->begin(), s->end(), buf + pos);
			return pos + s->size();
		}
		template<typename CharT, typename CT, typename CU>
		static std::size_t copy_(CharT* buf, std::size_t pos, const concator<CT, CU>* s){
			pos = s->copy_(buf, pos, s->left);
			return s->copy_(buf, pos, s->right);
		}

		explicit concator(const T& lhs, const U& rhs) 
			: left(&lhs), right(&rhs)
		{}

		template<typename Str1, typename Str2>
		friend concator<Str1, Str2 > operator<< (const Str1& lhs, const Str2&  rhs);

		template<typename CT, typename CU, typename Str>
		friend concator<concator<CT, CU>, Str> operator<< (concator<CT, CU>&& lhs, const Str& rhs);
		
		template<typename Str, typename CT, typename CU>
		friend concator<Str, concator<CT,CU> > operator<< (const Str& lhs, concator<CT, CU>&& rhs);
		
		template<typename CT1, typename CU1, typename CT2, typename CU2>
		friend concator<concator<CT1, CT2>, concator<CT2,CU2> > operator<< (concator<CT1, CU1>&& lhs, concator<CT2, CU2>&& rhs);

		concator& operator=(const concator& ) = delete;
	};
	template<typename Str1, typename Str2>
	concator<Str1, Str2 > operator<< (const Str1& lhs, const Str2&  rhs){
		return concator<Str1, Str2>(lhs, rhs);
	}
	template<typename CT, typename CU, typename Str>
	concator<concator<CT, CU>, Str> operator<< (concator<CT, CU>&& lhs, const Str& rhs){
		return concator<concator<CT,CU>, Str>(lhs, rhs);
	}
	template<typename Str, typename CT, typename CU>
	concator<Str, concator<CT,CU> > operator<< (const Str& lhs, concator<CT, CU>&& rhs){
		return concator<Str, concator<CT, CU> >(lhs, rhs);
	}
	template<typename CT1, typename CU1, typename CT2, typename CU2>
	concator<concator<CT1, CT2>, concator<CT2,CU2> > operator<< (concator<CT1, CU1>&& lhs, concator<CT2, CU2>&& rhs){
		return concator<concator<CT1, CU1>, concator<CT2, CU2> >(lhs, rhs);
	}

	template<typename CharT>
	void swap(basic_string<CharT>& lhs, basic_string<CharT>& rhs)
	{
		lhs.swap(rhs);
	}

	template<typename CharT>
	bool operator < (const basic_string<CharT>& lhs, const basic_string<CharT>& rhs)
	{
        typedef basic_range_string<const CharT> range_type;
        return lhs.c_str() != rhs.c_str()
            && static_cast<range_type>(lhs) < static_cast<range_type>(rhs);
	}

	template<typename CharT>
	bool operator == (const basic_string<CharT>& lhs
		, const basic_string<CharT>& rhs)
	{
		typedef basic_range_string<const CharT> range_type;
		return lhs.c_str() == rhs.c_str()||
            ( lhs.hash() == rhs.hash()
            && static_cast<range_type>(lhs) == static_cast<range_type>(rhs));
	}

	typedef basic_string<char> string;
	typedef basic_string<wchar_t> wstring;

	template<typename CharT>
	std::basic_ostream<CharT >& operator<<(
		std::basic_ostream<CharT >& os, const basic_string<CharT>& out)
	{
		return os << out.c_str();
	}

	template<typename CharT>
	std::basic_ostream<CharT >& operator>> (
		std::basic_ostream<CharT >& os, const basic_string<CharT>& in)
	{
		std::basic_string<CharT> tmp;
		os >> tmp;
		in = tmp.c_str();
	}

	template<typename CharT>
	std::basic_ostream<CharT >& operator<<(
		std::basic_ostream<CharT >& os, const basic_range_string<const CharT>& out)
	{
		for (typename basic_range_string<const CharT>::const_pointer p (out.data())
			, end(out.data() + out.size()); p != end; ++ p)
			os << *p;
		return os;
	}

	///This class intends to be used to hold an immutable string
	template < typename CharT >
	class basic_string_builder : totally_ordered<basic_string_builder<CharT>>
	{
		AIO_ENABLE_TEST;
	public:
		typedef typename char_traits<CharT>::type traits_type;
		typedef typename traits_type::char_type value_type;

		typedef heap heap_type;
		typedef abi_allocator<CharT> allocator_type;
		typedef typename allocator_type::size_type size_type;
		typedef typename allocator_type::difference_type difference_type;

		typedef typename allocator_type::reference reference;
		typedef typename allocator_type::const_reference const_reference;
		typedef typename allocator_type::pointer pointer;
		typedef typename allocator_type::const_pointer const_pointer;

		typedef pointer iterator;
		typedef const_pointer const_iterator;

		static const size_type npos = size_type(-1);

		//ctor
		explicit basic_string_builder(heap& h = memory::get_global_heap())
			: m_heap(&h)
			, m_capacity(0)
			, m_size(0)
			, m_data(0)
		{}

		basic_string_builder(basic_string_builder&& rhs)
			: m_heap(rhs.m_heap)
			, m_capacity(rhs.m_capacity)
			, m_size(rhs.m_size)
			, m_data(rhs.m_data)
		{
			rhs.m_capacity = 0;
			rhs.m_size = 0;
			rhs.m_data = 0;
		}

		basic_string_builder(const basic_string_builder& rhs)
			: m_heap(rhs.m_heap)
			, m_capacity(0)
			, m_size(rhs.m_size)
			, m_data(0)
		{
			if (m_size > 0)
			{
				AIO_PRE_CONDITION(rhs.m_data != 0);

				m_capacity = m_size + 1;
				m_data = malloc_(m_capacity);
				traits_type::copy(m_data, rhs.m_data, m_capacity);
			}
		}

		basic_string_builder(const basic_range_string<const CharT>& rhs
				, heap& h = memory::get_global_heap())
			: m_heap(&h)
			, m_capacity(0)
			, m_size(rhs.size())
			, m_data(0)
		{
			if (m_size > 0)
			{
				AIO_PRE_CONDITION(rhs.data() != 0);

				m_capacity = m_size + 1;
				m_data = malloc_(m_capacity);
				traits_type::copy(m_data, rhs.data(), m_capacity);
			}
		}

		basic_string_builder(const_pointer rhs, size_type pos = 0, size_type n = npos)
			: m_heap(&memory::get_global_heap())
			, m_capacity(0)
			, m_size(0)
			, m_data(0)
		{
			AIO_PRE_CONDITION(rhs != 0 && pos <= traits_type::length(rhs));
			n = n == npos ? traits_type::length(rhs + pos) : n;

			AIO_PRE_CONDITION(pos + n <= traits_type::length(rhs));

			if (n > 0)
			{
				m_size = n;
				m_capacity = n + 1;
				m_data = malloc_(m_capacity);
				traits_type::copy(m_data, rhs, m_capacity);
			}
		}

		basic_string_builder(const_pointer rhs, heap& h
				, size_type pos = 0, size_type n = npos)
			: m_heap(&h)
			, m_capacity(0)
			, m_size(0)
			, m_data(0)
		{
			AIO_PRE_CONDITION(rhs != 0 && pos <= traits_type::length(rhs));
			n = n == npos ? traits_type::length(rhs + pos) : n;

			AIO_PRE_CONDITION(pos + n <= traits_type::length(rhs));

			if (n > 0)
			{
				m_size = n;
				m_capacity = n + 1;
				m_data = malloc_(m_capacity);
				traits_type::copy(m_data, rhs, m_capacity);
				m_data[m_size] = CharT();
			}
		}
		basic_string_builder(size_type count, CharT ch
				, heap& h = memory::get_global_heap())
			: m_heap(&h)
			, m_capacity(0)
			, m_size(count)
			, m_data(0)
		{
			if (m_size > 0)
			{
				m_capacity = m_size + 1;
				m_data = malloc_(m_capacity);
				std::fill(m_data, m_data + m_size, ch);
				m_data[m_size] = CharT();
			}
		}
		template<typename ForwardIterator>
		basic_string_builder(const range<ForwardIterator>& r
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
				m_capacity = m_size + 1;
				m_data = malloc_(m_capacity);
				std::copy(r.begin(), r.end(), m_data);
				m_data[m_size] = CharT();
			}
		}
		
		basic_string_builder(const basic_string<CharT>& rhs
				, heap& h = memory::get_global_heap())
			: m_heap(&h)
			, m_capacity(0)
			, m_size(rhs.size())
			, m_data(0)
		{
			if (m_size > 0)
			{
				m_capacity = m_size +  1;
				m_data = malloc_(m_capacity);
				traits_type::copy(m_data, rhs.c_str(), m_capacity);
			}
		}

		//dtor
		~basic_string_builder()
		{
			if (m_data)
			{
				free_(m_data, m_capacity);
			}
		}

		//assignment
		basic_string_builder& operator=(const basic_range_string<const CharT> rhs)
		{
			basic_string_builder<CharT>(rhs, *m_heap).swap(*this);
			return *this;
		}

		basic_string_builder& operator=(const basic_string<CharT>& rhs)
		{
			basic_string_builder<CharT>(rhs, *m_heap).swap(*this);
			return *this;
		}
		basic_string_builder& operator=(basic_string_builder&& rhs)
		{
			basic_string_builder<CharT>(rhs, *m_heap).swap(*this);
			return *this;
		}
		basic_string_builder& operator=(const basic_string_builder<CharT>& rhs)
		{
			if (this != &rhs)
			{
				basic_string_builder<CharT>(rhs, *m_heap).swap(*this);
			}
			return *this;
		}
		basic_string_builder& operator=(const_pointer rhs)
		{
			basic_string_builder<CharT>(rhs, *m_heap).swap(*this);
			return *this;
		}
		basic_string_builder& assign(const_pointer rhs
				, size_type pos = 0, size_type n = npos)
		{
			basic_string_builder<CharT>(rhs, *m_heap, pos, n).swap(*this);
			return *this;
		}
		basic_string_builder& assign(size_type count, CharT ch)
		{
			basic_string_builder<CharT>(count, ch, *m_heap).swap(*this);
			return *this;
		}
		template<typename ForwardIterator>
		basic_string_builder& assign(const range<ForwardIterator>& r)
		{
			basic_string_builder<CharT>(r, *m_heap).swap(*this);
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
					traits_type::copy(np, m_data, m_size + 1);
					free_(m_data, m_capacity);
				}
				else
					*np = CharT();
				m_data = np;
				m_capacity = n;
			}
		}
		void resize(size_type n, CharT ch = CharT())
		{
			if (n < m_size)
			{
				m_size = n;
				m_data[m_size] = CharT();
			}
			else if (n > m_size)
			{
				if (n >= m_capacity)
					reserve(new_cap_(n + 1));
				std::fill(m_data + m_size, m_data + n, ch);
				m_size = n;
				m_data[n] = CharT();
			}
		}
		//insert
		template<typename ForwardIterator>
		basic_string_builder& insert(iterator pos, const range<ForwardIterator>& r)
		{
			difference_type len = std::distance(r.begin(), r.end());
			AIO_PRE_CONDITION(len >= 0);
			if (m_size + len >= m_capacity)
			{
				size_type ncap = new_cap_(m_size + len + 1);

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
				std::copy(r.begin(), r.end(),  pos);
			}
			m_size += len;
			m_data[m_size] = CharT();
			return *this;
		}

		basic_string_builder& insert(iterator pos, size_type n, CharT ch)
		{
			if (m_size + n >= m_capacity)
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
			m_data[m_size] = CharT();
			return *this;
		}
		basic_string_builder& insert(iterator pos, const_pointer s)
		{
			AIO_PRE_CONDITION(s != 0);
			size_type len = traits_type::length(s);
			if (m_size + len >= m_capacity)
			{
				size_type ncap = new_cap_(m_size + len + 1);

				pointer np = malloc_(ncap);
				pointer ni = std::copy(begin(), pos, np);
				ni = std::copy(s, s + len, ni);
				std::copy(pos, end(), ni);
				if (m_data)
					free_(m_data, m_capacity);
				m_capacity = ncap;
				m_data = np;
			}
			else
			{
				std::copy_backward(pos, end(),  m_data + m_size + len);
				std::copy(s, s + len, m_data + pos - begin());
			}
			m_size += len;
			m_data[m_size] = CharT();
			return *this;
		}

		//append
		basic_string_builder& operator+=(basic_range_string<const CharT> rhs)
		{
			return insert(end(), to_range(rhs));
		}

		basic_string_builder& operator+=(const basic_string<CharT>& rhs)
		{
			return insert(end(), to_range(rhs));
		}

		basic_string_builder& operator+=( const basic_string_builder<CharT>& rhs)
		{
			return insert(end(), to_range(rhs));
		}

		basic_string_builder& operator+=(const_pointer rhs)
		{
			AIO_PRE_CONDITION(rhs != 0);
			return insert(end(), make_range(rhs, rhs + traits_type::length(rhs)));
		}

		basic_string_builder& append(const_pointer rhs
				, size_type pos = 0, size_type n = npos)
		{
			AIO_PRE_CONDITION(rhs != 0);

			const_pointer first = rhs + pos;

			const_pointer last = first + (n == npos ? traits_type::length(first) : n);

			return insert(end(), make_range(first, last));
		}

		basic_string_builder& append(size_type count, CharT ch)
		{
			return insert(end(), count, ch);
		}

		template<typename ForwardIterator>
		basic_string_builder& append(const range<ForwardIterator>& r)
		{
			return insert(end(), r);
		}
        
		basic_string_builder& append(const basic_string<CharT>& r)
		{
			return insert(end(), to_range(r));
		}

		basic_string_builder& push_back(CharT ch)
		{
			if (m_size + 1 < m_capacity)
			{
				m_data[m_size] = ch;
				m_data[++m_size] = CharT();
			}
			else
				append(1, ch);
			return *this;
		}

		basic_string_builder& pop_back(size_type n = 1)
		{
			AIO_PRE_CONDITION(n <= m_size);
			m_size -= n;
			m_data[m_size] = CharT();
			return *this;
		}
	
		//erase
		iterator erase(iterator p)
		{
			AIO_PRE_CONDITION(p >= begin() && p < end());
			std::copy(p + 1, end, p);
			m_data[--m_size] = CharT();
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
				m_data[m_size] = CharT();
			}
			return r.begin();
		}

		template<typename ForwardIterator>
		basic_string_builder& replace(const range<iterator>& r,
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
				m_data[m_size] = CharT();
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

		basic_string_builder& substr(const range<iterator>& r)
		{
			AIO_PRE_CONDITION(r.begin() <= r.end());
			AIO_PRE_CONDITION(r.begin() >= begin() && r.begin() <= end());
			AIO_PRE_CONDITION(r.end() >= begin() && r.end() <= end());

			if (!empty())
			{
				std::copy(r.begin(), r.end(), begin());
				m_size = std::distance(r.begin(), r.end());
				m_data[m_size] = CharT();
			}
			return *this;
		}
		//methods
		void clear()
		{
			if (!empty())
			{
				m_size = 0;
				m_data[0] = CharT();
			}
		}
		void swap(basic_string_builder& rhs)
		{
			using std::swap;
			swap(m_heap, rhs.m_heap);
			swap(m_capacity, rhs.m_capacity);
			swap(m_size, rhs.m_size);
			swap(m_data, rhs.m_data);
		}


		//extract

		operator basic_range_string<CharT> () 
		{
			return basic_range_string<CharT>(begin(), end());
		}

		operator basic_range_string<const CharT> () const
		{
			return basic_range_string<const CharT>(begin(), end());
		}

        basic_string<CharT> str() const
		{
			return basic_string<CharT>(*this);
		}

		//queries
		bool empty() const
		{
			return m_size == 0;
		}

		//iterator
		iterator begin()
		{
			return m_data? m_data : iterator(-1);
		}

		iterator end()
		{
			return  begin() + m_size;
		}

		const_iterator begin() const
		{
			return m_data? m_data : iterator(-1);
		}

		const_iterator end() const
		{
			return  begin() + m_size;
		}

		//content access
		const_pointer data() const
		{
			return c_str();
		}

		const_pointer c_str() const
		{
			return m_data ? m_data : empty_string_();
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

		/// if equal, return 0. if lhs < rhs, return negtive, otherwise return > 0
		static int compare(const_pointer lhs, size_type lhs_size, 
			const_pointer rhs, size_type rhs_size)
		{
            const_pointer lhs_end = lhs + lhs_size;
            const_pointer rhs_end = rhs + rhs_size;
            for (; lhs != lhs_end && rhs != rhs_end; ++lhs, ++rhs)
            {
                size_t test = *lhs - *rhs;
                if (test != 0) return test;
            }                

            if (lhs != lhs_end) //rhs == rhs_end
                return 1;
            else if(rhs != rhs_end)
                return -1;
            return 0;
		}

	private:		
		pointer malloc_(size_type ncap)
		{
			return reinterpret_cast<pointer>(m_heap->malloc(ncap * sizeof(CharT), sizeof(CharT), 0));
		}

		void free_(pointer p, size_type size)
		{
			m_heap->free(p, sizeof(CharT) * size, sizeof(CharT));
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

		static const_pointer empty_string_()
		{
			const static CharT empty_s = CharT();
			return &empty_s;
		}
		heap* m_heap;
		size_type m_capacity;
		size_type m_size;
		pointer m_data;
	};
	
	template<typename CharT>
	bool operator< ( const basic_string_builder<CharT>& lhs, const basic_string_builder<CharT>& rhs)
	{
		typedef basic_range_string<const CharT> range_type;
		return static_cast<range_type>(lhs) < static_cast<range_type>(rhs);
	}

	//equals
	template<typename CharT>
	bool operator ==(const basic_string_builder<CharT>& lhs, const basic_string_builder<CharT>& rhs)
	{
		typedef basic_range_string<const CharT> range_type;
		return static_cast<range_type>(lhs) == static_cast<range_type>(rhs);
	}

	template<typename CharT>
	void swap(basic_string_builder<CharT>& lhs, basic_string_builder<CharT>& rhs)
	{
		lhs.swap(rhs);
	}

	typedef basic_string_builder<char> string_builder;
	typedef basic_string_builder<wchar_t> wstring_builder;

	template<typename StringType>
	range<buffer<byte>::iterator> string_to_range( StringType & cont)
	{
		return make_range(reinterpret_cast<buffer<byte>::iterator>(cont.begin()), 
				reinterpret_cast<buffer<byte>::iterator>(cont.end()));
	}

	template<typename StringType>
	range<buffer<byte>::const_iterator> string_to_c_range( const StringType & cont)
	{
		return make_range(reinterpret_cast<buffer<byte>::const_iterator>(cont.begin()), 
				reinterpret_cast<buffer<byte>::const_iterator>(cont.end()));
	}
}
#include <xirang/config/abi_suffix.h>

#endif //end AIO_STRING_H

