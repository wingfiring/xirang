//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_ITERATOR_T_H
#define AIO_COMMON_ITERATOR_T_H

#include <xirang/assert.h>
#include <xirang/range.h>

#include <cstddef>
#include <iterator>     //for iterator_traits
#include <functional> //for std:less

#define AIO_ITERATOR_TRAITS_TYPEDEF(Traits)\
	typedef typename Traits::value_type value_type;\
	typedef typename Traits::difference_type difference_type;\
	typedef typename Traits::pointer pointer;\
	typedef typename Traits::reference reference;

#define AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF(Traits)\
	typedef typename Traits::iterator_category iterator_category;\
	AIO_ITERATOR_TRAITS_TYPEDEF(Traits)

namespace aio { namespace itr_{

	const std::size_t IteratorSizeLimit = 5;

	class ibase{
		public:
			virtual ibase* clone_to(void* candidate_place) const = 0;
			virtual void destroy() = 0;
			virtual void swap(ibase& rhs) = 0;
		protected:
			virtual ~ibase(){}
	};

	template<typename Traits> class ioutput : public ibase
	{
		public:
			AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF(Traits);

			virtual void set(const value_type& var) = 0;
			virtual void increase() = 0;
	};


	template<typename Traits> class iinput : public ibase
	{
		public:
			AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF(Traits);

			virtual bool equals(const ibase& rhs) const = 0;
			virtual reference deref() const = 0;
			virtual void increase() = 0;
	};

	template<typename Traits> class iforward : public iinput<Traits> 
	{ };


	template<typename Traits> class ibidirection : public iforward<Traits>
	{
		public:
			AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF(Traits);

			virtual void increase() = 0;
			virtual void decrease() = 0;
	};


	template<typename Traits> class irandom : public ibidirection<Traits>
	{
		public:
			AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF(Traits);

			virtual void advance(difference_type n) = 0;
			virtual difference_type distance(const ibase& rhs) = 0;
			virtual reference element(difference_type idx) = 0;
	};

	template<typename IteratorIFace, typename RealIterator> 
		class imp_base : public IteratorIFace
	{
		public:
			typedef RealIterator real_iterator_type;

			virtual real_iterator_type& get_real() = 0;
			virtual const real_iterator_type& get_real() const = 0;
	};

	template<typename IteratorIFace, typename RealIterator> class imp_output 
		: public imp_base<IteratorIFace, RealIterator>
	{
		public:
			typedef imp_base<IteratorIFace, RealIterator> base_type;
			AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF(base_type);

			virtual void set(const value_type& var) { *this->get_real() = var; }
			virtual void increase() { ++this->get_real(); }
	};

	template<typename IteratorIFace, typename RealIterator> class imp_input 
		: public imp_base<IteratorIFace, RealIterator>
	{
		public:
			typedef imp_base<IteratorIFace, RealIterator> base_type;
			AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF(base_type);
			typedef imp_input<IteratorIFace, RealIterator> self_type;

			virtual reference deref() const { return *this->get_real(); }
			virtual void increase() { ++this->get_real(); }
			virtual bool equals(const ibase& rhs) const
			{	
				AIO_PRE_CONDITION(dynamic_cast<const self_type*>(&rhs) != 0);
				const self_type* other = static_cast<const self_type*>(&rhs);
				return this->get_real() == other->get_real();
			}
	};

	template<typename IteratorIFace, typename RealIterator> class imp_forward 
		: public imp_input<IteratorIFace, RealIterator>
	{ };

	template<typename IteratorIFace, typename RealIterator> class imp_bidir 
		: public imp_forward<IteratorIFace, RealIterator>
	{ 
		public:
			virtual void decrease() { --this->get_real(); }
	};

	template<typename IteratorIFace, typename RealIterator> class imp_random 
		: public imp_bidir<IteratorIFace, RealIterator>
	{ 
		public:
			typedef imp_bidir<IteratorIFace, RealIterator> base_type;
			AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF(base_type);
			typedef imp_random<IteratorIFace, RealIterator> self_type;

			virtual void advance(difference_type n){
				this->get_real() += n;
			}
			virtual difference_type distance(const ibase& rhs){
				AIO_PRE_CONDITION(dynamic_cast<const self_type*>(&rhs) != 0);

				const self_type* other = static_cast<const self_type*>(&rhs);
				return  other->get_real() - this->get_real();
			}
			virtual reference element(difference_type idx){
				return this->get_real()[idx];

			}
	};

	template<typename Base> class imp_wrap: public Base
	{
		public:
			typedef typename Base::real_iterator_type real_iterator_type;
			AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF(Base);

			typedef imp_wrap<Base> self_type;

			explicit imp_wrap(const real_iterator_type& p) : m_pos(p) {}

			virtual real_iterator_type& get_real() { return m_pos;}
			virtual const real_iterator_type& get_real() const { return m_pos;}

			virtual Base* clone_to(void* place) const
			{
				AIO_PRE_CONDITION(place != 0);

				return create(place, m_pos);
			}

			virtual void swap(ibase& rhs) 
			{
				using std::swap;
				AIO_PRE_CONDITION(dynamic_cast<self_type*>(&rhs) != 0);
				self_type* other = static_cast<self_type*>(&rhs);
				swap(m_pos, other->m_pos);
			}

			static bool canInplaceCreate() 
			{
				return sizeof(self_type) <= IteratorSizeLimit * sizeof(self_type*);
			}

			static Base* create(void* place, const real_iterator_type& itr)
			{
				AIO_PRE_CONDITION(place != 0);
				if (canInplaceCreate())
				{	
					return new (place) self_type(itr);
				}
				else
				{
					self_type* ptr = new self_type(itr);
					return (((self_type**)place)[IteratorSizeLimit - 1] = ptr);
				}
			}

			virtual void destroy() 
			{
				if (canInplaceCreate())
					this->~self_type();
				else 
					delete this;
			}
		private:
			real_iterator_type m_pos;

	};

	class iterator_base
	{
		public:
			enum { is_type_erased_iterator = 1};

			bool valid () const 
			{
				return get_base_imp_() != 0;
			}
			bool embeded() const{
				return m_imp[0] != 0;
			}

		protected:
			iterator_base() {
				clear_();
			}
			~iterator_base(){
				if (get_base_imp_())
					get_base_imp_()->destroy();
			}

			template<typename Iter>
			void init_ (const Iter & rhs)
			{
				clear_();
				if (rhs.get_base_imp_())
					rhs.get_base_imp_()->clone_to(m_imp);
			}

			template<typename Iter> Iter& assignment_(Iter& lhs, const Iter& rhs) {
				if (&lhs != &rhs)
					Iter(rhs).swap(lhs);
				return lhs;
			}

			template<typename Iter> static typename Iter::reference deref_(const Iter& itr ) { 
				AIO_PRE_CONDITION(itr.valid());
				return itr.template get_imp_<typename Iter::interface_type>()->deref();
			}

			template<typename Iter> static Iter& increase_(Iter& itr) { 
				AIO_PRE_CONDITION(itr.valid());
				itr.template get_imp_<typename Iter::interface_type>()->increase();
				return itr;
			}

			template<typename Iter> static Iter pos_increase_(Iter& itr){
				AIO_PRE_CONDITION(itr.valid());
				Iter tmp (itr);
				++itr;
				return tmp;
			}

			template<typename Iter> static Iter& decrease_(Iter& itr) { 
				AIO_PRE_CONDITION(itr.valid());
				itr.template get_imp_<typename Iter::interface_type>()->decrease();
				return itr;
			}

			template<typename Iter> static Iter post_decrease_(Iter& itr){
				AIO_PRE_CONDITION(itr.valid());
				Iter tmp (itr);
				--itr;
				return tmp;
			}

			template<typename Iter> static Iter&  advance_(Iter& itr, typename Iter::difference_type n){
				AIO_PRE_CONDITION(itr.valid());
				itr.template get_imp_<typename Iter::interface_type>()->advance(n);
				return itr;
			}

			template<typename Iter> static typename Iter::difference_type 
				distance_(const Iter& lhs, const Iter& rhs){
					AIO_PRE_CONDITION(lhs.valid());
					AIO_PRE_CONDITION(rhs.valid());
					return lhs.template get_imp_<typename Iter::interface_type>()
						->distance(*rhs.template get_imp_<typename Iter::interface_type>());
				}

			template<typename Iter> static typename Iter::reference 
				element_(Iter& itr, typename Iter::difference_type n){
				AIO_PRE_CONDITION(itr.valid());
				return itr.template get_imp_<typename Iter::interface_type>()->element(n);
			}

			void swap_(iterator_base & rhs)
			{
				iterator_base* left = this;
				iterator_base* right = &rhs;

				switch((state_() << 4) + rhs.state_()){
					case 0x0:
					case 0x01:
					case 0x10:
						get_base_imp_()->swap(*rhs.get_base_imp_());
						break;
					case 0x02:
						std::swap(left, right);	//fall through
					case 0x20:
						{
							right->get_base_imp_()->clone_to(left->m_imp);
							right->get_base_imp_()->destroy();
							right->clear_();
						}
						break;
					case 22:
						break;
					default:
						std::swap(rhs.m_imp[IteratorSizeLimit - 1], m_imp[IteratorSizeLimit - 1]);
				}
			}

			int state_() const{
				return m_imp[0]
					? 0
					: (m_imp[IteratorSizeLimit - 1] ? 1 : 2);
			}

			template<typename InterfaceType>
				InterfaceType * get_imp_() const {
					return m_imp[0] == 0 ? static_cast<InterfaceType*>(m_imp[IteratorSizeLimit - 1]) 
						: const_cast<InterfaceType*>(reinterpret_cast<const InterfaceType*>(m_imp));
				}

			ibase* get_base_imp_() const{
				return get_imp_<ibase>();
			}

			void clear_() {
				for (size_t i = 0; i < IteratorSizeLimit; ++i)
					m_imp[i] = 0;
			}

			ibase* m_imp[IteratorSizeLimit];			
		private:
			iterator_base & operator= (const iterator_base & rhs); //disable
			iterator_base(const iterator_base& rhs);
	};
}

template<typename T>
struct default_itr_traits
{
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef T value_type;
	typedef ptrdiff_t difference_type;
	typedef typename std::remove_reference<T>::type * pointer;
	typedef typename std::remove_reference<T>::type& reference;
};

template<typename T>
struct const_itr_traits
{
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef T value_type;
	typedef ptrdiff_t difference_type;

	typedef const typename std::remove_reference<T>::type * pointer;
	typedef const typename std::remove_reference<T>::type& reference;
};

#define AIO_OUTPUT_ITERATOR_BASIC_METHOD_IMPS()\
	self_type & operator= (const self_type & rhs) { return this->assignment_(*this, rhs);}\
	void swap(self_type& rhs) { this->swap_(rhs);}\
	self_type & operator++ () { return this->increase_(*this);}\
	self_type operator++ (int) { return this->pos_increase_(*this);}

#define AIO_ITERATOR_BASIC_METHOD_IMPS()\
	AIO_OUTPUT_ITERATOR_BASIC_METHOD_IMPS()\
	reference operator*() const{ return this->deref_(*this);}\
	pointer operator-> () const { return &**this; }\
	bool equals(const self_type& rhs) const{ return this->equals_(rhs); }

template<typename Traits> class output_iterator : public itr_::iterator_base
{ 
	public:
		typedef std::input_iterator_tag iterator_category;
		AIO_ITERATOR_TRAITS_TYPEDEF(Traits);
		typedef itr_::ioutput<Traits> interface_type;
		typedef output_iterator self_type;

		output_iterator(){}
		template <typename OtherIter> explicit output_iterator (const OtherIter& itr
				, typename std::enable_if<!std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			typedef itr_::imp_wrap<itr_::imp_output<interface_type, OtherIter>> imp_type;
			imp_type::create(this->m_imp, itr);	
		}
		template <typename OtherIter> explicit output_iterator (const OtherIter& itr
				, typename std::enable_if<std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			this->template init_<self_type>(itr);
		}
		output_iterator(const output_iterator& rhs) { 
			this->template init_<self_type>(rhs);
		}

		self_type& operator*() { return *this;}
		self_type& operator=(const value_type& var) {
			this->get_imp_<interface_type>()->set(var);
			return *this;
		}

		AIO_OUTPUT_ITERATOR_BASIC_METHOD_IMPS();
};

template<typename Traits> class input_iterator : public itr_::iterator_base
{ 
	public:
		typedef std::output_iterator_tag iterator_category;
		AIO_ITERATOR_TRAITS_TYPEDEF(Traits);
		typedef itr_::iinput<Traits> interface_type;
		typedef input_iterator self_type;

		input_iterator() {}
		template <typename OtherIter> explicit input_iterator (const OtherIter& itr
				, typename std::enable_if<!std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			typedef itr_::imp_wrap<itr_::imp_input<interface_type, OtherIter>> imp_type;
			imp_type::create(m_imp, itr);	
		}
		template <typename OtherIter> explicit input_iterator (const OtherIter& itr
				, typename std::enable_if<std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			this->template init_<self_type>(itr);
		}
		input_iterator(const input_iterator& rhs) { 
			this->template init_<self_type>(rhs);
		}

		AIO_ITERATOR_BASIC_METHOD_IMPS();
	protected:
		bool equals_ (const input_iterator<Traits> & rhs) const
		{
			auto this_imp = this->template get_imp_<interface_type>();
			auto that_imp = rhs.template get_imp_<interface_type>();
			return this_imp == that_imp
				|| (this_imp != 0 && that_imp != 0 && this_imp->equals(*that_imp));
		}

};

template<typename Traits> class forward_iterator : public input_iterator<Traits>
{ 
	public:
		typedef std::forward_iterator_tag iterator_category;
		AIO_ITERATOR_TRAITS_TYPEDEF(Traits);
		typedef itr_::iforward<Traits> interface_type;
		typedef forward_iterator self_type;

		forward_iterator(){}
		template <typename OtherIter> explicit forward_iterator (const OtherIter& itr
				, typename std::enable_if<!std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			typedef itr_::imp_wrap<itr_::imp_forward<interface_type, OtherIter>> imp_type;
			imp_type::create(this->m_imp, itr);	
		}
		template <typename OtherIter> explicit forward_iterator (const OtherIter& itr
				, typename std::enable_if<std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			this->template init_<self_type>(itr);
		}
		forward_iterator(const forward_iterator& rhs) { 
			this->template init_<self_type>(rhs);
		}

		AIO_ITERATOR_BASIC_METHOD_IMPS();
};

template<typename Traits> class bidir_iterator : public forward_iterator<Traits>
{ 
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		AIO_ITERATOR_TRAITS_TYPEDEF(Traits);
		typedef itr_::ibidirection<Traits> interface_type;
		typedef bidir_iterator self_type;

		bidir_iterator(){}
		template <typename OtherIter> explicit bidir_iterator (const OtherIter& itr
				, typename std::enable_if<!std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			typedef itr_::imp_wrap<itr_::imp_bidir<interface_type, OtherIter>> imp_type;
			imp_type::create(this->m_imp, itr);	
		}
		template <typename OtherIter> explicit bidir_iterator (const OtherIter& itr
				, typename std::enable_if<std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			this->template init_<self_type>(itr);
		}

		bidir_iterator(const bidir_iterator& rhs) { 
			this->template init_<self_type>(rhs);
		}
		AIO_ITERATOR_BASIC_METHOD_IMPS();
		self_type & operator-- () { return this->decrease_(*this);}
		self_type operator-- (int) { return this->post_decrease_(*this);}
};

template<typename Traits> class random_iterator : public bidir_iterator<Traits>
{ 
	public:
		typedef std::random_access_iterator_tag iterator_category;
		AIO_ITERATOR_TRAITS_TYPEDEF(Traits);
		typedef itr_::irandom<Traits> interface_type;
		typedef random_iterator self_type;

		random_iterator(){}
		template <typename OtherIter> explicit random_iterator (const OtherIter& itr
				, typename std::enable_if<!std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			typedef itr_::imp_wrap<itr_::imp_random<interface_type, OtherIter>> imp_type;
			imp_type::create(this->m_imp, itr);	
		}
		template <typename OtherIter> explicit random_iterator (const OtherIter& itr
				, typename std::enable_if<std::is_base_of<OtherIter&, itr_::iterator_base&>::value, null_type>::type *  = 0)
		{
			this->template init_<self_type>(itr);
		}

		random_iterator(const random_iterator& rhs) { 
			this->template init_<self_type>(rhs);
		}
		AIO_ITERATOR_BASIC_METHOD_IMPS();
		self_type & operator-- () { return this->decrease_(*this);}
		self_type operator-- (int) { return this->post_decrease_(*this);}

		self_type& operator+=(difference_type n) { return this->advance_(*this, n);}
		self_type& operator-=(difference_type n) { return this->advance_(*this, -n);}
		difference_type operator-(const self_type& rhs) { return this->distance_(rhs, *this);}
		reference operator[](difference_type idx) const { return this->element_(*this, idx);}
};

template<typename Traits>
random_iterator<Traits> operator+(const random_iterator<Traits>& itr, 
		typename random_iterator<Traits>::difference_type n){
	auto tmp = itr;
	tmp += n;
	return tmp;
}

template<typename Traits>
random_iterator<Traits> operator+(typename random_iterator<Traits>::difference_type n, 
		const random_iterator<Traits>& itr){
	return itr +n;
}
template<typename Traits>
random_iterator<Traits> operator-(const random_iterator<Traits>& itr, 
		typename random_iterator<Traits>::difference_type n){
	auto tmp = itr;
	tmp -= n;
	return tmp;
}

template<typename Traits>
bool operator<(const random_iterator<Traits>& lhs, const random_iterator<Traits>& rhs){
	return lhs.distance(rhs) > 0;
}
template<typename Traits>
bool operator>(const random_iterator<Traits>& lhs, const random_iterator<Traits>& rhs){
	return lhs.distance(rhs) < 0;
}
template<typename Traits>
bool operator<=(const random_iterator<Traits>& lhs, const random_iterator<Traits>& rhs){
	return lhs.distance(rhs) >= 0;
}
template<typename Traits>
bool operator>=(const random_iterator<Traits>& lhs, const random_iterator<Traits>& rhs){
	return lhs.distance(rhs) <= 0;
}

template <typename Traits, template<typename> class IteratorType >
	typename std::enable_if<IteratorType<Traits>::is_type_erased_iterator, void>::type 
swap(IteratorType<Traits> & lhs, IteratorType<Traits> & rhs)
{
	lhs.swap(rhs);
}
template <typename Traits, template<typename> class IteratorType >
	typename std::enable_if<IteratorType<Traits>::is_type_erased_iterator, bool>::type 
operator==(const IteratorType<Traits>& lhs, const IteratorType<Traits>& rhs)
{ return lhs.equals(rhs);}

template <typename Traits, template<typename> class IteratorType >
	typename std::enable_if<IteratorType<Traits>::is_type_erased_iterator, bool>::type 
operator!=(const IteratorType<Traits>& lhs, const IteratorType<Traits>& rhs)
{ return !lhs.equals(rhs);}


template<typename RealIterator, typename Selector, typename Traits = std::iterator_traits<RealIterator> > 
class select_iterator 
{
	public:
		typedef typename Traits::iterator_category iterator_category;
		typedef typename Selector::value_type value_type;
		typedef typename Traits::difference_type difference_type;
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

		template<typename Rng> filter_iterator(Rng& rng, Pred pred = Pred (), 
				typename std::enable_if<!std::is_same<Rng, filter_iterator>::value, null_type>::type* = 0) 
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
		typedef typename Traits::pointer pointer;
		typedef typename Traits::reference reference;

		joined_iterator(){}
		template<typename Itr>
			joined_iterator(const Itr& first1, const Itr& last1, 
					const Itr& first2, bool select_1st = true) 
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
		typedef typename Traits::pointer pointer;
		typedef typename Traits::reference reference;

		merge_iterator(){}
		template<typename Itr>
			merge_iterator(const Itr& first1, const Itr& last1, const Itr& first2, 
					const Itr& last2, BinPred pred = BinPred()) 
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

#undef AIO_ITERATOR_TRAITS_TYPEDEF
#undef AIO_TYPE_ERASED_ITERATOR_TRAITS_TYPEDEF
#undef AIO_ITERATOR_BASIC_METHOD_IMPS
#undef AIO_OUTPUT_ITERATOR_BASIC_METHOD_IMPS

#endif //end AIO_COMMON_ITERATOR_T_H

