
#include "precompile.h"
#include <aio/common/interface.h>

BOOST_AUTO_TEST_SUITE(interface_suite)
	using namespace aio;

struct Bar{ virtual void bar() = 0; };

template<typename Derive>
struct BarCo : public Bar
{
	virtual void bar() {
		static_cast<const Derive*>(this)->get_target()->bar();
	};
};

template<typename Derive>
BarCo<Derive> get_interface_map(Derive*, Bar*);


struct Foo{
	virtual void foo() = 0;
};

template<typename Derive>
struct FooCo : public Foo
{
	virtual void foo() {
		static_cast<const Derive*>(this)->get_target()->foo();
	};
};

template<typename Derive>
FooCo<Derive> get_interface_map(Derive*, Foo*);

struct X{
	X(): m(0), n(0){}

	void foo() const { n += 1;}
	void bar() const { n -= 1;}
	void foo() { m += 1;}
	void bar() { m -= 1;}

	int m;
	mutable int n;
};

void interface_test()
{
	X a;
	interface_ref<Foo, Bar> ref(a);
	
	ref.get<Foo>().foo();
	BOOST_CHECK(a.m == 1);

	ref.get<Bar>().bar();
	BOOST_CHECK(a.m == 0);

	const X a1;
	interface_ref<Foo, Bar> ref1(a1);
	ref1.get<Foo>().foo();
	BOOST_CHECK(a1.n == 1);
	ref1.get<Bar>().bar();
	BOOST_CHECK(a1.n == 0);

	interface_ref<Foo, Bar> ref2(ref);
	ref2.get<Foo>().foo();
	BOOST_CHECK(a.m == 1);
	ref2.get<Bar>().bar();
	BOOST_CHECK(a.m == 0);

	interface_ref<Foo> ref3(ref);
	ref3.get<Foo>().foo();
	BOOST_CHECK(a.m == 1);

	ref3 = ref2;
	ref3.get<Foo>().foo();
	BOOST_CHECK(a.m == 2);


	X* px = new X;
	interface_auto<Foo, Bar> af((std::unique_ptr<X>(px)));
	af.get<Foo>().foo();
	BOOST_CHECK(px->m == 1);

}
BOOST_AUTO_TEST_CASE(interface_case)
{
	interface_test();
}
BOOST_AUTO_TEST_SUITE_END() 
