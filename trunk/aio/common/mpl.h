//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_MPL_H
#define AIO_COMMON_MPL_H

#include <type_traits>
namespace aio{ namespace mpl{

template<typename T, size_t N>
struct array { T data[N]; };

template<typename... Args> struct seq{ 
	typedef seq<Args...> type;
};

template<typename Seq> struct size;
template<size_t i, typename Seq> struct at;
template<typename Seq> struct first{ typedef typename at<0, Seq>::type type;};
template<typename Seq> struct last{ typedef typename at<size<Seq>::value - 1, Seq>::type type;};
template<typename Seq> struct empty{ static constexpr bool value = (size<Seq>::value == 0);};
template<typename Seq1, typename Seq2> struct concat;
template<typename Seq, typename T> struct push_back;
template<typename Seq, typename T> struct push_front;
template<typename Seq, size_t c = 1> struct pop_back;
template<typename Seq, size_t c = 1> struct pop_front;
template<typename Seq, template<typename, typename> class Comp> struct partition;
template<typename Seq, template<typename, typename> class Comp> struct sort;
template<typename Seq1, typename Seq2, template<typename, typename> class Comp> struct equal{ static const bool value = false;};
template<typename Seq, size_t p, typename T> struct insert;
template<typename Seq, size_t p> struct erase;
template<typename Seq, size_t beg, size_t end> struct sub;


template<typename T, size_t N> struct size<array<T, N>> { 
	static constexpr size_t value = N;
};
template<typename... Args> struct size<seq<Args...>> { 
	static constexpr size_t value = sizeof...(Args);
};

template<typename T, typename... Args> struct at<0, seq<T, Args...>>{
	typedef T type;
};
template<size_t i, typename T, typename ... Args> struct at<i, seq<T, Args...>>{
	typedef typename at<i-1, seq<Args...>>::type type;
};

template<typename... Args1, typename... Args2> struct concat<seq<Args1...>, seq<Args2...>> {
	typedef seq<Args1..., Args2...> type;
};

template<typename ... Args, typename T> struct push_back <seq<Args...>, T>{
	typedef seq<Args..., T> type;
};
template<typename ... Args, typename T> struct push_front <seq<Args...>, T>{
	typedef seq<T, Args...> type;
};

template<typename ... Args> struct pop_back <seq<Args...>,  0>{
	typedef seq<Args...> type;
};
template<typename ... Args, typename T, size_t c> struct pop_back <seq<Args..., T>,  c>{
	typedef typename pop_back<seq<Args...>, c - 1>::type type;
};

template<typename ... Args> struct pop_front <seq<Args...>, 0>{
	typedef seq<Args...> type;
};
template<typename T, typename ... Args, size_t c> struct pop_front <seq<T, Args...>, c>{
	typedef typename pop_front<seq<Args...>, c - 1>::type type;
};

template<template<typename, typename> class Comp> 
struct partition<seq<>, Comp> {
	typedef seq<> type;
	typedef seq<> mid;
	typedef seq<> rest;
};
template<typename T, template<typename, typename> class Comp> 
struct partition<seq<T>, Comp> {
	typedef seq<> type;
	typedef seq<T> mid;
	typedef seq<> rest;
};
template<typename T, typename U, template<typename, typename> class Comp> 
struct partition<seq<T, U>, Comp> {
	typedef typename std::conditional<Comp<U,T>::value, seq<U>, seq<>>::type type;
	typedef seq<T> mid;
	typedef typename std::conditional<Comp<U,T>::value, seq<>, seq<U>>::type rest;
};
template<typename T, typename U, typename ... Args, template<typename, typename> class Comp> 
struct partition<seq<T, U, Args...>, Comp>
{
	typedef typename concat<typename partition<seq<T, U>, Comp>::type, typename  partition<seq<T, Args...>, Comp>::type>::type type;
	typedef seq<T> mid;
	typedef typename concat<typename partition<seq<T, U>, Comp>::rest, typename  partition<seq<T, Args...>, Comp>::rest>::type rest;
};

template<template<typename, typename> class Comp> 
struct sort<seq<>, Comp> {
	typedef seq<> type;
};
template<typename T, template<typename, typename> class Comp> 
struct sort<seq<T>, Comp> {
	typedef seq<T> type;
};
template<typename T, typename ...Args, template<typename, typename> class Comp> 
struct sort<seq<T, Args...>, Comp>
{
	typedef typename partition<seq<T, Args...>, Comp>::type left;
	typedef typename partition<seq<T, Args...>, Comp>::mid mid;
	typedef typename partition<seq<T, Args...>, Comp>::rest right;
	typedef typename concat<typename concat<typename sort<left, Comp>::type, mid>::type, typename sort<right, Comp>::type>::type type;
};


template<template<typename, typename> class Comp> 
struct equal<seq<>, seq<>, Comp>{
	static const bool value = true;
};

template<typename T1, typename... Args1, typename T2, typename... Args2, template<typename, typename> class Comp> 
struct equal<seq<T1, Args1...>, seq<T2, Args2...>, Comp>{
	static const bool value = (sizeof...(Args1) == sizeof...(Args2))
		&& Comp<T1, T2>::value
		&& equal<seq<Args1...>, seq<Args2...>, Comp>::value;
};

}}


#endif //end AIO_COMMON_MPL_H


