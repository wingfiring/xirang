#include <aio/xirang/ext_object.h>
#include <aio/xirang/binder.h>
#include <aio/common/archive/ext_heap_archive.h>
#include <aio/common/atomic.h>

namespace xirang
{
	ExtObject::ExtObject()
		: m_heap(0), m_ext_heap(0), m_data(0), m_counter(0)
	{ 
	}

	ExtObject::ExtObject(Type t, heap& h, ext_heap& eh)
		: m_heap(&h), m_ext_heap(&eh), m_data(0), m_counter(0), m_type(t)
	{
	}
	ExtObject::ExtObject(const ExtObject& rhs)
		: m_heap(rhs.m_heap), m_ext_heap(rhs.m_ext_heap)
		  , m_data(0), m_counter(0), m_type(rhs.m_type)
	{
		if (rhs.m_handle)
		{
			AIO_PRE_CONDITION(rhs.valid());
			Pin pt(*this);
			ConstPin po(rhs);
			m_type.methods().assign(po.get(), pt.get());
		}
	}
	
	ExtObject::~ExtObject()
	{
		destroy_();
	}

	ExtObject& ExtObject::operator=(const ExtObject& rhs)
	{
		AIO_PRE_CONDITION(m_type == rhs.type());
		
		if (this != &rhs)
		{
			ExtObject(rhs).swap(*this);
		}

		return *this;
	}

	bool ExtObject::valid() const
	{
		return m_type.valid();
	}

	void ExtObject::swap(ExtObject& rhs)
	{
		AIO_PRE_CONDITION(m_counter == 0 && rhs.m_counter == 0);

		using std::swap;
		swap(m_heap, rhs.m_heap);
		swap(m_ext_heap, rhs.m_ext_heap);
		swap(m_data, rhs.m_data);
		swap(m_counter, rhs.m_counter);

		swap(m_type, rhs.m_type);
		swap(m_handle, rhs.m_handle);
		swap(m_handle_archive, rhs.m_handle_archive);
	}

	void ExtObject::release()
	{
        //TODO: imp
	}

	void ExtObject::destroy_()
	{
		AIO_PRE_CONDITION(m_counter == 0);
		if (m_handle)
		{
			pin_();
			m_type.methods().destruct(CommonObject(m_type, m_data));
			m_ext_heap->unpin(m_data);
			m_ext_heap->deallocate(m_handle);
			m_handle.clear();
		}

		if (m_handle_archive)
		{
			m_ext_heap->deallocate(m_handle_archive);
			m_handle_archive.clear();
		}
	}

	Type ExtObject::type() const
	{
		return m_type;
	}

	heap& ExtObject::get_heap() const
	{
		return *m_heap;
	}
	ext_heap& ExtObject::get_ext_heap() const
	{
		return *m_ext_heap;
	}

	std::size_t ExtObject::pinCount() const
	{
		return m_counter;
	}
	void* ExtObject::data_() const
	{
		AIO_PRE_CONDITION(m_counter > 0 );
		return m_data;
	}
	void ExtObject::pin_() const
	{
		if (m_counter == 0)
		{
			//lazy construct
			if (!m_handle)
			{
				m_handle = m_ext_heap->allocate(m_type.payload(), m_type.align(), handle());
				m_data = m_ext_heap->pin(m_handle);
				m_type.methods().construct(CommonObject(m_type, m_data), *m_heap, *m_ext_heap );
			}
			else
			{
				m_data = m_ext_heap->pin(m_handle);
				
				if (m_handle_archive.valid())
				{
					aio::archive::ext_heap_archive ar(*m_ext_heap, m_handle_archive);
					m_type.methods().deserialize(ar, CommonObject(m_type, m_data), *m_heap, *m_ext_heap);
				}
			}
		}
		++m_counter;
	}
	void ExtObject::unpin_() const
	{
		AIO_PRE_CONDITION(m_counter > 0 );
		--m_counter;
		if (m_counter == 0)
		{
			AIO_PRE_CONDITION(m_data != 0);

			aio::archive::ext_heap_archive ar(*m_ext_heap, ext_heap::handle());
			m_type.methods().serialize(ar, CommonObject(m_type, m_data));
			if (m_handle_archive.valid())
				m_ext_heap->deallocate(m_handle_archive);
			m_handle_archive = ar.get_handle();

			m_ext_heap->unpin(m_data);
			m_data = 0;
		}
	}

	ExtObject::ConstPin::ConstPin(const ExtObject& obj)
		: m_obj(obj), m_data(0)
	{
		AIO_PRE_CONDITION(obj.valid());
		m_obj.pin_();
	}
	ExtObject::ConstPin::~ConstPin()
	{
		m_obj.unpin_();
	}
	ConstCommonObject ExtObject::ConstPin::get()
	{
		return ConstCommonObject(m_obj.type(), m_obj.data_());
	}


	ExtObject::Pin::Pin(ExtObject& obj)
		: m_obj(obj), m_data(0)
	{
		AIO_PRE_CONDITION(obj.valid());
		m_obj.pin_();
	}
	ExtObject::Pin::~Pin()
	{
		m_obj.unpin_();
	}
	CommonObject ExtObject::Pin::get()
	{
		return CommonObject(m_obj.type(), m_obj.data_());
	}


	void ExtObjMethods::construct(CommonObject obj, heap& inner, ext_heap& outer) const
	{
		Type t = obj.type();
		AIO_PRE_CONDITION(t.valid() && t.argCount() == 1);

		TypeArg ta = t.arg(0);
		AIO_PRE_CONDITION(ta.name() == "value_type");

		t = ta.type();
		AIO_PRE_CONDITION(t.valid());
		new (obj.data()) ExtObject(t, inner, outer);
	}
	void ExtObjMethods::destruct(CommonObject obj) const
	{
		ExtObject& eo = uncheckBind<ExtObject>(obj);
		eo.~ExtObject();
	}
	void ExtObjMethods::assign(ConstCommonObject src, CommonObject dest) const
	{
		*reinterpret_cast<ExtObject*>(dest.data()) = *reinterpret_cast<const ExtObject*>(src.data());
	}

	void ExtObjMethods::deserialize(aio::archive::reader& rd, CommonObject obj, heap& inner, ext_heap& ext) const
	{
		rd & *reinterpret_cast<ExtObject*>(obj.data());
	}
	void ExtObjMethods::serialize(aio::archive::writer& wr, ConstCommonObject obj) const
	{
		const ExtObject& eo = uncheckBind<const ExtObject>(obj);
		AIO_PRE_CONDITION(eo.pinCount() == 0);

		wr & eo;
	}

	void ExtObjMethods::release(CommonObject obj) const
	{
		uncheckBind<ExtObject>(obj).release();
	}
	void ExtObjMethods::beginLayout(std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
	{
		payload = sizeof(ExtObject);
		offset = 0;
		align = sizeof(void*);
		pod = false;
	}
	void ExtObjMethods::nextLayout(TypeItem& item, std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
	{
		AIO_PRE_CONDITION(false && "should no composed item");
	}
	const TypeInfoHandle& ExtObjMethods::typeinfo() const
	{
		return typeinfo_;
	}
	const MethodsExtension* ExtObjMethods::extension() const
	{
		static MethodsExtension methodsExt = 
		{
			0,
			&hash_
		};
		return &methodsExt;
	}
	std::size_t ExtObjMethods::hash_(ConstCommonObject lhs)
	{
		union {
			const void * p;
			std::size_t hash;
		} res;
		res.p = lhs.data();
		return res.hash;
	}
	const TypeInfo<ExtObject> ExtObjMethods::typeinfo_;
}
