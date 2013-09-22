/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang/ref_ptr.h>
#include <xirang/assert.h>
#include <xirang/backward/atomic.h>

//BOOST
#include <boost/mpl/list.hpp>

BOOST_AUTO_TEST_SUITE(atomic_suite)
using namespace aio;

typedef boost::mpl::list<int, long> test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(atomic_case, T, test_types)
{
	typedef atomic::atomic_t<T> atomic_type;

	T t(0), t1(1), t2(2), t3(3), t4(4), t5(5);
	
	atomic_type a = {t};

	BOOST_CHECK(sync_get(a) == t);
	sync_set(a, t1);
	BOOST_CHECK(sync_get(a) == t1);
	BOOST_CHECK(!sync_cas(a, t, t2));
	BOOST_CHECK(sync_cas(a, t1, t2));

	BOOST_CHECK(sync_add_fetch(a, t1) == t3 );
	BOOST_CHECK(sync_get(a) == t3 );

	BOOST_CHECK(sync_sub_fetch(a, t1) == t2 );
	BOOST_CHECK(sync_get(a) == t2 );

	BOOST_CHECK(sync_or_fetch(a, t1) == t3 );
	BOOST_CHECK(sync_get(a) == t3 );

	BOOST_CHECK(sync_and_fetch(a, t2) == t2 );
	BOOST_CHECK(sync_get(a) == t2 );

	BOOST_CHECK(sync_xor_fetch(a, t1) == t3 );
	BOOST_CHECK(sync_get(a) == t3 );

	//BOOST_CHECK(sync_nand_fetch(a, t4) == t4 );
	//BOOST_CHECK(sync_get(a) == t4 );
	sync_set(a, t4);


	BOOST_CHECK(sync_fetch_add(a, t1) == t4 );
	BOOST_CHECK(sync_get(a) == t5 );

	BOOST_CHECK(sync_fetch_sub(a, t1) == t5 );
	BOOST_CHECK(sync_get(a) == t4 );

	BOOST_CHECK(sync_fetch_or(a, t1) == t4 );
	BOOST_CHECK(sync_get(a) == t5 );

	BOOST_CHECK(sync_fetch_and(a, t1) == t5 );
	BOOST_CHECK(sync_get(a) == t1 );

	BOOST_CHECK(sync_fetch_xor(a, t2) == t1 );
	BOOST_CHECK(sync_get(a) == t3 );

	//BOOST_CHECK(sync_fetch_nand(a, t1) == t3 );
	//BOOST_CHECK(sync_get(a) == 0 );

}

BOOST_AUTO_TEST_CASE(atomic_bool_case)
{
	typedef atomic::atomic_t<bool> atomic_type;

	atomic_type a = {false};

	BOOST_CHECK(sync_cas(a, false, true));
	BOOST_CHECK(sync_get(a));

	atomic::sync_fence();

	atomic::atomic_flag flag ={0};

	BOOST_CHECK(sync_acquire(flag));
	BOOST_CHECK(flag.value == 1);
	sync_release(flag);
	BOOST_CHECK(flag.value == 0);

}

BOOST_AUTO_TEST_SUITE_END()
