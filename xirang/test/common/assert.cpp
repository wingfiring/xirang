
/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/assert.h>
#include <aio/common/context_except.h>

//BOOST
#include <boost/mpl/list.hpp>

//STL
#include <string>

BOOST_AUTO_TEST_SUITE(assert_suite)

using namespace aio;

BOOST_AUTO_TEST_CASE(test_contract_tag)
{
	exception_reporter except_engin;
	contract_handler_saver saver(&except_engin);
	BOOST_REQUIRE(contract_handler::get_handle() == &except_engin); //test contract_handler_saver

	BOOST_CHECK_THROW(contract_handler::process(pre, 0, 0, 0, 0, 0),  pre_exception);
	BOOST_CHECK_THROW(contract_handler::process(post, 0, 0, 0, 0, 0),  post_exception);
	BOOST_CHECK_THROW(contract_handler::process(invariant, 0, 0, 0, 0, 0),  invariant_exception);

	//pre_ post_ and invariant_exception are derived from contract_exception.
	BOOST_CHECK_THROW(contract_handler::process(invariant, 0, 0, 0, 0, 0),  contract_exception);
	
}

BOOST_AUTO_TEST_SUITE_END()
