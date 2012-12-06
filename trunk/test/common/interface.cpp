
#include "precompile.h"
#include <aio/common/interface.h>

BOOST_AUTO_TEST_SUITE(interface_suite)
	using namespace aio;

struct Bar{ virtual void bar() = 0; };

template<typename RealType>
struct BarCo : public Bar
{
	virtual void bar() {
		static_cast<RealType*>(*((void**)this + 1))->bar();
	};
};

template<typename CoClass>
BarCo<CoClass> get_interface_map(Bar*, CoClass*);


struct Foo{
	virtual void foo() = 0;
};

template<typename RealType>
struct FooCo : public Foo
{
	virtual void foo() {
		static_cast<RealType*>(*((void**)this + 1))->foo();
	};
};

template<typename CoClass>
FooCo<CoClass> get_interface_map(Foo*, CoClass*);

struct X{
	X(): m(0), n(0){}

	void foo() const { n += 1;}
	void bar() const { n -= 1;}
	void foo() { m += 1;}
	void bar() { m -= 1;}

	int m;
	mutable int n;
};

struct Z{
	void fun(){ std::cout << "Z::fun\n";}
};

struct Z_to_Foo : public Foo{
	virtual void foo() {
		static_cast<Z*>(*((void**)this + 1))->fun();
	}
};
Z_to_Foo get_interface_map(Foo*, Z*);

template<typename RealType>
struct Z_to_Bar : public Bar
{
	virtual void bar() {
		static_cast<RealType*>(*((void**)this + 1))->fun();
	};
};

//template<typename RealType>
//Z_to_Bar<RealType> get_interface_map(RealType*, Bar*, Z*);

void interface_test()
{
	X a;
	iref<Foo, Bar> ref(a);
	
	ref.get<Foo>().foo();
	BOOST_CHECK(a.m == 1);

	ref.get<Bar>().bar();
	BOOST_CHECK(a.m == 0);

	const X a1;
	iref<Foo, Bar> ref1(a1);
	ref1.get<Foo>().foo();
	BOOST_CHECK(a1.n == 1);
	ref1.get<Bar>().bar();
	BOOST_CHECK(a1.n == 0);

	iref<Foo, Bar> ref2(ref);
	ref2.get<Foo>().foo();
	BOOST_CHECK(a.m == 1);
	ref2.get<Bar>().bar();
	BOOST_CHECK(a.m == 0);

	iref<Foo> ref3(ref);
	ref3.get<Foo>().foo();
	BOOST_CHECK(a.m == 1);

	ref3 = ref2;
	ref3.get<Foo>().foo();
	BOOST_CHECK(a.m == 2);


	X* px = new X;
	iauto<Foo, Bar> af((unique_ptr<X>(px)));
	af.get<Foo>().foo();
	BOOST_CHECK(px->m == 1);

	Z z;
	static_assert(private_::is_iref<iauto<Foo, Bar>>::value, "oops...");

	iref<Foo, noif<Bar>> refz(z);
	iref<Foo, opt<Bar>> refb(*px);
	iref<Foo, opt<Bar>> refc(iref<Foo, noif<Bar>>(z));
	iref<Foo, opt<Bar>> refd(refz);
	iref<Foo, opt<Bar>> refe(af);

	BOOST_CHECK(refz.get<Bar>() == 0);
	BOOST_CHECK(refb.get<Bar>() != 0);

}
BOOST_AUTO_TEST_CASE(interface_case)
{
	interface_test();
}
BOOST_AUTO_TEST_SUITE_END() 
