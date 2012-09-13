#ifndef AIO_XIRANG_EXT_OBJECT_H
#define AIO_XIRANG_EXT_OBJECT_H

#include <aio/xirang/object.h>
#include <aio/xirang/typebinder.h>

#include <aio/common/memory.h>
#include <aio/common/iarchive.h>

namespace xirang
{
	class ExtObject
	{
	public:
		struct ConstPin
		{
			ConstPin(const ExtObject& obj);
			~ConstPin();
			ConstCommonObject get();

			ConstPin(const ConstPin&) /*= delete*/;
			ConstPin& operator=(const ConstPin&) /*= delete*/;
		private:
			const ExtObject& m_obj;
			void* m_data;

		};
		struct Pin 
		{
			Pin(ExtObject& obj);
			~Pin();
			CommonObject get();

			Pin(const Pin&) /*= delete*/;
			Pin& operator=(const Pin&) /*= delete*/;
		private:
			ExtObject& m_obj;
			void* m_data;
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
		void release();

		template<typename Archive>
		friend Archive& operator &(Archive & ar, ExtObject& obj)
		{
			return ar & obj.m_heap & obj.m_ext_heap & obj.m_type 
				& obj.m_handle.begin & obj.m_handle.end 
				& obj.m_handle_archive.begin & obj.m_handle_archive.end;
		}
		template<typename Archive>
		friend Archive& operator &(Archive & ar, const ExtObject& obj)
		{
			return ar & obj.m_heap & obj.m_ext_heap & obj.m_type 
				& obj.m_handle.begin & obj.m_handle.end 
				& obj.m_handle_archive.begin & obj.m_handle_archive.end;
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
		mutable ext_heap::handle m_handle_archive;
	};

	struct ExtObjMethods : public TypeMethods
	{
		virtual void construct(CommonObject obj, heap& , ext_heap&) const;
		virtual void destruct(CommonObject obj) const;
		virtual void assign(ConstCommonObject src, CommonObject dest) const;
		virtual void deserialize(aio::archive::reader& rd, CommonObject obj, heap& inner, ext_heap& ext) const;
		virtual void serialize(aio::archive::writer& wr, ConstCommonObject obj) const;

		virtual void release(CommonObject obj) const;

		virtual void beginLayout(std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const;
		virtual void nextLayout(TypeItem& item, std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const;
		virtual const TypeInfoHandle& typeinfo() const;
		virtual const MethodsExtension* extension() const;
		private:
		static std::size_t hash_(ConstCommonObject lhs);

		static const TypeInfo<ExtObject> typeinfo_;
	};

}
#endif //end AIO_XIRANG_EXT_OBJECT_H
