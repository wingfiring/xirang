#include <xirang/type/typebinder.h>
#include <xirang/type/type.h>
#include <xirang/io/versiontype.h>
#include <xirang/io/sha1.h>

#include <cstring>

#include <boost/functional/hash.hpp>

namespace xirang { namespace type{

	namespace {
		TypeMethods defaultMethods;
		std::size_t align_lcm(std::size_t p1, std::size_t p2)
		{
			return p1 < p2 ? p2 : p1;
		}
	}
	version_type calculateObjVersion(ConstCommonObject obj){
		AIO_PRE_CONDITION(obj.valid());
		sha1 sha;
		iref<io::writer> sink(sha);
		obj.type().methods().serialize(sink.get<io::writer>(), obj);

		version_type ver;
		ver.id = sha.get_digest();
		return std::move(ver);
	}

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
	void TypeMethods::serialize(io::writer& wr, ConstCommonObject obj){
		AIO_PRE_CONDITION(obj.valid());
		AIO_PRE_CONDITION(&obj.type().methods() == this );

		for (auto &i : obj.members()){
			i.type().methods().serialize(wr, i.asCommonObject());
		}
	}
	void TypeMethods::deserialize(io::reader& rd, CommonObject obj, heap& inner, ext_heap& outer){
		AIO_PRE_CONDITION(obj.valid());
		AIO_PRE_CONDITION(&obj.type().methods() == this );

		for (auto &i : obj.members()){
			i.type().methods().deserialize(rd, i.asCommonObject(), inner, outer);
		}
	}

	const MethodsExtension* TypeMethods::extension() const
	{
		return 0;
	}

	version_type TypeMethods::getTypeVersion(Type t) const{
		AIO_PRE_CONDITION(t.valid());
		AIO_PRE_CONDITION(&t.methods() == this );
		sha1 sha;
		calculateTypeSha(sha, t);

		version_type ver;
		ver.id = sha.get_digest();
		return std::move(ver);
	}
	// TODO: unified type sha & serialization
	void TypeMethods::calculateTypeSha(sha1& sha, Type t) const{
		AIO_PRE_CONDITION(t.valid());
		AIO_PRE_CONDITION(&t.methods() == this );

		auto s = io::exchange::as_sink(sha);
		AIO_PRE_CONDITION(t.valid());
		s & t.modelName() & t.model();
		for (auto& i : t.args())
			s & i.name() & i.typeName() & i.type();
		for (auto& i : t.members())
			s & i.name() & i.typeName() & i.type();
	}
	static const string K_GenericType(".sys.type.generic$");
	const string& TypeMethods::internalID() const{
		return K_GenericType;
	}

    size_t hasher<string>::apply(ConstCommonObject obj)
    {
        const string& v = uncheckBind<string>(obj);
        return v.hash();
    }
}}

