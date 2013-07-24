#ifndef AIO_XIRANG_EXT_OBJECT_H
#define AIO_XIRANG_EXT_OBJECT_H

#include <aio/xirang/object.h>
#include <aio/xirang/typebinder.h>
#include <aio/xirang/serialize.h>

#include <aio/common/memory.h>

namespace xirang
{
	//Type of ExtObject must be relocatable.
	class ExtObject
	{
	public:
		struct ConstPin
		{
			ConstPin();
			ConstPin(const ExtObject& obj);
			ConstPin(ConstPin&& rhs);
			ConstPin& operator=(ConstPin&& rhs);
			~ConstPin();

			ConstCommonObject get() const;
			bool valid() const;
			explicit operator bool() const;
			void swap(ConstPin& rhs);

		protected:
			ExtObject* m_obj;

			ConstPin(const ConstPin&) /*= delete*/;
			ConstPin& operator=(const ConstPin&) /*= delete*/;
		};
		struct Pin : ConstPin
		{
			Pin();
			Pin(ExtObject& obj);
			Pin(Pin&& rhs);
			Pin& operator=(Pin&& rhs);
			~Pin();

			CommonObject get() const;
			void swap(Pin& rhs);

		private:
			Pin(const Pin&) /*= delete*/;
			Pin& operator=(const Pin&) /*= delete*/;
		};

		friend struct Pin;
		friend struct ConstPin;

		ExtObject();
		ExtObject(Type t, heap& h, ext_heap& eh);
		ExtObject(const ExtObject& rhs);
		~ExtObject();
		ExtObject& operator=(const ExtObject& rhs);

		bool valid() const;

		Type type() const;
		heap& get_heap() const;
		ext_heap& get_ext_heap() const;

		void swap(ExtObject& rhs);

		template<typename Archive>
		friend Archive& operator &(Archive & ar, const ExtObject& obj)
		{
			return ar & obj.m_heap & obj.m_ext_heap & obj.m_type 
				& obj.m_handle.begin & obj.m_handle.end;
		}

		std::size_t pinCount() const;

	private:

		void* data_() const;
		void pin_() const;
		void unpin_() const;
		void destroy_();

		heap* m_heap;
		ext_heap* m_ext_heap;
		mutable void * m_data;
		mutable std::size_t m_counter;

		Type m_type;
		mutable ext_heap::handle m_handle;
	};

	template<> struct constructor<ExtObject>{
		static void apply(CommonObject obj, heap& hp, ext_heap& ehp);
	};

	template<> struct hasher<ExtObject> {
		static size_t apply(ConstCommonObject obj);
	};

	template<> struct extendMethods<ExtObject> {
		static MethodsExtension* value();
	};

}
#endif //end AIO_XIRANG_EXT_OBJECT_H
