/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <xirang/string_algo/string.h>

BOOST_AUTO_TEST_SUITE(string_algo_suite)
using namespace aio;

BOOST_AUTO_TEST_CASE(replace_case)
{
    aio::string txt = literal("This is a test");
    aio::string result = replace(txt, 'i', 'I');
    BOOST_CHECK(result == literal("ThIs Is a test"));
}

BOOST_AUTO_TEST_CASE(toupper_case)
{
    BOOST_CHECK(toupper('i') == 'I');
    BOOST_CHECK(toupper('I') == 'I');
    BOOST_CHECK(toupper('$') == '$');
}

BOOST_AUTO_TEST_CASE(tolower_case)
{
    BOOST_CHECK(tolower('I') == 'i');
    BOOST_CHECK(tolower('i') == 'i');
    BOOST_CHECK(tolower('$') == '$');
}

BOOST_AUTO_TEST_CASE(tolower_copy_case)
{
    BOOST_CHECK(tolower_copy(string("THis Is -- A TesT.")) == literal("this is -- a test."));
}

BOOST_AUTO_TEST_CASE(toupper_copy_case)
{
    BOOST_CHECK(toupper_copy(string("THis Is -- A TesT.")) == literal("THIS IS -- A TEST."));
}

BOOST_AUTO_TEST_CASE(rfind_case)
{
    string txt ("012*45*789");
    BOOST_CHECK(rfind(txt.begin(), txt.end(), '*') == txt.begin() + 6);

    BOOST_CHECK(rfind(txt, '*') == txt.begin() + 6);

    const string txt2 = txt;
    BOOST_CHECK(rfind(txt2, '*') == txt2.begin() + 6);
}

BOOST_AUTO_TEST_CASE(find_case)
{
    string txt ("012*45*789");
    BOOST_CHECK(find(txt.begin(), txt.end(), '*') == txt.begin() + 3);

    BOOST_CHECK(find(txt, '*') == txt.begin() + 3);

    const string txt2 = txt;
    BOOST_CHECK(find(txt2, '*') == txt2.begin() + 3);
}
BOOST_AUTO_TEST_SUITE_END()
