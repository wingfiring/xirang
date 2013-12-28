#include "../precompile.h"
#include <xirang/type/xirang.h>
#include <xirang/type/object.h>
#include <xirang/type/typebinder.h>
#include <xirang/type/binder.h>
#include <xirang/type/array.h>
#include <xirang/type/nativetypeversion.h>

#include <vector>
#include <iostream>
#include <stdint.h>
using namespace xirang;
using namespace xirang::type;

BOOST_AUTO_TEST_SUITE(xirang_array_suites)

BOOST_AUTO_TEST_CASE(array_case)
{
    Xirang xi("object_case", xirang::memory::get_global_heap(), xirang::memory::get_global_ext_heap());

    SetupXirang(xi);
    Type int_type = xi.root().findType("int");

    Array arr0;
    BOOST_CHECK(!arr0.valid());

    Array arr1(xi.get_heap(), xi.get_ext_heap(), int_type);

    BOOST_REQUIRE(arr1.valid());
    BOOST_REQUIRE(arr1.type() == int_type);

    BOOST_CHECK(arr1.get_heap().equal_to(xi.get_heap()));
    BOOST_CHECK(arr1.get_heap().equal_to(xi.get_heap()));
    BOOST_CHECK(arr1.size() == 0);
    BOOST_CHECK(arr1.empty());
    BOOST_CHECK(arr1.begin() == arr1.end());
    arr1.reserve(10);
    BOOST_CHECK(arr1.capacity() >= 10);

    const int meaning = 42;
    ConstCommonObject obj_meaning(int_type, &meaning);
    arr1.push_back(obj_meaning);

    BOOST_CHECK(arr1.size() == 1);
    BOOST_CHECK(!arr1.empty());
    BOOST_CHECK(arr1.begin() + 1 == arr1.end());
    BOOST_CHECK(++arr1.begin() == arr1.end());
    BOOST_CHECK(arr1.begin() == --arr1.end());
    BOOST_CHECK(bind<int>(arr1.front()) == meaning);
    BOOST_CHECK(&bind<int>(arr1.front()) !=  &meaning); //array elements are value
    BOOST_CHECK(bind<int>(arr1.back()) == meaning);
    BOOST_CHECK(bind<int>(arr1[0]) == meaning);
    ++bind<int>(arr1[0]);
    BOOST_CHECK(bind<int>(arr1[0]) == meaning + 1);

    arr1.insert(arr1.begin(), obj_meaning);
    BOOST_CHECK(arr1.size() == 2);
    BOOST_CHECK(arr1.begin() + 2 == arr1.end());
    BOOST_CHECK(++arr1.begin() == --arr1.end());
    BOOST_CHECK(bind<int>(arr1.front()) == meaning);
    BOOST_CHECK(bind<int>(arr1.back()) == meaning + 1);
    BOOST_CHECK(bind<int>(arr1[1]) == meaning + 1);

    arr1.erase(arr1.begin());
    BOOST_CHECK(arr1.size() == 1);
    BOOST_CHECK(arr1.begin() + 1 == arr1.end());
    BOOST_CHECK(bind<int>(arr1.front()) == meaning + 1);
    BOOST_CHECK(bind<int>(arr1.back()) == meaning + 1);

    arr1.pop_back();
    BOOST_CHECK(arr1.size() == 0);
    BOOST_CHECK(arr1.begin() == arr1.end());

    arr1.resize(30);
    BOOST_CHECK(arr1.size() == 30);
    BOOST_CHECK(bind<int>(arr1[0]) == 0);
    BOOST_CHECK(bind<int>(arr1[1]) == 0);
    BOOST_CHECK(bind<int>(arr1[29]) == 0);

    arr1.resize(5);
    BOOST_CHECK(arr1.size() == 5);
    BOOST_CHECK(bind<int>(arr1[0]) == 0);
    BOOST_CHECK(bind<int>(arr1[1]) == 0);
    BOOST_CHECK(bind<int>(arr1[4]) == 0);

    arr1.clear();
    BOOST_REQUIRE(arr1.valid());
    BOOST_REQUIRE(arr1.type() == int_type);
    BOOST_CHECK(arr1.get_heap().equal_to(xi.get_heap()));
    BOOST_CHECK(arr1.get_heap().equal_to(xi.get_heap()));
    BOOST_CHECK(arr1.size() == 0);
    BOOST_CHECK(arr1.empty());


    Array arr2(xi.get_heap(), xi.get_ext_heap(), int_type);

    arr1.resize(10);
    arr2.resize(10);
    BOOST_CHECK(arr1 == arr2);

    arr1.pop_back();
    BOOST_CHECK(arr1 < arr2);

    arr1.push_back(obj_meaning);
    BOOST_CHECK(arr1 > arr2);

    arr2 = arr1;
    BOOST_CHECK(arr1 == arr2);

    Type string_type = xi.root().findType("string");
    Array arr3(xi.get_heap(), xi.get_ext_heap(), string_type);

    xirang::string str("42");
    CommonObject obj_str(string_type, &str);
    for (int i=0; i < 10; i++)
    {
        arr3.push_back(obj_str);
    }

    BOOST_CHECK(arr3.size() == 10);
    BOOST_CHECK(bind<string>(arr3[0]) == str);
    BOOST_CHECK(bind<string>(arr3[1]) == str);
    BOOST_CHECK(bind<string>(arr3[9]) == str);

}

BOOST_AUTO_TEST_SUITE_END()
