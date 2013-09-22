/*
$COMMON_HEAD_COMMENTS_CONTEXT$
*/

#include "precompile.h"
#include <aio/common/iterator.h>

#include <vector>
#include <map>

BOOST_AUTO_TEST_SUITE(iterator_suite)

using namespace aio;
BOOST_AUTO_TEST_CASE(test_output)
{
	typedef output_iterator< default_itr_traits<int>> itr_type;
	itr_type it;
	BOOST_CHECK(!it.valid());

	std::vector<int> vec;
	auto oitr = std::back_inserter(vec);
	itr_type itr(oitr);
	*itr = 1;
	BOOST_CHECK(vec.size() == 1 && vec[0] == 1);
	++itr;
	*itr = 2;
	BOOST_CHECK(vec.size() == 2 && vec[1] == 2);
}

template<typename Iter> void test_input(Iter& itr){
	Iter it;
	BOOST_CHECK(!it.valid());
	BOOST_CHECK(it == Iter());
	
	BOOST_REQUIRE(itr.valid());
	BOOST_CHECK(itr != it);

	BOOST_CHECK(*itr == 1);
	BOOST_CHECK(*++itr == 2);
	BOOST_CHECK(*(itr++) == 2);
	BOOST_CHECK(*itr == 3);
}
BOOST_AUTO_TEST_CASE( input_iterator_case)
{
	std::vector<int> vec{1,2,3,4};
	typedef input_iterator<default_itr_traits<int>> iterator;
	iterator itr(vec.begin());
	test_input(itr);
}


template<typename Iter> void test_forward(Iter& itr)
{
	Iter itr2 = itr;
	test_input(itr);
	test_input(itr2);
}
BOOST_AUTO_TEST_CASE(forward_iterator_case)
{
	std::vector<int> vec{1,2,3,4};
	typedef forward_iterator<default_itr_traits<int>> iterator;
	iterator itr(vec.begin());
	test_forward(itr);
}

template<typename Iter> void test_bidirection(Iter& itr)
{
	test_forward(itr);
	BOOST_CHECK(*--itr == 2);
	BOOST_CHECK(*itr-- == 2);
	BOOST_CHECK(*itr == 1);

}
BOOST_AUTO_TEST_CASE(bidirecton_iterator_case)
{
	std::vector<int> vec{1,2,3,4};
	typedef bidir_iterator<default_itr_traits<int>> iterator;
	iterator itr(vec.begin());
	test_bidirection(itr);
}
template<typename Iter> void test_random(Iter& itr){
	test_bidirection(itr);
	itr += 2;
	BOOST_CHECK(itr[0] == 3);
	BOOST_CHECK(itr[-1] == 2);

	auto itr2 = itr;
	BOOST_CHECK(itr2 - itr == 0);
	--itr;
	BOOST_CHECK(itr2 - itr == 1);
	BOOST_CHECK(itr2 - itr == std::distance(itr, itr2));
	BOOST_CHECK(itr - itr2 == -1);
	BOOST_CHECK(itr2 == itr + 1);
	BOOST_CHECK(itr2 == 1+ itr);


}
BOOST_AUTO_TEST_CASE(random_iterator_case){
	std::vector<int> vec{1,2,3,4};
	typedef random_iterator<default_itr_traits<int>> iterator;
	iterator itr(vec.begin());
	test_random(itr);
}

BOOST_AUTO_TEST_CASE( iterator_case)
{
       std::vector<int> vec;
       for (int i = 0; i < 10; ++i)
               vec.push_back(i);

	   typedef range<bidir_iterator<default_itr_traits<int>>> range_type;
	   typedef range_type::iterator iterator_type;

       range_type r (vec.begin(), vec.end());

       iterator_type p;
       BOOST_CHECK(!p.valid());
       p = r.begin();
       BOOST_REQUIRE(p.valid());

       BOOST_CHECK(*(p++) == 0);
       BOOST_CHECK(*++p == 2);
       BOOST_CHECK(*--p == 1);
       BOOST_CHECK(*(p--) == 1);
       
       iterator_type p2 = p++;
       BOOST_CHECK(*p2 == 0 && *p == 1);
       p2.swap(p);
       BOOST_CHECK(*p2 == 1 && *p == 0);

       BOOST_CHECK(!p2.equals(p));
       --p2;
       BOOST_CHECK(p2.equals(p));
}



struct filter_odd{
    bool operator()(int x) const{
        return x & 1;
    }
};
BOOST_AUTO_TEST_CASE(filter_iterator_case)
{
    std::vector<int> vec(11);

    //int i = 0;
    //std::generate(vec.begin(), vec.end(), [&i](){return ++i;});
    for(size_t j = 0 ;j < vec.size(); ++j) vec[j]=j+1;
    typedef filter_iterator<std::vector<int>::iterator, filter_odd> filter_iterator;

    filter_iterator first(vec.begin(), vec.end()), last(vec.end(), vec.end()); 

    int counter = 0;
    for (; first != last; ++first, ++counter){
        BOOST_CHECK((*first & 1));
    }
    BOOST_CHECK(counter == 6);

    filter_iterator first2(vec), last2(vec.end(), vec.end()); 

    counter = 0;
    for (--last2; first2 != last2; --last2, ++counter){
        BOOST_CHECK((*last2 & 1));
    }
    BOOST_CHECK(counter == 5);
    

}

template<typename T, typename U>
struct select_2nd
{
    typedef U value_type;
    typedef U* pointer;
    typedef U& reference;

    U& operator()(std::pair<const T, U>& var) const{
        return var.second;
    }
};

BOOST_AUTO_TEST_CASE(select_iterator_case)
{
    std::map<int, int> data;
    for (int i=0; i < 10; ++i)
        data.insert(std::make_pair(i, 2*i));

    typedef select_iterator<std::map<int, int>::iterator, select_2nd<int,int> > iterator;
    iterator first(data.begin()), last(data.end());

    std::map<int, int>::iterator itr = data.begin();
    for (iterator itr2 = first; itr2 != last; ++itr, ++itr2){
        BOOST_CHECK(itr->second == *itr2);
    }

    itr = data.end();
    --itr;
    iterator itr2 = last;
    --itr2;
    for (; itr2 != first; --itr, --itr2){
        BOOST_CHECK(itr->second == *itr2);
    }
}

BOOST_AUTO_TEST_CASE(joined_iterator_case)
{
    std::vector<int> vec(10);
    for (int i= 0; i < 5; ++i)
    {
        vec[i] = i + 5;
        vec[i+5] = i;
    }

    typedef joined_iterator<std::vector<int>::iterator> iterator;
    iterator first(vec.begin() + 5, vec.end(), vec.begin());

    std::vector<int>::iterator mid_end = vec.begin() + 5;
    
    iterator last(vec.begin() + 5, vec.end(), vec.begin(), false);
    iterator itr = first;
    for (int i =0; itr != last; ++itr, ++i)
    {
        BOOST_CHECK(*itr == i);
    }

    itr = last;
    --itr;
    for (int i =0; itr != first; --itr, ++i)
    {
        BOOST_CHECK(*itr + i == 9);
    }
}

BOOST_AUTO_TEST_CASE(merge_sort_iterator_case)
{
    std::vector<int> vec1(5), vec2(5);
    for (int i= 0; i < 5; ++i)
    {
        vec1[i] = 2*i;
        vec2[i] = 2*i + 1;
    }

    typedef merge_iterator<std::vector<int>::iterator> iterator;
    iterator first(vec1, vec2);
    
    iterator last(vec1, vec2);
    last.jump_to_end();


    iterator itr = first;
    for (int i = 0; itr != last; ++itr, ++i)
    {
        BOOST_CHECK(*itr == i);
    }

    itr = last;
    --itr;
    for (int i = 0; itr != first; --itr, ++i)
    {
        BOOST_CHECK(*itr + i == 9);
    }
}
BOOST_AUTO_TEST_SUITE_END()
