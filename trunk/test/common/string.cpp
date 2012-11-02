/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/string.h>
#include <aio/common/assert.h>

//BOOST
#include <boost/mpl/list.hpp>

BOOST_AUTO_TEST_SUITE(string_suite)
using namespace aio;

typedef boost::mpl::list<char, wchar_t> test_types;

template<typename T>
struct test_data;

template<>
struct test_data<char>
{
	typedef const char* result;
	static result abcd() { return "abcd";}
	static result qwerty(){ return "qwerty";}
	static result abcd_qwerty(){ return "abcdqwerty";}
	
};
template<>
struct test_data<wchar_t>
{
	typedef const wchar_t* result;
	static result abcd() { return L"abcd";}
	static result qwerty(){ return L"qwerty";}
	static result abcd_qwerty(){ return L"abcdqwerty";}	
};

template<typename T>
struct test_data2;
template<>
struct test_data2<const char>
{
	typedef basic_range_string<const char> result;
	static result abcd() { return result("abcd");}
	static result qwerty(){ return result("qwerty");}
	static result abcd_qwerty(){ return result("abcdqwerty");}
	
};
template<>
struct test_data2<const wchar_t>
{
	typedef basic_range_string<const wchar_t> result;
	static result abcd() { return result(L"abcd");}
	static result qwerty(){ return result(L"qwerty");}
	static result abcd_qwerty(){ return result(L"abcdqwerty");}	
};

BOOST_AUTO_TEST_CASE_TEMPLATE(basic_range_string_case, T, test_types)
{
	typedef basic_range_string<const T> cstring;
	typedef test_data2<const T> data;

	cstring empty;

	BOOST_CHECK(empty.size() == 0);
	BOOST_CHECK(empty.empty());
	BOOST_CHECK(empty.begin() == empty.end());

	cstring abcd = data::abcd();
	BOOST_CHECK(abcd.size() == 4);
	BOOST_CHECK(!abcd.empty());
	BOOST_CHECK(abcd.data() != 0);
	BOOST_CHECK(abcd.begin() + 4 == abcd.end());

	cstring qwerty = data::qwerty();

	BOOST_CHECK(abcd == abcd);
	BOOST_CHECK(abcd != qwerty);
	BOOST_CHECK(abcd < qwerty);
	BOOST_CHECK(qwerty > abcd);
	BOOST_CHECK(abcd < data::qwerty());
	BOOST_CHECK(qwerty == qwerty);

	swap(abcd, qwerty);

	BOOST_CHECK(abcd == data::qwerty());

	cstring range_ctor(make_range(data::abcd_qwerty().begin(), data::abcd_qwerty().begin() + 4));
	BOOST_CHECK(range_ctor == data::abcd());

	cstring abcd2 = cstring ( range<typename cstring::iterator>(data::abcd()));
	cstring abcd3 = cstring ( to_range(data::abcd()));
}
BOOST_AUTO_TEST_CASE_TEMPLATE(basic_string_case, T, test_types)
{
	typedef basic_string<T> cstring;
	typedef test_data<T> data;

	cstring empty;

	BOOST_CHECK(empty.size() == 0);
	BOOST_CHECK(empty.empty());
	BOOST_CHECK(empty.c_str() != 0);
	BOOST_CHECK(empty.begin() == empty.end());

	cstring abcd = data::abcd();
	BOOST_CHECK(abcd.size() == 4);
	BOOST_CHECK(!abcd.empty());
	BOOST_CHECK(abcd.c_str() != 0);
	BOOST_CHECK(abcd.begin() + 4 == abcd.end());

	cstring qwerty = data::qwerty();

	BOOST_CHECK(abcd != qwerty);
	BOOST_CHECK(!(abcd < abcd));
	BOOST_CHECK(abcd < qwerty);
	BOOST_CHECK(abcd < data::qwerty());
	BOOST_CHECK(qwerty == qwerty);
	BOOST_CHECK((abcd + qwerty) == data::abcd_qwerty());
	BOOST_CHECK((abcd + data::qwerty()) == data::abcd_qwerty());
	BOOST_CHECK((data::abcd() + qwerty) == data::abcd_qwerty());

	swap(abcd, qwerty);

	BOOST_CHECK(abcd == data::qwerty());

	cstring range_ctor(make_range(data::abcd_qwerty(), data::abcd_qwerty() + 4));
	BOOST_CHECK(range_ctor == qwerty);
	
}

BOOST_AUTO_TEST_CASE_TEMPLATE(basic_string_builder_case, T, test_types)
{
	typedef basic_string_builder<T> cstring;
	typedef test_data2<const T> data;

	cstring empty;

	BOOST_CHECK(empty.size() == 0);
	BOOST_CHECK(empty.empty());
	BOOST_CHECK(empty.c_str() != 0);
	BOOST_CHECK(empty.begin() == empty.end());

	cstring abcd = data::abcd();
	BOOST_CHECK(abcd.size() == 4);
	BOOST_CHECK(!abcd.empty());
	BOOST_CHECK(abcd.c_str() != 0);
	BOOST_CHECK(abcd.begin() + 4 == abcd.end());

	cstring qwerty = data::qwerty();

	BOOST_CHECK(abcd != qwerty);
	BOOST_CHECK(abcd < qwerty);
	BOOST_CHECK(abcd < data::qwerty());
	BOOST_CHECK(qwerty == qwerty);
	cstring abcd_qwerty = basic_string<T>(abcd);
	abcd_qwerty += qwerty;
	BOOST_CHECK(abcd_qwerty == data::abcd_qwerty());

	swap(abcd, qwerty);

	BOOST_CHECK(abcd == data::qwerty());

	abcd = basic_string<T>(data::abcd());
	abcd = abcd.c_str();
	BOOST_CHECK(abcd == data::abcd());
	
	abcd.resize(6);
	BOOST_CHECK(abcd.size() == 6);
	abcd[4] = T();
	BOOST_CHECK(abcd[5] == T());

	abcd.resize(4);
	abcd.reserve(100);
	BOOST_CHECK(abcd.capacity() >= 100);
	BOOST_CHECK(abcd == data::abcd());

	abcd.insert(abcd.end(), to_range(data::qwerty()));
	BOOST_CHECK(abcd == data::abcd_qwerty());

	abcd.resize(4);
	BOOST_CHECK(abcd == data::abcd());
	abcd.append(data::qwerty().data());
	BOOST_CHECK(abcd == data::abcd_qwerty());
	
	abcd.erase(make_range(abcd.begin(), abcd.begin() + 4));
	BOOST_CHECK(abcd == data::qwerty());

	abcd = data::qwerty();
	abcd += data::qwerty();
	abcd.replace(make_range(abcd.begin(), abcd.begin() + data::qwerty().size()), to_range(data::abcd()));
	BOOST_CHECK(abcd == data::abcd_qwerty());

	abcd.substr(make_range(abcd.begin(), abcd.begin() + 4));
	BOOST_CHECK(abcd == data::abcd());

}

BOOST_AUTO_TEST_SUITE_END()
