//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_ASSERT_H
#define AIO_ASSERT_H

/** @file 

	contract programming support class
	includes runtime invariants check and compile time check.
	three invariant check are implemented: AIO_PRE_CONDITION, AIO_POST_CONDITION, AIO_INVARIANT.
*/

#include <aio/common/config.h>
#include <aio/common/exception.h>
#include <aio/common/macro_helper.h>

//must be latest include
#include <aio/common/config/abi_prefix.h>

namespace aio
{
	/**	includes invariant checkers.
	*/
	/** supported invatiant contract_categorys 
		@see http://en.wikipedia.org/wiki/Design_by_contract
		@see http://www.artima.com/intv/contracts.html
	*/
	enum class contract_category
	{				
		pre= 0,
		post,
		invariant
	};

	struct assertion_tag {};
	struct precondition_tag : assertion_tag {};
	struct postcondition_tag : assertion_tag {};
	struct invariant_tag : assertion_tag {};


	/** if needs customize error processing method, create a new engine dirived from this class*/
	class contract_handler
	{
	public:
		/** if invariants broken, this function called
			@param type invariant check type
			@param expr invariant check expression
			@param sourcefile source file name
			@param function in function name
			@param line line number
			@param message addtional message
			@return if the impementation ignores a error, return true, else return false.
			@note generally, process should print diagnosis info and then abort or active debuger.
			if necessary, process can throw an exception instead of abort, such as in release version.
			but be careful, use exceptions instead of invariants is almost always a bad practice.
		*/
		static bool process(contract_category type
			, const char_utf8* expr
			, const char_utf8* sourcefile
			, const char_utf8* function
			, int line
			, const char_utf8* message);

		virtual bool do_process(contract_category type
			, const char_utf8* expr
			, const char_utf8* sourcefile
			, const char_utf8* function
			, int line
			, const char_utf8* message) = 0;


		/** derivedable */
		virtual ~contract_handler();

		/** set error report engine
			@param engine new error report engine
		*/
		static void set_handle(contract_handler* engine);

		/** get error report engine
			@return current error report engine
		*/
		static contract_handler* get_handle();
	};

	/** it's used to keep the current contract_handler pointer and restore it when exit the scope.
	*/
	struct contract_handler_saver
	{
		/// \ctor Default ctor, save the result of contract_handler::get_handle() call
		contract_handler_saver() 
			: m_old(contract_handler::get_handle())
		{}

		/// \ctor same as the default ctor but set the current handler to new_handler
		explicit contract_handler_saver(contract_handler* new_handler ) 
			: m_old(contract_handler::get_handle())
		{
			contract_handler::set_handle(new_handler);
		}

		/// \dtor restore the saved handler.
		~contract_handler_saver() { contract_handler::set_handle(m_old);}
	private:
		contract_handler* m_old;
	};

	/** treat invariant as exception, base of assert_exception_impl
	*/
	AIO_EXCEPTION_TYPE(contract_exception);
	
	AIO_EXCEPTION_TYPE_EX(pre_exception, contract_exception);

	AIO_EXCEPTION_TYPE_EX(post_exception, contract_exception);
	
	AIO_EXCEPTION_TYPE_EX(invariant_exception, contract_exception);


	/** convert assert check failure to exceptions*/
	class exception_reporter : public contract_handler
	{
		/** @copydoc aio::contract::contract_handler::do_process */
		virtual bool do_process(contract_category type, 
				const char_utf8* expr, 
				const char_utf8* sourcefile, 
				const char_utf8* function, 
				int line, 
				const char_utf8* message);
	};

	/** default error report engine */
	class console_reporter : public contract_handler
	{
	public:
		/** @copydoc aio::contract::contract_handler::do_process */
		virtual bool do_process(contract_category type, 
				const char_utf8* expr, 
				const char_utf8* sourcefile, 
				const char_utf8* function, 
				int line, 
				const char_utf8* message);
	};

	void aio_assert(const char_utf8* expr, const char_utf8* sourcefile, 
		const char_utf8* function, int line, const char_utf8* message);
}

#if !defined(NDEBUG) || defined(AIO_KEEP_CONTRACT_ASSERT)
#  define AIO_AIO_INVARIANT_COMM_EX(contract_category, expr, msg)\
	(expr) ||\
		::aio::contract_handler::process(contract_category, \
		AIO_STRING(expr), __FILE__,\
			AIO_FUNCTION, __LINE__, msg)

#  define AIO_AIO_INVARIANT_COMM(contract_category, expr)\
	AIO_AIO_INVARIANT_COMM_EX(contract_category, expr, 0)

#else
#  define AIO_AIO_INVARIANT_COMM_EX(contract_category, expr, msg) 
#  define AIO_AIO_INVARIANT_COMM(contract_category, expr) 

#endif //end NDEBUG

#define AIO_PRE_CONDITION_EX(expr, msg)\
	AIO_AIO_INVARIANT_COMM_EX(aio::contract_category::pre, expr, msg)

#define AIO_POST_CONDITION_EX(expr, msg)\
	AIO_AIO_INVARIANT_COMM_EX(aio::contract_category::post, expr, msg)

#define AIO_INVARIANT_EX(expr, msg)\
	AIO_AIO_INVARIANT_COMM_EX(aio::contract_category::invariant, expr, msg)

#define AIO_PRE_CONDITION(expr)\
	AIO_AIO_INVARIANT_COMM(aio::contract_category::pre, expr)

#define AIO_POST_CONDITION(expr)\
	AIO_AIO_INVARIANT_COMM(aio::contract_category::post, expr)

#define AIO_INVARIANT(expr)\
	AIO_AIO_INVARIANT_COMM(aio::contract_category::invariant, expr)

#include <aio/common/config/abi_suffix.h>
#endif // end AIO_ASSERT_H

