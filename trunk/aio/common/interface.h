//XIRANG_LICENSE_PLACE_HOLDER
//
#ifndef AIO_COMMON_INTERFACE_H_
#define AIO_COMMON_INTERFACE_H_

#include <type_traits>
#include <cstring>	//for memcpy
#include <aio/common/backward/unique_ptr.h>

namespace interface_suite{
	struct Z;
	struct Bar;
	struct Foo;
}
namespace aio
{
	template<typename... Interfaces> struct iref;
	template<typename... Interfaces> struct iauto;
	template<typename Interface> struct opt{ };
	template<typename Interface> struct noif{ };

	template<typename Interface> struct interface_holder
	{
		interface_holder() : vptr(0){}
		interface_holder(Interface& t) : vptr(*(void**)&t){}
		Interface& get_interface() const{ return *(Interface*)(&vptr);}
		private:
		void* vptr;
	};
	template<typename Interface> struct interface_holder<opt<Interface>>
	{
		interface_holder() : vptr(0){}
		interface_holder(Interface& t) : vptr(*(void**)&t){}
		interface_holder(Interface* t) : vptr(t?*(void**)t : 0){}
		Interface* get_interface() const{ return (Interface*)(&vptr);}
		private:
		void* vptr;
	};


	template<typename Derive, typename Interface, typename CoClass>
	auto get_interface_map(Derive*, Interface*, CoClass*) -> typename Derive::coclass_type&;

	template<typename T> struct target_holder
	{
		T* target;
		explicit target_holder(T* t = 0) : target(t){}
	};

	namespace private_{
		struct interface_not_exist{};
		struct null_interface{
			null_interface(){
				*(void**)this = 0;
			}
			virtual ~null_interface(){}
			null_interface(const null_interface&) = delete;
		};

		template <typename Interface> struct get_interface_type{
			typedef Interface type;
			typedef Interface& return_type;
		};
		template <typename Interface> struct get_interface_type <noif<Interface>>{
			typedef Interface type;
			typedef Interface* return_type;
		};
		template <typename Interface> struct get_interface_type <opt<Interface>>{
			typedef Interface type;
			typedef Interface* return_type;
		};

		template<typename Interface> struct is_optional{
			static const bool value = false;
		};
		template<typename Interface> struct is_optional<opt<Interface>>{
			static const bool value = true;
		};
		template<typename Interface> struct is_noi{
			static const bool value = false;
		};
		template<typename Interface> struct is_noi<noif<Interface>>{
			static const bool value = true;
		};

		template<typename Derive, typename Interface, typename CoClass> struct get_base
			: public std::remove_reference<decltype(get_interface_map((Derive*)0, (Interface*)0, (CoClass*)0))>::type
		{
		};
		template<typename Derive, typename Interface, typename CoClass> struct get_base <Derive, opt<Interface>, CoClass>
			: public null_interface
		{ };
		template<typename Derive, typename Interface, typename CoClass> struct get_base <Derive, noif<Interface>, CoClass>
			: public null_interface
		{ };


		template<typename CoClass, typename... Interfaces> struct compose_vptr0
			: public get_base<compose_vptr0<CoClass, Interfaces...>, Interfaces, CoClass>...
		{
			typedef typename std::remove_const<CoClass>::type coclass_type;
			CoClass* get_target() const{ return ((CoClass**)this)[-1];}
		};
		template<typename CoClass, typename... Interfaces> struct compose_vptr 
		{
			target_holder<CoClass> m0;
			compose_vptr0<CoClass, Interfaces...> m2; 
			template<typename OtherCoClass> compose_vptr(OtherCoClass& u) : m0(&u){
			}
		};


		template<typename... Interfaces> struct valid_interface;

		template<typename Interface> struct valid_interface<Interface> {
			static const bool value = true;
		};

		template<typename Interface1, typename Interface2> struct valid_interface<Interface1, Interface2>
		{
			static const bool value = 
				!std::is_convertible<typename get_interface_type<Interface1>::type&
					, typename get_interface_type<Interface2>::type&>::value 
				&& !std::is_convertible<typename get_interface_type<Interface2>::type&
					, typename get_interface_type<Interface1>::type&>::value;
		};

		template<typename Interface1, typename Interface2, typename ... Interfaces>
			struct valid_interface<Interface1, Interface2, Interfaces...>
			{
				static const bool value = valid_interface<Interface1, Interface2>::value
					&& valid_interface<Interface1, Interfaces...>::value
					&& valid_interface<Interface2, Interfaces...>::value;
			};

		template<typename T, typename... Interfaces> struct find_convertible;

		template<bool found, typename T, typename U, typename... Args>
			struct find_convertible_helper
			{
				typedef U type;
			};

		template<typename T, typename U, typename... Args>
			struct find_convertible_helper<false, T, U, Args...>
			{
				typedef typename find_convertible<T, Args...>::type type;
			};

		template<typename T, typename U, typename... Args>
			struct find_convertible<T, U, Args...>
			{
				typedef typename find_convertible_helper<std::is_convertible<typename get_interface_type<U>::type&
					, typename get_interface_type<T>::type&>::value, T, U, Args...>::type type; 
			};

		template<typename T> struct find_convertible<T> { typedef interface_not_exist type;};

		template<typename T> struct is_iref{
				static const bool value = std::is_base_of<target_holder<void>, typename std::remove_reference<T>::type>::value;
			};

		template<typename T, typename Interface> struct get_return_type_imp{ typedef Interface& type;};
		template<typename T, typename Interface> struct get_return_type_imp<T, opt<Interface>>{ typedef Interface* type;};
		template<typename T, typename Interface> struct get_return_type_imp<T, noif<Interface>>{ typedef Interface* type;};
		template<typename T> struct get_return_type_imp<T, interface_not_exist>{ typedef T* type;};

		template<typename T, typename... Args>
			struct get_return_type : public get_return_type_imp<T, typename private_::find_convertible<T, Args...>::type>
			{
			};
	}

	template<typename... Args>
		struct iref : public target_holder<void>, public interface_holder<Args>...
	{
		static_assert(private_::valid_interface<Args...>::value, "interfaces must not be convertible between each other");

		iref(){}

		template<typename U, typename T = typename std::enable_if<!private_::is_iref<U>::value>::type> 
			iref(U&& obj, T* = 0)
		{
			private_::compose_vptr<typename std::remove_reference<U>::type, Args...> c(std::forward<U>(obj));
			static_assert(sizeof(c) == sizeof(*this), "size mismatch");
			std::memcpy(this, &c, sizeof(c));
		}

		template<typename... OArgs>
			iref(iref<OArgs...>& rhs) 
			: target_holder<void>(rhs.get_target()), interface_holder<Args>(
					rhs.get<typename private_::get_interface_type<Args>::type>())...
		{
		}

		template<typename... OArgs>
			iref(const iref<OArgs...>& rhs) 
			: target_holder<void>(rhs.get_target()), interface_holder<Args>(
					rhs.get<typename private_::get_interface_type<Args>::type>())...
		{
		}

		template<typename T> 
		typename private_::get_return_type<T, Args...>::type
		get() const {
			AIO_PRE_CONDITION(valid());

			typedef typename private_::get_return_type<T, Args...>::type return_type;
			typedef typename private_::find_convertible<T, Args...>::type type;
			return get_helper_<return_type>((type*)0);
		}
		template<typename... OArgs>
		iref& operator=(const iref<OArgs...>& rhs){
			return *this = iref(rhs);
		}
		void* get_target() const { return target;}
		explicit operator bool() const { return valid();}
		bool valid() const{ return target != 0;}

		private:
		template<typename Ret, typename Interface>
			Ret get_helper_(Interface*) const{
			const interface_holder<Interface>* this_ = this;
			return this_->get_interface();
			}
		template<typename Ret, typename Interface>
			Ret get_helper_(noif<Interface>*) const{
				return Ret();
			}
		template<typename Ret, typename Interface>
			Ret get_helper_(opt<Interface>*) const{
				const interface_holder<opt<Interface>>* this_ = this;
				return this_->get_interface();
			}
		template<typename Ret>
			Ret get_helper_(private_::interface_not_exist*) const{
				return Ret();
			}

	};

	template<typename... Args>
		struct iauto : public iref<Args...>
	{
		typedef unique_ptr<void> owner_t;
		iauto(){}

		template<typename U> iauto(unique_ptr<U>&& ptr) 
			: iref<Args...>(*ptr), target_ptr(std::move(ptr))
		{
		}

		template<typename... OArgs> iauto(iauto<OArgs...>&& rhs) 
			: iref<Args...>(rhs), target_ptr(std::move(rhs.target_ptr))
		{
		}
		template<typename U, typename = typename std::enable_if<std::is_rvalue_reference<U&&>::value>::type >
			iauto(U&& rhs)  
			: iref<Args...>(new U(std::forward<U>(rhs)))
			, target_ptr((U*)this->get_target())
		{
		}

		unique_ptr<void> target_ptr;
	};

}
#endif //end AIO_COMMON_INTERFACE_H_

