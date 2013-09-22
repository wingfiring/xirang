/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang/memory.h>
#include <xirang/assert.h>

//BOOST
#include <boost/mpl/list.hpp>

//STL
#include <string>

BOOST_AUTO_TEST_SUITE(memory_suite)

using namespace aio;
/* TODO: switch to heap interface
BOOST_AUTO_TEST_CASE(memory_handler_case)
{
	memory_handler& default_handler = memory_handler::get();
	BOOST_CHECK(default_handler.equal_to(memory_handler()));

	memory_handler handler2;
	BOOST_CHECK(handler2.equal_to(default_handler));
	{
		memory_handler_saver saver;
		memory_handler::set(handler2);
		BOOST_CHECK(&default_handler != &memory_handler::get());
		BOOST_CHECK(&handler2 == &memory_handler::get());
	}

	BOOST_CHECK(&default_handler == &memory_handler::get());

	void *p = default_handler.malloc(100, 0);
	BOOST_CHECK(p != 0);
	default_handler.free(p, 100);
}

*/

typedef boost::mpl::list<int,long, std::string> test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(abi_allocator_case, T, test_types)
{
	typedef abi_allocator<T> allocator;
	typedef abi_allocator<void> void_allocator;

	allocator a1;
	void_allocator av;

	BOOST_CHECK(a1 == av);
	BOOST_CHECK(a1.get_heap().equal_to(memory::get_global_heap()));

	uninitialized_allocator_ptr<T> p(a1);
	p.allocate();
	a1.construct(p.get(), T());

	BOOST_CHECK(*p.get() == T());

	a1.destroy(p.get());
	p.deallocate();

	abi_allocator<int> alloc = a1;
	alloc = av;

	abi_allocator<int> alloc2 = av;
	alloc = a1;

	unuse(alloc2);

}
BOOST_AUTO_TEST_SUITE_END()
