//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_RANGE_H
#define AIO_COMMON_RANGE_H

#include <aio/common/config.h>
#include <aio/common/iterator_category.h>

//STL
#include <algorithm>

namespace aio
{
	template<typename Iterator>
	struct range
	{
		typedef Iterator iterator;
		typedef std::size_t size_type;
		
		range() : m_beg(), m_end() {}

		template<typename OtherItr>
		range(const OtherItr& first, const OtherItr& last) 
			: m_beg(first), m_end(last) {}

		template<typename Cont> 
		explicit range(Cont& cont)
			: m_beg(cont.begin()), m_end(cont.end())
		{
		}

        template<typename Cont> 
		explicit range(const Cont& cont)
			: m_beg(cont.begin()), m_end(cont.end())
		{
		}

		template<typename UItr> 
		range(const range<UItr>& cont)
			: m_beg(cont.begin()), m_end(cont.end())
		{
		}

		iterator begin() const { return m_beg;}
		iterator end() const { return m_end;}

		bool empty() const { return begin() == end();}
		size_type size() const { return std::distance(begin(), end());}

		/* explicit */ operator bool() const { return begin() != end();}

		void swap(range& rhs)
		{
			using std::swap;
			swap(m_beg, rhs.m_beg);
			swap(m_end, rhs.m_end);
		}
	protected:
		Iterator m_beg, m_end;
	};
	template<typename Iter>
	void swap(range<Iter>& lhs, range<Iter>& rhs)
	{
		lhs.swap(rhs);
	}

	template<typename Iter>
	range<Iter> make_range(Iter  first, Iter  last)
	{
		return range<Iter>(first, last);
	}

	template<typename Cont>
	range<typename Cont::iterator> to_range(Cont& cont)
	{
		return make_range(cont.begin(), cont.end());
	}

    template<typename Cont>
	range<typename Cont::const_iterator> to_range(const Cont& cont)
	{
		return make_range(cont.begin(), cont.end());
	}

	template<typename Range1, typename Range2>
	bool lexicographical_compare( const Range1& lhs, const Range2& rhs)
	{
		return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}
}
#endif //end AIO_COMMON_RANGE_H

