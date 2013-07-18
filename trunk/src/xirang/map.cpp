#include <aio/xirang/map.h>


#include <map>

namespace xirang
{
	struct Comp
	{
		Comp(Type key) : m_key(key)
		{
			AIO_PRE_CONDITION(key.valid());
			AIO_PRE_CONDITION(key.methods().extension() != 0);
			AIO_PRE_CONDITION(key.methods().extension()->compare != 0);

		}

		bool operator()(const void* k1, const void* k2)	const
		{
			return m_key.methods().extension()->compare(ConstCommonObject(m_key, k1), ConstCommonObject(m_key, k2)) < 0;
		}

		Type m_key;
	};

	typedef std::map<void*, void*, Comp> var_type;
	class MapIteratorImp
	{
		public:
		var_type::iterator m_pos;
		Type m_key, m_value;

		MapIteratorImp(Type key, Type value, var_type::iterator pos)
			:m_pos(pos), m_key(key), m_value(value)
		{
		}
		CommonObject get_key()
		{
			return CommonObject(m_key, m_pos->first);
		}
		CommonObject get_value()
		{
			return CommonObject(m_value, m_pos->second);
		}
		void next() { ++m_pos;}
		void prev() { --m_pos;}
	};

	class MapImp
	{
		Type m_key, m_value;

		heap* m_h;
		ext_heap* m_exth;

		void* clone_(ConstCommonObject obj)
		{
			Type type = obj.type();

			void* p = m_h->malloc(type.payload(), type.align(), 
					0);
			type.methods().construct(CommonObject(type, p), *m_h, *m_exth);
			type.methods().assign(obj, CommonObject(type, p));

			return p;
		}
	public:
		typedef Map::const_iterator const_iterator;
		typedef Map::iterator iterator;

		MapImp(heap& h, ext_heap& eh, Type key, Type value)
			: m_key(key), m_value(value), m_h(&h), m_exth(&eh), m_var(Comp(key))
		{
		}

		MapImp(const MapImp& other)
			: m_key(other.m_key), m_value(other.m_value)
			  , m_h(other.m_h), m_exth(other.m_exth), m_var(Comp(other.m_key))
		{
			assign(other);
		}

		~MapImp()
		{
			clear();
		}

		void assign(const MapImp& other)
		{
			AIO_PRE_CONDITION(m_key == other.m_key);
			AIO_PRE_CONDITION(m_value == other.m_value);
			clear();
			for (var_type::const_iterator itr = other.m_var.begin(); itr != other.m_var.end(); ++itr)
			{
				void* pk = clone_(ConstCommonObject(m_key, itr->first));
				void* pv = clone_(ConstCommonObject(m_key, itr->second));
				m_var[pk] = pv;
			}

		}

		MapImp& operator=(const MapImp& other) /*= delete*/;

		Type keyType() const { return m_key;}
		Type valueType() const { return m_value;}

		heap& get_heap() const{ return *m_h;}
		ext_heap& get_ext_heap() const { return *m_exth;}

		std::size_t size() const{ return m_var.size();}
		bool empty() const{ return m_var.empty();}

		iterator begin() { return iterator(new MapIteratorImp(m_key, m_value, m_var.begin()));}
		iterator end() { return iterator(new MapIteratorImp(m_key, m_value, m_var.end()));}

		iterator find(ConstCommonObject key)
		{
			return iterator(new MapIteratorImp(m_key, m_value, m_var.find(const_cast<void*>(key.data()))));
		}

		iterator lower_bound(ConstCommonObject key)
		{
			return iterator(new MapIteratorImp(m_key, m_value, m_var.lower_bound(const_cast<void*>(key.data()))));
		}
		iterator upper_bound(ConstCommonObject key)
		{
			return iterator(new MapIteratorImp(m_key, m_value, m_var.lower_bound(const_cast<void*>(key.data()))));
		}

		CommonObject operator[](ConstCommonObject key) 
		{
			AIO_PRE_CONDITION(key.type() == m_key);
			void* pk = clone_(key);
			void* p = m_h->malloc(m_value.payload(), m_value.align(), 
					0);
			m_value.methods().construct(CommonObject(m_value, p), *m_h, *m_exth);

			m_var[pk] = p;

			return CommonObject(m_value, p);
		}

		void insert(ConstCommonObject k, ConstCommonObject v)
		{
			AIO_PRE_CONDITION(k.type() == m_key);
			AIO_PRE_CONDITION(v.type() == m_value);

			void* pk = clone_(k);
			void* pv = clone_(v);
			m_var[pk] = pv;

		}

		iterator erase(const iterator& pos)
		{
			AIO_PRE_CONDITION(pos.valid());
			m_key.methods().destruct(CommonObject(m_key, pos.m_imp->m_pos->first));
			m_value.methods().destruct(CommonObject(m_value, pos.m_imp->m_pos->second));
			m_h->free(pos.m_imp->m_pos->first, m_key.payload(), m_key.align());
			m_h->free(pos.m_imp->m_pos->second, m_value.payload(), m_value.align());

            var_type::iterator next_pos = pos.m_imp->m_pos;
			m_var.erase(next_pos++);
			return iterator( new MapIteratorImp(m_key, m_value, next_pos));
		}


		void clear()
		{
			for (var_type::iterator itr = m_var.begin(); itr != m_var.end(); ++itr)
			{
				m_key.methods().destruct(CommonObject(m_key, itr->first));
				m_value.methods().destruct(CommonObject(m_value, itr->second));
				m_h->free(itr->first, m_key.payload(), m_key.align());
				m_h->free(itr->second, m_value.payload(), m_value.align());
			}
			m_var.clear();
		}

		private:
		var_type m_var;		
	};

	Map::const_iterator::const_iterator() : m_imp(0){}

	Map::const_iterator::const_iterator(MapIteratorImp* imp)
		: m_imp(imp)
	{
		AIO_PRE_CONDITION(imp != 0);
	}

	Map::const_iterator::const_iterator(const const_iterator& rhs )
		: m_imp(0)
	{
		if (rhs.valid())
			m_imp = new MapIteratorImp(*rhs.m_imp);
	}
	Map::const_iterator::~const_iterator() 
	{
		if (valid())
			aio::check_delete(m_imp);
	}

	Map::const_iterator::value_type Map::const_iterator::first() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->get_key();
	}
	Map::const_iterator::value_type Map::const_iterator::second() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->get_value();
	}

	Map::const_iterator& Map::const_iterator::operator++()
	{
		AIO_PRE_CONDITION(valid());
		m_imp->next();
		return *this;
	}
	Map::const_iterator Map::const_iterator::operator++(int)
	{
		AIO_PRE_CONDITION(valid());
		Map::const_iterator tmp = *this;
		++*this;
		return tmp;
	}

	Map::const_iterator& Map::const_iterator::operator--()
	{
		AIO_PRE_CONDITION(valid());
		m_imp->prev();
		return *this;
	}
	Map::const_iterator Map::const_iterator::operator--(int)
	{
		AIO_PRE_CONDITION(valid());
		Map::const_iterator tmp = *this;
		--*this;
		return tmp;
	}

	bool Map::const_iterator::operator==(const Map::const_iterator& rhs) const
	{
		return valid() 
			? rhs.valid() && m_imp->m_pos == rhs.m_imp->m_pos
			: !rhs.valid();
	}
	bool Map::const_iterator::operator!=(const Map::const_iterator& rhs) const
	{
		return !(*this == rhs);
	}

	bool Map::const_iterator::valid() const
	{
		return m_imp != 0;
	}


	Map::iterator::iterator() : const_iterator(){}

	Map::iterator::iterator(MapIteratorImp* imp)
		: const_iterator(imp)
	{
	}

	Map::iterator::value_type Map::iterator::first() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->get_key();
	}
	Map::iterator::value_type Map::iterator::second() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->get_value();
	}

	Map::iterator& Map::iterator::operator++()
	{
		AIO_PRE_CONDITION(valid());
		m_imp->next();
		return *this;
	}
	Map::iterator Map::iterator::operator++(int)
	{
		AIO_PRE_CONDITION(valid());
		Map::iterator tmp = *this;
		++*this;
		return tmp;
	}

	Map::iterator& Map::iterator::operator--()
	{
		AIO_PRE_CONDITION(valid());
		m_imp->prev();
		return *this;
	}
	Map::iterator Map::iterator::operator--(int)
	{
		AIO_PRE_CONDITION(valid());
		Map::iterator tmp = *this;
		--*this;
		return tmp;
	}


	Map::Map() : m_imp(0){ }
	Map::Map(heap& h, ext_heap& eh, Type key, Type value)
		: m_imp(new MapImp(h, eh, key, value))
	{
		AIO_PRE_CONDITION(key.valid());
		AIO_PRE_CONDITION(value.valid());
	}
				

	Map::Map(const Map& rhs) 
		: m_imp(0)
	{
		if (rhs.valid())
			m_imp = new MapImp(*rhs.m_imp);
	}

	Map::~Map()
	{
		if (valid())
		{
			clear();
			aio::check_delete(m_imp);
		}
	}

	void Map::assign(const Map& other)
	{
		if (valid())
		{
			AIO_PRE_CONDITION(other.valid());
			m_imp->assign(*other.m_imp);
		}
		else if (other.valid())
		{
			m_imp = new MapImp(*other.m_imp);
		}
	}
	Map& Map::operator=(const Map& other)
	{
		if (this != &other)
			Map(other).swap(*this);
		return *this;
	}

	void Map::swap(Map& other)
	{
		std::swap(m_imp, other.m_imp);
	}

	bool Map::valid() const
	{
		return m_imp != 0;
	}

	Type Map::keyType() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->keyType();
	}
	Type Map::valueType() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->valueType();
	}

	heap& Map::get_heap() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->get_heap();
	}
	ext_heap& Map::get_ext_heap() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->get_ext_heap();
	}

	std::size_t Map::size() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->size();
	}
	bool Map::empty() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->empty();
	}

	Map::const_iterator Map::begin() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->begin();
	}
	Map::iterator Map::begin()
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->begin();
	}
	Map::const_iterator Map::end() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->end();
	}
	Map::iterator Map::end()
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->end();
	}

	Map::iterator Map::find(ConstCommonObject key)
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->find(key);
	}
	Map::const_iterator Map::find(ConstCommonObject key) const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->find(key);
	}

	Map::iterator Map::lower_bound(ConstCommonObject key)
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->lower_bound(key);
	}
	Map::const_iterator Map::lower_bound(ConstCommonObject key) const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->lower_bound(key);
	}
	Map::iterator Map::upper_bound(ConstCommonObject key)
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->upper_bound(key);
	}
	Map::const_iterator Map::upper_bound(ConstCommonObject key) const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->upper_bound(key);
	}


	CommonObject Map::operator[](ConstCommonObject key) 
	{
		AIO_PRE_CONDITION(valid());
		return (*m_imp)[key];
	}

	void Map::insert(ConstCommonObject k, ConstCommonObject v)
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->insert(k, v);
	}
	Map::iterator Map::erase(const iterator& pos)
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->erase(pos);
	}


	void Map::clear()
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->clear();
	}

}
