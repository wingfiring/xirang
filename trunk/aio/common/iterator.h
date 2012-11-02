//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_ITERATOR_T_H
#define AIO_COMMON_ITERATOR_T_H

#include <aio/common/assert.h>
#include <aio/common/range.h>

#include <cstddef>
#include <iterator>     //for iterator_traits
#include <functional> //for std:less

namespace aio
{
	const std::size_t IteratorSizeLimit = 5;

	template<typename Traits> class IteratorImpBase
	{
		public:
			typedef typename Traits::iterator_category iterator_category;
            typedef typename Traits::value_type value_type;
            typedef typename Traits::difference_type difference_type;
            typedef typename Traits::distance_type distance_type;	// retained
            typedef typename Traits::pointer pointer;
            typedef typename Traits::reference reference;

			virtual reference operator*() const = 0;
			virtual void operator++() = 0;
			virtual void operator--() = 0;
			virtual IteratorImpBase<Traits>* clone(IteratorImpBase<Traits>** place) const = 0;
			virtual void destroy() = 0;
			virtual bool equals(const IteratorImpBase<Traits>& rhs) const = 0;
			virtual void swap(IteratorImpBase<Traits>& rhs) = 0;
		protected:
			virtual ~IteratorImpBase() {}
	};

	template<typename Traits, typename RealIterator> 
		class IteratorImp : public IteratorImpBase<Traits>
	{
		public:
			typedef IteratorImp<Traits, RealIterator> SelfType;

			typedef typename Traits::iterator_category iterator_category;
            typedef typename Traits::value_type value_type;
            typedef typename Traits::difference_type difference_type;
            typedef typename Traits::distance_type distance_type;	// retained
            typedef typename Traits::pointer pointer;
            typedef typename Traits::reference reference;

			explicit IteratorImp(const RealIterator& p) : pos(p) {}

			virtual reference operator*() const { return *pos; }

			virtual void operator++() { ++pos; }

			virtual void operator--() { --pos; }

			virtual bool equals(const IteratorImpBase<Traits>& rhs) const
			{	
				AIO_PRE_CONDITION(dynamic_cast<const SelfType*>(&rhs) != 0);
                const SelfType* other = static_cast<const SelfType*>(&rhs);
				return pos == other->pos;
			}

			virtual IteratorImpBase<Traits>* clone(IteratorImpBase<Traits>** place) const
			{
				AIO_PRE_CONDITION(place != 0);

				return create(place, pos);
			}

			virtual void swap(IteratorImpBase<Traits>& rhs) 
			{
				using std::swap;
				AIO_PRE_CONDITION(dynamic_cast<SelfType*>(&rhs) != 0);
                SelfType* other = static_cast<SelfType*>(&rhs);
				swap(pos, other->pos);
			}

			static bool canInplaceCreate() 
			{
				return sizeof(IteratorImp) <= IteratorSizeLimit * sizeof(IteratorImp*);
			}

			static IteratorImpBase<Traits>* create(IteratorImpBase<Traits>** place, const RealIterator& itr)
			{
				AIO_PRE_CONDITION(place != 0);
				if (canInplaceCreate())
				{	
					return new (place) SelfType(itr);
				}
				else
				{
					SelfType* ptr = new SelfType(itr);
					return (place[IteratorSizeLimit - 1] = ptr);
				}
			}

			virtual void destroy() 
			{
				if (canInplaceCreate()) {
					this->~IteratorImp();
				}
				else {
					delete this;
				}
			}
		private:
			RealIterator pos;
	};

	template<typename Traits> class IteratorT
	{
		public:
            typedef typename Traits::iterator_category iterator_category;
            typedef typename Traits::value_type value_type;
            typedef typename Traits::difference_type difference_type;
            typedef typename Traits::distance_type distance_type;	// retained
            typedef typename Traits::pointer pointer;
            typedef typename Traits::reference reference;

			IteratorT () {
				clear_();
			}

			template <typename OtherIter>
			explicit IteratorT (const OtherIter& itr)
			{
                clear_();
				IteratorImp<Traits, OtherIter>::create(m_imp, itr);	
			}

			IteratorT (const IteratorT & rhs)
			{
                clear_();
				if (rhs.get_imp_())
					rhs.get_imp_()->clone(m_imp);
				else
					clear_();
			}

			~IteratorT ()
			{
				if (get_imp_())
				{
					get_imp_()->destroy();
				}
			}

			IteratorT & operator++ () 
			{
				AIO_PRE_CONDITION(valid());
				++(*get_imp_());
				return *this;
			}

			IteratorT operator++ (int)
			{
				AIO_PRE_CONDITION(valid());
				IteratorT tmp (*this);
				++(*this);
				return tmp;
			}

			IteratorT & operator-- ()
			{
				AIO_PRE_CONDITION(valid());
				--(*get_imp_());
				return *this;
			}

			IteratorT operator-- (int)
			{
				AIO_PRE_CONDITION(valid());
				IteratorT tmp (*this);
				--(*this);
				return tmp;
			}

			reference operator* () const
			{
				AIO_PRE_CONDITION(valid());

				return **get_imp_();
			}

			pointer operator-> () const
			{
				AIO_PRE_CONDITION(valid());

				return &**this;
			}

			IteratorT & operator= (const IteratorT & rhs)
			{
				if (this != &rhs)
				{
					IteratorT(rhs).swap(*this);
				}
				return *this;
			}

			void swap (IteratorT & rhs)
			{
				IteratorImpBase<Traits>* this_imp = get_imp_();
				IteratorImpBase<Traits>* other_imp = rhs.get_imp_();
				if (this_imp && other_imp)
				{
					this_imp->swap(*other_imp);
				}
				else  if (this_imp)
				{
					this_imp->clone(rhs.m_imp);
					this_imp->destroy();
					clear_();
				}
				else if (other_imp)
				{
					other_imp->clone(m_imp);
					other_imp->destroy();
					rhs.clear_();
				}
			}

			bool equals (const IteratorT & rhs) const
			{
				return get_imp_() == rhs.get_imp_()
					|| (get_imp_() != 0 && rhs.get_imp_() != 0 && get_imp_()->equals(*rhs.get_imp_()));
			}

			bool valid () const 
			{
				return get_imp_() != 0;
			}
			bool embeded() const{
				return m_imp[0] != 0;
			}
		private:
			IteratorImpBase<Traits>* get_imp_() const {
				return m_imp[0] == 0 ? m_imp[IteratorSizeLimit - 1] : const_cast<IteratorImpBase<Traits>*>(reinterpret_cast<const IteratorImpBase<Traits>*>(m_imp));
			}
			void clear_() {
				for (size_t i = 0; i < IteratorSizeLimit; ++i)
					m_imp[i] = 0;
			}
			IteratorImpBase<Traits>* m_imp[IteratorSizeLimit];			
	};

    template<typename T>
    struct default_itr_traits
    {
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef ptrdiff_t distance_type;	// retained
        typedef typename std::remove_reference<T>::type * pointer;
        typedef typename std::remove_reference<T>::type& reference;
    };

    template<typename T>
    struct const_itr_traits
    {
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef ptrdiff_t distance_type;	// retained
        
        typedef const typename std::remove_reference<T>::type * pointer;
        typedef const typename std::remove_reference<T>::type& reference;
    };

	template <typename Traits>
	void swap(IteratorT<Traits> & lhs, IteratorT<Traits> & rhs)
	{
		lhs.swap(rhs);
	}
	template <typename Traits>
	inline bool operator==(const IteratorT<Traits>& lhs, const IteratorT<Traits>& rhs)
	{ return lhs.equals(rhs);}

	template <typename Traits>
	inline bool operator!=(const IteratorT<Traits>& lhs, const IteratorT<Traits>& rhs)
	{ return !lhs.equals(rhs);}

    template<typename RealIterator, typename Selector, typename Traits = std::iterator_traits<RealIterator> > 
    class select_iterator 
	{
	public:
        typedef typename Traits::iterator_category iterator_category;
        typedef typename Selector::value_type value_type;
        typedef typename Traits::difference_type difference_type;
        //typedef typename Traits::distance_type distance_type;
        typedef typename Selector::pointer pointer;
        typedef typename Selector::reference reference;


        template<typename Itr>
		select_iterator(Itr itr, Selector s = Selector())
			: m_pos(itr), m_selector(s)
		{}

		reference operator*() const{
			return m_selector(*m_pos);
		}
        pointer operator->() const{
			return &**this;
		}

		select_iterator& operator++() {
			++m_pos;
			return *this;
		}

        select_iterator operator++(int) {
            select_iterator tmp = *this;
			++m_pos;
			return tmp;
		}

		select_iterator& operator--() {
			--m_pos;
			return *this;
		}

        select_iterator operator--(int) {
			select_iterator tmp = *this;
			--m_pos;
			return tmp;
		}
		
		bool operator==(const select_iterator& rhs) const
		{
            return m_pos == rhs.m_pos;
		}

        bool operator!=(const select_iterator& rhs) const
		{
			return !(m_pos == rhs.m_pos);
		}

	private:
		RealIterator m_pos;
        Selector m_selector;
	};

    template<typename RealIterator, typename Pred, typename Traits = std::iterator_traits<RealIterator>  >
    class filter_iterator
    {
    public:
        typedef typename Traits::iterator_category iterator_category;
        typedef typename Traits::value_type value_type;
        typedef typename Traits::difference_type difference_type;
        //typedef typename Traits::distance_type distance_type;	// retained
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;

        filter_iterator(){}
        template<typename Itr>
        filter_iterator(const Itr& first, const Itr& last, Pred pred = Pred() ) 
            : m_base(first), m_last(last), m_pred(pred)
        {
            satisfy_predicate_();
        }
		
        filter_iterator(const filter_iterator& other) 
			: m_base(other.m_base), m_last(other.m_last), m_pred(other.m_pred)
        {   
        }

        template<typename Rng>
        filter_iterator(const Rng& rng, Pred pred = Pred ()) 
            : m_base(rng.begin()), m_last(rng.end()), m_pred(pred)
        {
            satisfy_predicate_();
        }
        
        template<typename Rng>
		filter_iterator(Rng& rng, Pred pred = Pred (), typename std::enable_if<!std::is_same<Rng, filter_iterator>::value, void* >::type = 0) 
            : m_base(rng.begin()), m_last(rng.end()), m_pred(pred)
        {
            satisfy_predicate_();
        }

        reference operator*() const{
			return *m_base;
		}
        pointer operator->() const{
			return &*m_base;
		}

		filter_iterator& operator++() {
			++m_base;
            satisfy_predicate_();
			return *this;
		}

        filter_iterator operator++(int) {
            filter_iterator tmp(*this);
			++(*this);
			return tmp;
		}

		filter_iterator& operator--() {
            while(!m_pred(*--m_base)){};
			return *this;
		}

        filter_iterator operator--(int) {
			filter_iterator tmp(*this);
			--(*this);
			return tmp;
		}
		
		bool operator==(const filter_iterator& rhs) const
		{
            return m_base == rhs.m_base;
		}

        bool operator!=(const filter_iterator& rhs) const
		{
			return !(*this == rhs);
		}

    private:
        void satisfy_predicate_()
        {
            while (m_base != m_last && !this->m_pred(*m_base))
              ++m_base;
        }

        RealIterator m_base, m_last;
        Pred m_pred;
        
    };

    template<typename RealIterator, typename Traits = std::iterator_traits<RealIterator>  >
    class joined_iterator
    {
    public:
        typedef typename Traits::iterator_category iterator_category;
        typedef typename Traits::value_type value_type;
        typedef typename Traits::difference_type difference_type;
        ///typedef typename Traits::distance_type distance_type;	// retained
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;

        joined_iterator(){}
        template<typename Itr>
        joined_iterator(const Itr& first1, const Itr& last1, const Itr& first2, bool select_1st = true) 
            : m_base(first1), m_last1(last1), m_first2(first2), m_select_1st(select_1st)
        {
            if (m_select_1st)
                adjust_();
        }

        template<typename Rng>
        joined_iterator(const Rng& rng1, const Rng& rng2) 
            : m_base(rng1.begin()), m_last1(rng1.end()), m_first2(rng2.begin()), m_select_1st(true)
        {
            adjust_();
        }

        template<typename Rng>
        joined_iterator(Rng& rng1, Rng& rng2 ) 
            : m_base(rng1.begin()), m_last1(rng1.end()), m_first2(rng2.begin()), m_select_1st(true)
        {
            adjust_();
        }

        reference operator*() const{
			return *m_base;
		}
        pointer operator->() const{
			return &*m_base;
		}

		joined_iterator& operator++() {
			++m_base;
            adjust_();
			return *this;
		}

        joined_iterator operator++(int) {
            joined_iterator tmp = *this;
			++(*this);
			return tmp;
		}

		joined_iterator& operator--() {
            if (!m_select_1st && m_base == m_first2)
            {
                m_base = m_last1;
                m_select_1st = true;
            }
            --m_base;
			return *this;
		}

        joined_iterator operator--(int) {
			joined_iterator tmp = *this;
			--(*this);
			return tmp;
		}
		
		bool operator==(const joined_iterator& rhs) const
		{
            return m_select_1st == rhs.m_select_1st
                && m_base == rhs.m_base;
		}

        bool operator!=(const joined_iterator& rhs) const
		{
			return !(*this == rhs);
		}

    private:
        void adjust_(){
            if (m_base == m_last1)
            {
                m_select_1st = false;
                m_base = m_first2;
            }
        }
        RealIterator m_base, m_last1, m_first2;
        bool m_select_1st;        
    };

    template<typename RealIterator, typename Traits = std::iterator_traits<RealIterator>
        , typename BinPred = std::less<typename Traits::value_type>  >
    class merge_iterator
    {
    public:
        typedef typename Traits::iterator_category iterator_category;
        typedef typename Traits::value_type value_type;
        typedef typename Traits::difference_type difference_type;
        //typedef typename Traits::distance_type distance_type;	// retained
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;

        merge_iterator(){}
        template<typename Itr>
        merge_iterator(const Itr& first1, const Itr& last1, const Itr& first2, const Itr& last2, BinPred pred = BinPred()) 
            : m_first1(first1), m_last1(last1)
            , m_first2(first2), m_last2(last2)
            , m_base1(first1), m_base2(first2)
            , m_pred(pred)
        {
            adjust_();
        }

        template<typename Rng>
        merge_iterator(const Rng& rng1, const Rng& rng2, BinPred pred = BinPred()) 
            : m_first1(rng1.begin()), m_last1(rng1.end())
            , m_first2(rng2.begin()), m_last2(rng2.end())
            , m_base1(rng1.begin()), m_base2(rng2.begin())
            , m_pred(pred)
        {
            adjust_();
        }

        template<typename Rng>
        merge_iterator(Rng& rng1, Rng& rng2 , BinPred pred = BinPred()) 
            : m_first1(rng1.begin()), m_last1(rng1.end())
            , m_first2(rng2.begin()), m_last2(rng2.end())
            , m_base1(rng1.begin()), m_base2(rng2.begin())
            , m_pred(pred)
        {
            adjust_();
        }

        reference operator*() const{
            return m_select_1st ? *m_base1 : *m_base2;
		}
        pointer operator->() const{
			return &**this;
		}

		merge_iterator& operator++() {
			m_select_1st = m_select_1st 
                ? (++m_base1, (m_base2 == m_last2) || ((m_base1 != m_last1) && m_pred(*m_base1, *m_base2))) 
                : (++m_base2, (m_base1 == m_last1) || ((m_base2 != m_last2) && m_pred(*m_base1, *m_base2)));
			return *this;
		}

        merge_iterator operator++(int) {
            merge_iterator tmp = *this;
			++(*this);
			return tmp;
		}

		merge_iterator& operator--() {
            m_select_1st =  m_select_1st 
                ? (m_base2 == m_first2 ? (--m_base1, true) : (--m_base2, m_base1 != m_last1 && m_pred(*m_base1, *m_base2)))
                : (m_base1 == m_first1 ? (--m_base2, false) : (--m_base1, m_base2 != m_last2 && m_pred(*m_base1, *m_base2)));
            return *this;
		}

        merge_iterator operator--(int) {
			merge_iterator tmp = *this;
			--(*this);
			return tmp;
		}
		
		bool operator==(const merge_iterator& rhs) const
		{
            return m_select_1st == rhs.m_select_1st
                && (m_select_1st ? m_base1 == rhs.m_base1 : m_base2 == rhs.m_base2);
		}

        bool operator!=(const merge_iterator& rhs) const
		{
			return !(*this == rhs);
		}

        void jump_to_end(){
            m_select_1st = true;
            m_base1 = m_last1;
            m_base2 = m_last2;
        }
    private:
        void adjust_(){
            m_select_1st =  m_base2 == m_last2 || (m_base1 != m_last1 && m_pred(*m_base1, *m_base2));
        }
        
        RealIterator m_first1, m_last1, m_first2, m_last2;
        RealIterator m_base1, m_base2;
        BinPred m_pred;
        bool m_select_1st;
    };
}

#endif //end AIO_COMMON_ITERATOR_T_H
