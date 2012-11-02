#include <aio/xirang/array.h>
#include <aio/xirang/typebinder.h>

#include <aio/common/buffer.h>
#include <stdint.h>

namespace xirang
{
	class ArrayImp
	{
	public:
		ArrayImp(heap& al, ext_heap& eh, Type t)
			: type(t), data(al), eheap(&eh)
		{
			AIO_PRE_CONDITION(t.valid());
		}
		Type type;
		aio::buffer<byte> data;
		ext_heap* eheap;
	};

	Array::Array() 
		: m_imp(0)
	{}
	Array::Array(heap& al, ext_heap& eh, Type t) 
		:m_imp(new ArrayImp(al,eh, t))
	{
	}

	Array::Array(const Array& other)
		: m_imp(0) 
	{
		if (!other.valid())
			return;

		m_imp = new ArrayImp(other.get_heap(), other.get_ext_heap(), other.type());
		if (other.empty())
			return;

		if (m_imp->type.isPod())
		{
			m_imp->data = other.m_imp->data;
		}
		else
		{
			m_imp->data.resize(other.m_imp->data.size());
			byte *p = m_imp->data.data();
			for (Array::const_iterator itr = other.begin(); itr != other.end(); ++itr)
			{
				//TODO: exception safe code
				CommonObject obj(m_imp->type, p);
				m_imp->type.methods().construct(obj, get_heap(), get_ext_heap());
				m_imp->type.methods().assign(*itr, obj);
				p += m_imp->type.payload();
			}
		}
	}

	Array::~Array()
	{
		if (m_imp)
		{
			clear();
			delete m_imp;
		}
	}

	void Array::assign(const Array& rhs)
	{
		if (this != &rhs)
			Array(rhs).swap(*this);
	}

	Array& Array::operator=(const Array& other)
	{
		if (&other != this)
			Array(other).swap(*this);
		return *this;
	}

	void Array::swap(Array& other)
	{
		std::swap(m_imp, other.m_imp);
	}

	bool Array::valid() const 
	{
		return m_imp != 0;
	}

	Type Array::type() const
	{
		return valid() ? m_imp->type : Type();
	}

	heap& Array::get_heap() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->data.get_heap();
	}

	ext_heap& Array::get_ext_heap() const
	{
		AIO_PRE_CONDITION(valid());
		return *m_imp->eheap;
	}

	std::size_t Array::size() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->data.size() / type().payload();
	}

	bool Array::empty() const
	{
		return m_imp == 0 || m_imp->data.empty();
	}

	Array::const_iterator Array::begin() const
	{
		AIO_PRE_CONDITION(valid());
		return const_iterator(type(), empty() ? 0 : m_imp->data.data());
	}

	Array::iterator Array::begin() 
	{
		AIO_PRE_CONDITION(valid());
		return iterator(type(), empty() ? 0 : m_imp->data.data());
	}

	Array::const_iterator Array::end() const 
	{
		AIO_PRE_CONDITION(valid());
		return const_iterator(type(), empty() ? 0 : m_imp->data.data() + m_imp->data.size());
	}

	Array::iterator Array::end() 
	{
		AIO_PRE_CONDITION(valid());
		return iterator(type(), empty() ? 0 : m_imp->data.data() + m_imp->data.size());
	}

	ConstCommonObject Array::front() const 
	{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(!empty());
		return *begin();
	}

	CommonObject Array::front() 
	{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(!empty());
		return *begin();
	}

	ConstCommonObject Array::back() const
	{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(!empty());
		return *--end() ;
	}

	CommonObject Array::back() 
	{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(!empty());
		return *--end() ;
	}


	ConstCommonObject Array::operator[](std::size_t idx)  const
	{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(!empty());
		AIO_PRE_CONDITION(idx < size());
		return *(begin() + idx);
	}

	CommonObject Array::operator[](std::size_t idx) 
	{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(!empty());
		AIO_PRE_CONDITION(idx < size());
		return *(begin() + idx);
	}

	void Array::push_back(ConstCommonObject obj)
	{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(obj.type() == type());
		std::size_t old_size = m_imp->data.size();
		//TODO:not safe if object hold a pointer to itself.
		m_imp->data.resize(old_size + type().payload());
		byte* p = m_imp->data.data() + old_size;
		if (!type().isPod())
			type().methods().construct(CommonObject(type(), p), get_heap(), get_ext_heap());
		type().methods().assign(obj, CommonObject(type(), p) );

	}
	void Array::pop_back()
	{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(!empty());
		std::size_t new_size = m_imp->data.size() - type().payload();
		if (!type().isPod())
			type().methods().destruct(CommonObject(type(), m_imp->data.data() + new_size));
		m_imp->data.resize(new_size);
	}

	void Array::insert(Array::iterator pos, ConstCommonObject obj)
	{
		AIO_PRE_CONDITION(valid());
        std::size_t s = size();

		Type t = type();
		std::size_t idx = (pos - begin()) * t.payload();
		//TODO:not safe if object hold a pointer to itself.
		m_imp->data.insert(m_imp->data.begin() + idx, t.payload(), 0);
		if (!type().isPod())
			type().methods().construct(CommonObject(type(), m_imp->data.data() + idx), get_heap(), get_ext_heap());
		type().methods().assign(obj, CommonObject(type(), m_imp->data.data() + idx) );
        
        AIO_POST_CONDITION(size() == s + 1);
	}

	Array::iterator Array::erase(Array::iterator pos)
	{
		AIO_PRE_CONDITION(valid());
		AIO_PRE_CONDITION(!empty());
		AIO_PRE_CONDITION(pos != end());
		Array::iterator ia = pos;
		for (Array::iterator last = --end(); ia != last; ++ia)
		{
            ia->assign( *(ia + 1));
		}
		pop_back();
		return pos;
	}

	void Array::resize(std::size_t s)
	{
		AIO_PRE_CONDITION(valid());

		Type t = type();
		TypeMethods& methods = t.methods();

		std::size_t new_size = s * type().payload();
		if (s < size())
		{
			if (!t.isPod())
			{
				for (byte* p = m_imp->data.data() + m_imp->data.size() - type().payload()
						; p >= m_imp->data.data() + new_size; p -= type().payload())
				{
					methods.destruct(CommonObject(t, p));
				}
			}
			m_imp->data.resize(new_size);
		}
		if(s > size())
		{
			std::size_t old_size = m_imp->data.size();
			m_imp->data.resize(new_size);

			//TODO: need exception safe
			for (byte* p = m_imp->data.data() + old_size
					; p != m_imp->data.data() + m_imp->data.size(); p += type().payload())
			{
				methods.construct(CommonObject(t, p), get_heap(), get_ext_heap());
			}
		}
	}

	void Array::reserve(std::size_t s)
	{
		AIO_PRE_CONDITION(valid());

		m_imp->data.reserve(s * type().payload());
	}

	std::size_t Array::capacity() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->data.capacity() / type().payload();
	}

	void Array::clear()
	{
		AIO_PRE_CONDITION(valid());

        Type t = type();
        if (!t.isPod() && !empty())
        {
            TypeMethods& methods = t.methods();
            for (byte* p = m_imp->data.data() + m_imp->data.size() - type().payload()
                ; p >= m_imp->data.data(); p -= type().payload())
            {
                methods.destruct(CommonObject(t, p));
            }
        }
        m_imp->data.clear();
	}

    int Array::compare(const Array& rhs) const
    {
        AIO_PRE_CONDITION(type() == rhs.type());

		if (!valid() && !rhs.valid())
			return 0;

		const MethodsExtension* pextern = type().methods().extension();

		AIO_PRE_CONDITION(pextern && pextern->compare);

		size_t min_size = std::min(size(), rhs.size());
        Array::const_iterator last = begin() + min_size;

		for (Array::const_iterator li = begin(), ri = rhs.begin(); li != last; ++li, ++ri)
		{
			int ret = pextern->compare(*li, *ri);
            if (ret != 0)
				return ret;
		}
        return min_size < size()
            ? 1
            : min_size < rhs.size()
            ? -1 
            : 0;
    }

    void constructor<Array>::apply(CommonObject obj, heap& hp, ext_heap& ehp)
    {
        AIO_PRE_CONDITION(obj.type().unresolvedArgs() == 0);

        Type value_type = obj.type().arg(0).type();
        new (obj.data()) Array(hp, ehp, value_type);
    }

    aio::archive::writer& serializer<Array>::apply(aio::archive::writer& ar, ConstCommonObject obj){
        const Array& buf = uncheckBind<Array>(obj);
        ar & (uint32_t)buf.size();
        Type t = buf.type();

        std::size_t size = buf.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            t.methods().serialize(ar, buf[i]);
        }
        return ar;
    }


    aio::archive::reader& deserializer<Array>::apply(aio::archive::reader& rd, CommonObject obj, heap& inner, ext_heap& ext){

        Array& buf = uncheckBind<Array>(obj);
        uint32_t size;
        Type t = buf.type();
        rd & size;
		buf.resize(size);

        for(std::size_t i = 0; i < size;  i++)
        {
            t.methods().deserialize(rd, buf[i], inner, ext);
        }
        return rd;
    }


    size_t hasher<Array>::apply(ConstCommonObject obj) {
            return (size_t)obj.data();
        }

    int comparison<Array>::apply(ConstCommonObject lhs,ConstCommonObject rhs) {
        return uncheckBind<Array>(lhs).compare(uncheckBind<Array>(rhs));
    }
}

