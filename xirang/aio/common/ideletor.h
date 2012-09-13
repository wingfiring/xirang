#ifndef AIO_COMMON_IDELETOR_H
#define AIO_COMMON_IDELETOR_H

#include <aio/common/config.h>

#ifdef MSVC_COMPILER_
#include <tuple>    //just for macros of _CLASS_ARG0_DEF_MAX etc
#endif
namespace aio
{
	struct AIO_INTERFACE ideletor
	{
		virtual void destroy() = 0;
		protected:
		virtual ~ideletor(){}
	};
#if 0
	template<typename Deletor, typename ... Base>
	struct ideletorT : ideletor, Base...
	{
		explicit ideletorT(Deletor dtor) : m_deletor(dtor){}
		virtual void destroy()
		{
			m_deletor(this);
		}
	private:
		Deletor m_deletor;
	};

    template<typename Base>
	struct default_deletorT : ideletor, Base
	{
		template<typename ... Args>
		explicit default_deletorT(Args && ... arg) : Base(arg ... ) {}
		virtual void destroy()
		{
			delete this;
		}
	};

	template<typename Base>
	struct no_action_deletorT : ideletor, Base
	{
		template<typename ... Args>
		explicit no_action_deletorT(Args && ... arg) : Base(arg ... ) {}
		virtual void destroy()
		{
		}
	};

    template<typename Deletor, _CLASS_ARG0_DEF_MAX>
	struct ideletorT : ideletor, _ARG0_ARG1_MAX
	{
		explicit ideletorT(Deletor dtor) : m_deletor(dtor){}
		virtual void destroy()
		{
			m_deletor(this);
		}
	private:
		Deletor m_deletor;
	};
#else
    template<typename Base>
	struct default_deletorT : ideletor, Base
	{
        default_deletorT() : Base(){}

		template<typename T0>
		default_deletorT(const T0& t0) 
            : Base(t0) {}
		template<typename T0>
		default_deletorT(T0& t0) 
            : Base(t0) {}
        template<typename T0, typename T1>
		default_deletorT(const T0& t0, const T1& t1) 
            : Base(t0, t1) {}
		virtual void destroy()
		{
			delete this;
		}        
	};

	template<typename Base>
	struct no_action_deletorT : ideletor, Base
	{
        no_action_deletorT() : Base(){}
		template<typename T0>
		no_action_deletorT(const T0& t0) : Base(t0) {}

        template<typename T0, typename T1>
		no_action_deletorT(const T0& t0, const T1& t1) 
            : Base(t0, t1) {}

		virtual void destroy()
		{		}
	};
#endif
	

}
#endif //end AIO_COMMON_IDELETOR_H

