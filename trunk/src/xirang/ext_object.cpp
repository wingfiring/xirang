#include <aio/xirang/ext_object.h>
#include <aio/xirang/binder.h>
#include <aio/common/backward/atomic.h>

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
			m_handle = handle();
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

			m_ext_heap->unpin(m_data);
			m_data = 0;
		}
	}

	ExtObject::ConstPin::ConstPin()
		: m_obj(0)
	{}
	ExtObject::ConstPin::ConstPin(const ExtObject& obj)
		: m_obj(&const_cast<ExtObject&>(obj))
	{
		AIO_PRE_CONDITION(obj.valid());
		m_obj->pin_();
	}
	ExtObject::ConstPin::ConstPin(ExtObject::ConstPin&& rhs) 
		: m_obj(rhs.m_obj)
	{
		rhs.m_obj = 0;
	}
	ExtObject::ConstPin& ExtObject::ConstPin::operator=(ExtObject::ConstPin&& rhs){
		if(this != &rhs)
			ConstPin(std::move(rhs)).swap(*this);
		return *this;		
	}
	void ExtObject::ConstPin::swap(ExtObject::ConstPin& rhs){
		std::swap(m_obj, rhs.m_obj);
	}
	ExtObject::ConstPin::~ConstPin()
	{
		if (m_obj) m_obj->unpin_();
	}
	ConstCommonObject ExtObject::ConstPin::get() const
	{
		AIO_PRE_CONDITION(valid());
		return ConstCommonObject(m_obj->type(), m_obj->data_());
	}

	bool ExtObject::ConstPin::valid() const{ return m_obj != 0;}
	ExtObject::ConstPin::operator bool() const{ return valid();}

	ExtObject::Pin::Pin(){}
	ExtObject::Pin::Pin(ExtObject& obj) 
		: ExtObject::ConstPin(obj)
	{
	}
	ExtObject::Pin::Pin(Pin&& rhs)
		: ConstPin(std::forward<ConstPin>(rhs))
	{
	}
	ExtObject::Pin& ExtObject::Pin::operator=(Pin&& rhs){
		if(this != &rhs)
			Pin(std::move(rhs)).swap(*this);
		return *this;
		
	}
	ExtObject::Pin::~Pin()
	{
	}
	CommonObject ExtObject::Pin::get() const
	{
		return CommonObject(m_obj->type(), m_obj->data_());
	}
	void ExtObject::Pin::swap(Pin& rhs){
		std::swap(m_obj, rhs.m_obj);
	}

	void constructor<ExtObject>::apply(CommonObject obj, heap& hp, ext_heap& ehp)
	{
		Type t = obj.type();
		AIO_PRE_CONDITION(t.valid() && t.argCount() == 1);

		TypeArg ta = t.arg(0);
		AIO_PRE_CONDITION(ta.name() == "value_type");

		t = ta.type();
		AIO_PRE_CONDITION(t.valid());
		new (obj.data()) ExtObject(t, hp, ehp);
	}

	size_t hasher<ExtObject>::apply(ConstCommonObject obj) {
		return (size_t)obj.data();
	}

	MethodsExtension* extendMethods<ExtObject>::value(){
		static MethodsExtension methodsExt = 
		{
			0,
			&hasher<ExtObject>::apply
		};
		return &methodsExt;
	}
}

