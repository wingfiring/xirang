#include <xirang/typebinder.h>
#include <xirang/type.h>
#include <cstring>

#include <boost/functional/hash.hpp>

namespace xirang {

	namespace {
		TypeMethods defaultMethods;
		std::size_t align_lcm(std::size_t p1, std::size_t p2)
		{
			return p1 < p2 ? p2 : p1;
		}

		TypeInfo<void> voidType;
	}

	TypeInfoHandle::~TypeInfoHandle() {}

	TypeMethods& DefaultMethods()
	{
		return defaultMethods;
	}

	void TypeMethods::construct(CommonObject obj, heap& al, ext_heap& eh) const
	{
		AIO_PRE_CONDITION(obj.valid());

		Type t = obj.type();
		byte* p = reinterpret_cast<byte*>(obj.data());

		char* pos = reinterpret_cast<char*>(p);
		TypeItemRange members = t.members();
		for (TypeItemRange::iterator itr(members.begin()); itr != members.end(); ++itr)
		{
			Type ti = (*itr).type();
			AIO_PRE_CONDITION(ti.valid() && ti.isMemberResolved());
			ti.methods().construct(CommonObject(ti, pos + (*itr).offset()), al, eh);
		}
	}

	void TypeMethods::destruct(CommonObject obj) const
	{
		AIO_PRE_CONDITION(obj.valid());

		Type t = obj.type();

		if (t.isPod())
			return;

        SubObjRange members = obj.members();
        typedef std::reverse_iterator<SubObjRange::iterator> iterator_type;
        for (SubObjRange::iterator itr (iterator_type(members.end())), end(iterator_type(members.begin()));
            itr != end; ++itr)
        {   
            SubObject ti = *itr;
            Type t2 = ti.type();
            if (!t2.isPod())
                t2.methods().destruct(ti.asCommonObject());
        }
	}

	TypeMethods::~TypeMethods()
	{
	}

	void TypeMethods::assign(ConstCommonObject src, CommonObject dest) const
	{
		AIO_PRE_CONDITION(src.valid() && dest.valid() );
		AIO_PRE_CONDITION(src.type() == dest.type() );

		Type t = src.type();

		if (t.isPod())
		{
			std::memcpy(dest.data(), src.data(), t.payload());
			return;
		}
        
        ConstSubObjRange rng_src = src.members();
        SubObjRange rng_dest = dest.members();
        
        ConstSubObjRange::iterator itr_src = rng_src.begin();
        SubObjRange::iterator itr_dest = rng_dest.begin();
        for (; itr_src != rng_src.end(); ++itr_src, ++itr_dest)
        {
            ConstSubObject tis = *itr_src;
            SubObject tid = *itr_dest;
            tid.asCommonObject().assign( tis.asCommonObject());
        }
	}

	void TypeMethods::beginLayout(std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
	{
		payload = 0;
		offset = 0;
		align = 1;
		pod = true;
	}
	void TypeMethods::nextLayout(TypeItem& item, std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
	{
		if (payload != Type::no_size)
		{
			if (!item.type().valid())
			{
				payload = Type::no_size;
				offset = Type::no_size;
				align = Type::no_size;
			}
			else
			{
				AIO_PRE_CONDITION(item.type().isComplete());

				std::size_t padding = payload % item.type().align();
				if (padding != 0)
					padding = item.type().align() - padding;
				payload += item.type().payload() + padding;
				offset = payload;
				pod = pod && item.type().isPod();
				align = align_lcm(align, item.type().align());
			}
		}
	}
	const TypeInfoHandle& TypeMethods::typeinfo() const
	{
		return voidType;
	}
	const MethodsExtension* TypeMethods::extension() const
	{
		return 0;
	}

    size_t hasher<string>::apply(ConstCommonObject obj)
    {
        const string& v = uncheckBind<string>(obj);
        return v.hash();
    }
}
