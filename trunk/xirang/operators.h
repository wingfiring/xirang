//  Boost operators.hpp header file  ----------------------------------------//

//  (C) Copyright David Abrahams, Jeremy Siek, Daryle Walker 1999-2001.
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.aio.org/LICENSE_1_0.txt)

//  See http://www.aio.org/libs/utility/operators.htm for documentation.
#ifndef AIO_COMMON_OPERATIONS_H_
#define AIO_COMMON_OPERATIONS_H_
#include <xirang/config.h>
namespace aio{
	namespace private_{
		template<typename T> struct empty_base{};
	}

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct less_than_comparable2 : B
	{
		friend bool operator<=(const T& x, const U& y) { return !static_cast<bool>(x > y); }
		friend bool operator>=(const T& x, const U& y) { return !static_cast<bool>(x < y); }
		friend bool operator>(const U& x, const T& y)  { return y < x; }
		friend bool operator<(const U& x, const T& y)  { return y > x; }
		friend bool operator<=(const U& x, const T& y) { return !static_cast<bool>(y < x); }
		friend bool operator>=(const U& x, const T& y) { return !static_cast<bool>(y > x); }
	};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct less_than_comparable1 : B
	{
		friend bool operator>(const T& x, const T& y)  { return y < x; }
		friend bool operator<=(const T& x, const T& y) { return !static_cast<bool>(y < x); }
		friend bool operator>=(const T& x, const T& y) { return !static_cast<bool>(x < y); }
	};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct equality_comparable2 : B
	{
		friend bool operator==(const U& y, const T& x) { return x == y; }
		friend bool operator!=(const U& y, const T& x) { return !static_cast<bool>(x == y); }
		friend bool operator!=(const T& y, const U& x) { return !static_cast<bool>(y == x); }
	};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct equality_comparable1 : B
	{
		friend bool operator!=(const T& x, const T& y) { return !static_cast<bool>(x == y); }
	};

	// A macro which produces "name_2left" from "name".
#define BOOST_OPERATOR2_LEFT(name) name##2##_##left

	// This is the optimal implementation for ISO/ANSI C++,
	// but it requires the compiler to implement the NRVO.
	// If the compiler has no NRVO, this is the best symmetric
	// implementation available.

#define BOOST_BINARY_OPERATOR_COMMUTATIVE( NAME, OP )                         \
	template <class T, class U, class B = ::aio::private_::empty_base<T> >    \
	struct NAME##2 : B                                                        \
	{                                                                         \
		friend T operator OP( const T& lhs, const U& rhs )                    \
		{ T nrv( lhs ); nrv OP##= rhs; return nrv; }                          \
		friend T operator OP( const U& lhs, const T& rhs )                    \
		{ T nrv( rhs ); nrv OP##= lhs; return nrv; }                          \
	};                                                                        \
	\
	template <class T, class B = ::aio::private_::empty_base<T> >             \
	struct NAME##1 : B                                                        \
	{                                                                         \
		friend T operator OP( const T& lhs, const T& rhs )                    \
		{ T nrv( lhs ); nrv OP##= rhs; return nrv; }                          \
	};

#define BOOST_BINARY_OPERATOR_NON_COMMUTATIVE( NAME, OP )               	\
	template <class T, class U, class B = ::aio::private_::empty_base<T> >  \
	struct NAME##2 : B                                                      \
	{                                                                       \
		friend T operator OP( const T& lhs, const U& rhs )                  \
		{ T nrv( lhs ); nrv OP##= rhs; return nrv; }                        \
	};                                                                      \
	\
	template <class T, class U, class B = ::aio::private_::empty_base<T> >  \
	struct BOOST_OPERATOR2_LEFT(NAME) : B                                   \
	{                                                                       \
		friend T operator OP( const U& lhs, const T& rhs )                  \
		{ T nrv( lhs ); nrv OP##= rhs; return nrv; }                        \
	};                                                                      \
	\
	template <class T, class B = ::aio::private_::empty_base<T> >           \
	struct NAME##1 : B                                                      \
	{                                                                       \
		friend T operator OP( const T& lhs, const T& rhs )                  \
		{ T nrv( lhs ); nrv OP##= rhs; return nrv; }                        \
	};


BOOST_BINARY_OPERATOR_COMMUTATIVE( multipliable, * )
BOOST_BINARY_OPERATOR_COMMUTATIVE( addable, + )
BOOST_BINARY_OPERATOR_NON_COMMUTATIVE( subtractable, - )
BOOST_BINARY_OPERATOR_NON_COMMUTATIVE( dividable, / )
BOOST_BINARY_OPERATOR_NON_COMMUTATIVE( modable, % )
BOOST_BINARY_OPERATOR_COMMUTATIVE( xorable, ^ )
BOOST_BINARY_OPERATOR_COMMUTATIVE( andable, & )
BOOST_BINARY_OPERATOR_COMMUTATIVE( orable, | )

#undef BOOST_BINARY_OPERATOR_COMMUTATIVE
#undef BOOST_BINARY_OPERATOR_NON_COMMUTATIVE
#undef BOOST_OPERATOR2_LEFT

	//  incrementable and decrementable contributed by Jeremy Siek

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct incrementable : B
	{
		friend T operator++(T& x, int)
		{
			incrementable_type nrv(x);
			++x;
			return nrv;
		}
		private: // The use of this typedef works around a Borland bug
		typedef T incrementable_type;
	};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct decrementable : B
	{
		friend T operator--(T& x, int)
		{
			decrementable_type nrv(x);
			--x;
			return nrv;
		}
		private: // The use of this typedef works around a Borland bug
		typedef T decrementable_type;
	};

	//  Iterator operator classes (contributed by Jeremy Siek) ------------------//

	template <class T, class P, class B = ::aio::private_::empty_base<T> >
		struct dereferenceable : B
	{
		P operator->() const
		{ 
			return &*static_cast<const T&>(*this); 
		}
	};

	template <class T, class I, class R, class B = ::aio::private_::empty_base<T> >
		struct indexable : B
	{
		R operator[](I n) const
		{
			return *(static_cast<const T&>(*this) + n);
		}
	};

	//  More operator classes (contributed by Daryle Walker) --------------------//
	//  (NRVO-friendly implementation contributed by Daniel Frey) ---------------//

#define BOOST_BINARY_OPERATOR( NAME, OP )                                     \
	template <class T, class U, class B = ::aio::private_::empty_base<T> >        \
	struct NAME##2 : B                                                            \
	{                                                                             \
		friend T operator OP( const T& lhs, const U& rhs )                          \
		{ T nrv( lhs ); nrv OP##= rhs; return nrv; }                              \
	};                                                                            \
	\
	template <class T, class B = ::aio::private_::empty_base<T> >                 \
	struct NAME##1 : B                                                            \
	{                                                                             \
		friend T operator OP( const T& lhs, const T& rhs )                          \
		{ T nrv( lhs ); nrv OP##= rhs; return nrv; }                              \
	};

	BOOST_BINARY_OPERATOR( left_shiftable, << )
	BOOST_BINARY_OPERATOR( right_shiftable, >> )

#undef BOOST_BINARY_OPERATOR

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct equivalent2 : B
	{
		friend bool operator==(const T& x, const U& y)
		{
			return !static_cast<bool>(x < y) && !static_cast<bool>(x > y);
		}
	};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct equivalent1 : B
	{
		friend bool operator==(const T&x, const T&y)
		{
			return !static_cast<bool>(x < y) && !static_cast<bool>(y < x);
		}
	};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct partially_ordered2 : B
	{
		friend bool operator<=(const T& x, const U& y)
		{ return static_cast<bool>(x < y) || static_cast<bool>(x == y); }
		friend bool operator>=(const T& x, const U& y)
		{ return static_cast<bool>(x > y) || static_cast<bool>(x == y); }
		friend bool operator>(const U& x, const T& y)
		{ return y < x; }
		friend bool operator<(const U& x, const T& y)
		{ return y > x; }
		friend bool operator<=(const U& x, const T& y)
		{ return static_cast<bool>(y > x) || static_cast<bool>(y == x); }
		friend bool operator>=(const U& x, const T& y)
		{ return static_cast<bool>(y < x) || static_cast<bool>(y == x); }
	};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct partially_ordered1 : B
	{
		friend bool operator>(const T& x, const T& y)
		{ return y < x; }
		friend bool operator<=(const T& x, const T& y)
		{ return static_cast<bool>(x < y) || static_cast<bool>(x == y); }
		friend bool operator>=(const T& x, const T& y)
		{ return static_cast<bool>(y < x) || static_cast<bool>(x == y); }
	};

	//  Combined operator classes (contributed by Daryle Walker) ----------------//

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct totally_ordered2
		: less_than_comparable2<T, U
		  , equality_comparable2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct totally_ordered1
		: less_than_comparable1<T
		  , equality_comparable1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct additive2
		: addable2<T, U
		  , subtractable2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct additive1
		: addable1<T
		  , subtractable1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct multiplicative2
		: multipliable2<T, U
		  , dividable2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct multiplicative1
		: multipliable1<T
		  , dividable1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct integer_multiplicative2
		: multiplicative2<T, U
		  , modable2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct integer_multiplicative1
		: multiplicative1<T
		  , modable1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct arithmetic2
		: additive2<T, U
		  , multiplicative2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct arithmetic1
		: additive1<T
		  , multiplicative1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct integer_arithmetic2
		: additive2<T, U
		  , integer_multiplicative2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct integer_arithmetic1
		: additive1<T
		  , integer_multiplicative1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct bitwise2
		: xorable2<T, U
		  , andable2<T, U
		  , orable2<T, U, B
		  > > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct bitwise1
		: xorable1<T
		  , andable1<T
		  , orable1<T, B
		  > > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct unit_steppable
		: incrementable<T
		  , decrementable<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct shiftable2
		: left_shiftable2<T, U
		  , right_shiftable2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct shiftable1
		: left_shiftable1<T
		  , right_shiftable1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct ring_operators2
		: additive2<T, U
		  , subtractable2_left<T, U
		  , multipliable2<T, U, B
		  > > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct ring_operators1
		: additive1<T
		  , multipliable1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct ordered_ring_operators2
		: ring_operators2<T, U
		  , totally_ordered2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct ordered_ring_operators1
		: ring_operators1<T
		  , totally_ordered1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct field_operators2
		: ring_operators2<T, U
		  , dividable2<T, U
		  , dividable2_left<T, U, B
		  > > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct field_operators1
		: ring_operators1<T
		  , dividable1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct ordered_field_operators2
		: field_operators2<T, U
		  , totally_ordered2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct ordered_field_operators1
		: field_operators1<T
		  , totally_ordered1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct euclidian_ring_operators2
		: ring_operators2<T, U
		  , dividable2<T, U
		  , dividable2_left<T, U
		  , modable2<T, U
		  , modable2_left<T, U, B
		  > > > > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct euclidian_ring_operators1
		: ring_operators1<T
		  , dividable1<T
		  , modable1<T, B
		  > > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct ordered_euclidian_ring_operators2
		: totally_ordered2<T, U
		  , euclidian_ring_operators2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct ordered_euclidian_ring_operators1
		: totally_ordered1<T
		  , euclidian_ring_operators1<T, B
		  > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct euclidean_ring_operators2
		: ring_operators2<T, U
		  , dividable2<T, U
		  , dividable2_left<T, U
		  , modable2<T, U
		  , modable2_left<T, U, B
		  > > > > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct euclidean_ring_operators1
		: ring_operators1<T
		  , dividable1<T
		  , modable1<T, B
		  > > > {};

	template <class T, class U, class B = ::aio::private_::empty_base<T> >
		struct ordered_euclidean_ring_operators2
		: totally_ordered2<T, U
		  , euclidean_ring_operators2<T, U, B
		  > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct ordered_euclidean_ring_operators1
		: totally_ordered1<T
		  , euclidean_ring_operators1<T, B
		  > > {};

	template <class T, class P, class B = ::aio::private_::empty_base<T> >
		struct input_iteratable
		: equality_comparable1<T
		  , incrementable<T
		  , dereferenceable<T, P, B
		  > > > {};

	template <class T, class B = ::aio::private_::empty_base<T> >
		struct output_iteratable
		: incrementable<T, B
		  > {};

	template <class T, class P, class B = ::aio::private_::empty_base<T> >
		struct forward_iteratable
		: input_iteratable<T, P, B
		  > {};

	template <class T, class P, class B = ::aio::private_::empty_base<T> >
		struct bidirectional_iteratable
		: forward_iteratable<T, P
		  , decrementable<T, B
		  > > {};

	//  To avoid repeated derivation from equality_comparable,
	//  which is an indirect base class of bidirectional_iterable,
	//  random_access_iteratable must not be derived from totally_ordered1
	//  but from less_than_comparable1 only. (Helmut Zeisel, 02-Dec-2001)
	template <class T, class P, class D, class R, class B = ::aio::private_::empty_base<T> >
		struct random_access_iteratable
		: bidirectional_iteratable<T, P
		  , less_than_comparable1<T
		  , additive2<T, D
		  , indexable<T, D, R, B
		  > > > > {};

	// is_chained_base<> - a traits class used to distinguish whether an operator
	// template argument is being used for base class chaining, or is specifying a
	// 2nd argument type.

	// A type parameter is used instead of a plain bool because Borland's compiler
	// didn't cope well with the more obvious non-type template parameter.
	namespace private_ {
		struct true_t {};
		struct false_t {};
	} // namespace private_

	// Unspecialized version assumes that most types are not being used for base
	// class chaining. We specialize for the operator templates defined in this
	// library.
	template<class T> struct is_chained_base {
		typedef ::aio::private_::false_t value;
	};


	// Import a 4-type-argument operator template into aio (if necessary) and
	// provide a specialization of 'is_chained_base<>' for it.
# define BOOST_OPERATOR_TEMPLATE4(template_name4)                     \
	template<class T, class U, class V, class W, class B>               \
	struct is_chained_base< ::aio::template_name4<T, U, V, W, B> > {  \
		typedef ::aio::private_::true_t value;                            \
	};

	// Import a 3-type-argument operator template into aio (if necessary) and
	// provide a specialization of 'is_chained_base<>' for it.
# define BOOST_OPERATOR_TEMPLATE3(template_name3)                     \
	template<class T, class U, class V, class B>                        \
	struct is_chained_base< ::aio::template_name3<T, U, V, B> > {     \
		typedef ::aio::private_::true_t value;                            \
	};

	// Import a 2-type-argument operator template into aio (if necessary) and
	// provide a specialization of 'is_chained_base<>' for it.
# define BOOST_OPERATOR_TEMPLATE2(template_name2)                  \
	template<class T, class U, class B>                              \
	struct is_chained_base< ::aio::template_name2<T, U, B> > {     \
		typedef ::aio::private_::true_t value;                         \
	};

	// Import a 1-type-argument operator template into aio (if necessary) and
	// provide a specialization of 'is_chained_base<>' for it.
# define BOOST_OPERATOR_TEMPLATE1(template_name1)                  \
	template<class T, class B>                                       \
	struct is_chained_base< ::aio::template_name1<T, B> > {        \
		typedef ::aio::private_::true_t value;                         \
	};

	// BOOST_OPERATOR_TEMPLATE(template_name) defines template_name<> such that it
	// can be used for specifying both 1-argument and 2-argument forms. Requires the
	// existence of two previously defined class templates named '<template_name>1'
	// and '<template_name>2' which must implement the corresponding 1- and 2-
	// argument forms.
	//
	// The template type parameter O == is_chained_base<U>::value is used to
	// distinguish whether the 2nd argument to <template_name> is being used for
	// base class chaining from another aio operator template or is describing a
	// 2nd operand type. O == true_t only when U is actually an another operator
	// template from the library. Partial specialization is used to select an
	// implementation in terms of either '<template_name>1' or '<template_name>2'.
	//

# define BOOST_OPERATOR_TEMPLATE(template_name)                    \
	template <class T                                                  \
	,class U = T                                              \
	,class B = ::aio::private_::empty_base<T>                 \
	,class O = typename is_chained_base<U>::value             \
	>                                                         \
	struct template_name : template_name##2<T, U, B> {};               \
	\
	template<class T, class U, class B>                                \
	struct template_name<T, U, B, ::aio::private_::true_t>             \
	: template_name##1<T, U> {};                                     \
	\
	template <class T, class B>                                        \
	struct template_name<T, T, B, ::aio::private_::false_t>            \
	: template_name##1<T, B> {};                                     \
	\
	template<class T, class U, class B, class O>                       \
	struct is_chained_base< ::aio::template_name<T, U, B, O> > {     \
		typedef ::aio::private_::true_t value;                           \
	};                                                                 \
	\
	BOOST_OPERATOR_TEMPLATE2(template_name##2)                         \
	BOOST_OPERATOR_TEMPLATE1(template_name##1)


	BOOST_OPERATOR_TEMPLATE(less_than_comparable)
	BOOST_OPERATOR_TEMPLATE(equality_comparable)
	BOOST_OPERATOR_TEMPLATE(multipliable)
	BOOST_OPERATOR_TEMPLATE(addable)
	BOOST_OPERATOR_TEMPLATE(subtractable)
	BOOST_OPERATOR_TEMPLATE2(subtractable2_left)
	BOOST_OPERATOR_TEMPLATE(dividable)
	BOOST_OPERATOR_TEMPLATE2(dividable2_left)
	BOOST_OPERATOR_TEMPLATE(modable)
	BOOST_OPERATOR_TEMPLATE2(modable2_left)
	BOOST_OPERATOR_TEMPLATE(xorable)
	BOOST_OPERATOR_TEMPLATE(andable)
	BOOST_OPERATOR_TEMPLATE(orable)

	BOOST_OPERATOR_TEMPLATE1(incrementable)
	BOOST_OPERATOR_TEMPLATE1(decrementable)

	BOOST_OPERATOR_TEMPLATE2(dereferenceable)
	BOOST_OPERATOR_TEMPLATE3(indexable)

	BOOST_OPERATOR_TEMPLATE(left_shiftable)
	BOOST_OPERATOR_TEMPLATE(right_shiftable)
	BOOST_OPERATOR_TEMPLATE(equivalent)
	BOOST_OPERATOR_TEMPLATE(partially_ordered)

	BOOST_OPERATOR_TEMPLATE(totally_ordered)
	BOOST_OPERATOR_TEMPLATE(additive)
	BOOST_OPERATOR_TEMPLATE(multiplicative)
	BOOST_OPERATOR_TEMPLATE(integer_multiplicative)
	BOOST_OPERATOR_TEMPLATE(arithmetic)
	BOOST_OPERATOR_TEMPLATE(integer_arithmetic)
	BOOST_OPERATOR_TEMPLATE(bitwise)
	BOOST_OPERATOR_TEMPLATE1(unit_steppable)
	BOOST_OPERATOR_TEMPLATE(shiftable)
	BOOST_OPERATOR_TEMPLATE(ring_operators)
	BOOST_OPERATOR_TEMPLATE(ordered_ring_operators)
	BOOST_OPERATOR_TEMPLATE(field_operators)
	BOOST_OPERATOR_TEMPLATE(ordered_field_operators)
	BOOST_OPERATOR_TEMPLATE(euclidian_ring_operators)
	BOOST_OPERATOR_TEMPLATE(ordered_euclidian_ring_operators)
	BOOST_OPERATOR_TEMPLATE(euclidean_ring_operators)
	BOOST_OPERATOR_TEMPLATE(ordered_euclidean_ring_operators)
	BOOST_OPERATOR_TEMPLATE2(input_iteratable)
	BOOST_OPERATOR_TEMPLATE1(output_iteratable)
	BOOST_OPERATOR_TEMPLATE2(forward_iteratable)
	BOOST_OPERATOR_TEMPLATE2(bidirectional_iteratable)
	BOOST_OPERATOR_TEMPLATE4(random_access_iteratable)

#undef BOOST_OPERATOR_TEMPLATE
#undef BOOST_OPERATOR_TEMPLATE4
#undef BOOST_OPERATOR_TEMPLATE3
#undef BOOST_OPERATOR_TEMPLATE2
#undef BOOST_OPERATOR_TEMPLATE1

	// The following 'operators' classes can only be used portably if the derived class
	// declares ALL of the required member operators.
	template <class T, class U>
		struct operators2
	: totally_ordered2<T,U
	  , integer_arithmetic2<T,U
	  , bitwise2<T,U
	  > > > {};

	template <class T, class U = T>
		struct operators : operators2<T, U> {};

	template <class T> struct operators<T, T>
		: totally_ordered<T
		, integer_arithmetic<T
		, bitwise<T
		, unit_steppable<T
		> > > > {};

#if 0 	//disable iterator

	//  Iterator helper classes (contributed by Jeremy Siek) -------------------//
	//  (Input and output iterator helpers contributed by Daryle Walker) -------//
	//  (Changed to use combined operator classes by Daryle Walker) ------------//
	template <class T,
			 class V,
			 class D = std::ptrdiff_t,
			 class P = V const *,
			 class R = V const &>
		 struct input_iterator_helper
				 : input_iteratable<T, P
				   , aio::iterator<std::input_iterator_tag, V, D, P, R
				   > > {};

	template<class T>
		struct output_iterator_helper
		: output_iteratable<T
		  , aio::iterator<std::output_iterator_tag, void, void, void, void
		  > >
	{
		T& operator*()  { return static_cast<T&>(*this); }
		T& operator++() { return static_cast<T&>(*this); }
	};

	template <class T,
			 class V,
			 class D = std::ptrdiff_t,
			 class P = V*,
			 class R = V&>
				 struct forward_iterator_helper
				 : forward_iteratable<T, P
				   , aio::iterator<std::forward_iterator_tag, V, D, P, R
				   > > {};

	template <class T,
			 class V,
			 class D = std::ptrdiff_t,
			 class P = V*,
			 class R = V&>
				 struct bidirectional_iterator_helper
				 : bidirectional_iteratable<T, P
				   , aio::iterator<std::bidirectional_iterator_tag, V, D, P, R
				   > > {};

	template <class T,
			 class V, 
			 class D = std::ptrdiff_t,
			 class P = V*,
			 class R = V&>
				 struct random_access_iterator_helper
				 : random_access_iteratable<T, P, D, R
				   , aio::iterator<std::random_access_iterator_tag, V, D, P, R
				   > >
	{
		friend D requires_difference_operator(const T& x, const T& y) {
			return x - y;
		}
	}; // random_access_iterator_helper
#endif	//end disable iterator

}
#endif //end AIO_COMMON_OPERATIONS_H_
