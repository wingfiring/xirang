//XIRANG_LICENSE_PLACE_HOLDER
//
#ifndef AIO_COMMON_INTERFACE_H_
#define AIO_COMMON_INTERFACE_H_

#include <type_traits>
#include <cstring>	//for memcpy

namespace aio
{
	template<typename... Interfaces> struct interface_ref;
	template<typename... Interfaces> struct interface_auto;

	template<typename Interface> struct interface_holder
	{
		interface_holder() : vptr(0){}
		interface_holder(Interface& t) : vptr(*(void**)&t){}
		Interface& get_interface() const{ return *(Interface*)(&vptr);}
		private:
		void* vptr;
	};


	template<typename Derive, typename Interface>
	auto get_interface_map(Derive*, Interface*) -> typename Derive::coclass_type&;

	template<typename T> struct target_holder
	{
		T* target;
		explicit target_holder(T* t = 0) : target(t){}
	};

	namespace private_{

		template<typename Derive, typename Interface> struct get_base 
			: public std::remove_reference<decltype(get_interface_map((Derive*)0, (Interface*)0))>::type
		{ };

		template<typename CoClass, typename... Interfaces> struct compose_vptr0
			: public get_base<compose_vptr0<CoClass, Interfaces...>, Interfaces>...
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
				   !std::is_convertible<Interface1&, Interface2&>::value 
				&& !std::is_convertible<Interface2&, Interface1&>::value;
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
				typedef typename find_convertible_helper<std::is_convertible<U&, T&>::value, T, U, Args...>::type type; 
			};

		template<typename T> struct find_convertible<T> { };

		template<typename... Args> struct is_interface_ref{
				static const bool value = false;
			};
		template<typename... Args>
			struct is_interface_ref<interface_auto<Args...> >{
				static const bool value = true;
			};
		template<typename... Args>
			struct is_interface_ref<interface_ref<Args...> >{
				static const bool value = true;
			};
	}

	template<typename... Args>
		struct interface_ref : public target_holder<void>, public interface_holder<Args>...
	{
		static_assert(private_::valid_interface<Args...>::value, "interfaces must not be convertible between each other");

		template<typename U> interface_ref(U&& obj, 
				typename std::enable_if<!private_::is_interface_ref<U>::value, void*>::type = 0 )
		{
			private_::compose_vptr<typename std::remove_reference<U>::type, Args...> c(std::forward<U>(obj));
			static_assert(sizeof(c) == sizeof(*this), "size mismatch");
			std::memcpy(this, &c, sizeof(c));
		}

		template<typename... OArgs>
			interface_ref(interface_ref<OArgs...>& rhs)  
			: target_holder<void>(rhs.get_target()), interface_holder<Args>(rhs.get<Args>())...
		{
		}
		template<typename... OArgs>
			interface_ref(const interface_ref<OArgs...>& rhs) 
			: target_holder<void>(rhs.get_target()), interface_holder<Args>(rhs.get<Args>())...
		{
		}

		template<typename T> T& get() const{
			typedef typename private_::find_convertible<T, Args...>::type type;
			const interface_holder<type>* this_ = this;
			return this_->get_interface();
		}
		template<typename... OArgs>
		interface_ref& operator=(const interface_ref<OArgs...>& rhs){
			return *this = interface_ref(rhs);
		}
		void* get_target() const { return target;}
	};

	template<typename... Args>
		struct interface_auto : public interface_ref<Args...>
	{
		template<typename U>
			interface_auto(std::unique_ptr<U>&& ptr) : interface_ref<Args...>(*ptr), target(std::move(ptr))
		{
		}

		template<typename... OArgs>
			interface_auto(interface_auto<OArgs...>&& rhs)  : interface_ref<Args...>(rhs), target(std::move(rhs.target))
		{
		}
		private:

		std::unique_ptr<void> target;
	};

}
#endif //end AIO_COMMON_INTERFACE_H_

