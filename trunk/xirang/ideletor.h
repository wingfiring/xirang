//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_IDELETOR_H
#define AIO_COMMON_IDELETOR_H

#include <aio/common/config.h>

namespace aio
{
	struct AIO_INTERFACE ideletor
	{
		virtual void destroy() = 0;
		protected:
		virtual ~ideletor(){}
	};

	template<typename Derive> struct ideletor_co : public ideletor
	{
		virtual void destroy(){
			return static_cast<const Derive*>(this)->get_target()->destroy();
		}
	};
	template<typename Derive, typename CoClass>
	ideletor_co<Derive> get_interface_map(Derive*, ideletor*, CoClass*);


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
			check_delete(this);
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
}
#endif //end AIO_COMMON_IDELETOR_H

