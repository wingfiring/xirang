#include <aio/xirang/type.h>
#include "typeimp.h"
#include "namespaceimp.h"
#include <aio/xirang/typebinder.h>
#include <aio/xirang/namespace.h>

#include "impaccessor.h"
#include <boost/tokenizer.hpp>
#include <aio/common/char_separator.h>

namespace xirang
{

	TypeItem::TypeItem (TypeItemImp * p):m_imp (p) { }
    TypeItem::TypeItem (TypeItemImp & p):m_imp (&p) { }

	TypeItem::operator bool () const
	{
		return m_imp != 0;
	}

	bool TypeItem::valid () const
	{
		return m_imp != 0;
	}

	TypeItem::SubCategory TypeItem::category () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->category;
	}

	Type TypeItem::type () const
	{
		AIO_PRE_CONDITION (valid ());
		return Type (m_imp->type);
	}

	const string & TypeItem::name () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->name;
	}

	const string & TypeItem::typeName () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->typeName;
	}

    size_t TypeItem::index() const
    {
        AIO_PRE_CONDITION (valid ());
        return m_imp->index;
    }

	std::size_t TypeItem::offset () const
	{
		AIO_PRE_CONDITION (valid () );
		return m_imp->offset;
	}

	bool TypeItem::isResolved () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->type != 0 && m_imp->type->isMemberResolved();
	}

	int TypeItem::compare (const TypeItem& rhs) const
	{
        return comparePtr(m_imp, rhs.m_imp);
	}

	aio::archive::reader& operator&(aio::archive::reader& ar, TypeItem& ti)
	{
		return ar;
	}

	aio::archive::writer& operator&(aio::archive::writer& ar, const TypeItem& ti)
	{
		return ar;
	}

	//
	// TypeArg
	//
	TypeArg::TypeArg (TypeArgImp * imp ) : m_imp(imp){}
	TypeArg::TypeArg (TypeArgImp & imp ) : m_imp(&imp){}

	bool TypeArg::valid () const
	{
		return m_imp != 0;
	}

	Type TypeArg::type ()
	{
		AIO_PRE_CONDITION(valid());
		return Type(m_imp->type);
	}

	const string & TypeArg::name () const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->name;
	}

	const string & TypeArg::typeName () const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->typeName;
	}

	bool TypeArg::isBound () const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->type != 0;
	}

	int TypeArg::compare (const TypeArg & rhs) const
	{
        return comparePtr(m_imp, rhs.m_imp);
	}

	extern aio::archive::reader& operator&(aio::archive::reader& ar, TypeArg& ti)
	{
		return ar;
	}

	extern aio::archive::writer& operator&(aio::archive::writer& ar, const TypeArg& ti)
	{
		return ar;
	}

	//
	// Type
	//

	Type::Type (TypeImp * imp):m_imp (imp) { }
	Type::Type (TypeImp & imp):m_imp (&imp) { }

	bool Type::valid () const
	{
		return m_imp != 0;
	}

    Type::operator bool() const
	{
		return m_imp != 0;
	}

	//type name without namespace name
	const string & Type::name () const
	{
		AIO_PRE_CONDITION (valid () );
		return m_imp->name;
	}

	//identifier name
	string Type::fullName () const
	{
		AIO_PRE_CONDITION (valid ());

		std::vector < NamespaceImp * > parents;

		std::size_t sz = m_imp->name.size();

		for (NamespaceImp * ns = m_imp->parent;
				ns != 0; ns = ns->parent)
		{
			parents.push_back(ns);
			sz += ns->name.size();
		}
		sz += parents.size();

		aio::string_builder name;
		name.reserve(sz + m_imp->name.size());
		for (std::vector < NamespaceImp * >::reverse_iterator itr(parents.rbegin()),
				end (parents.rend()); itr != end; ++itr)
		{
			name += (*itr)->name;
			name.push_back('.');
		}
		name += m_imp->name;
		return string(name);
	}

    Namespace Type::parent() const
    {
        AIO_PRE_CONDITION(valid());
        return Namespace(m_imp->parent);
    }

	//if user didn't name a type, it has a internal name.
	bool Type::isInterim () const
	{
		AIO_PRE_CONDITION (valid () );
		return m_imp->name.empty () || m_imp->name[0] == '~';
	}

	bool Type::isMemberResolved() const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->isMemberResolved();
	}

	//true if no sub-item is unresolved.
	size_t Type::unresolvedArgs () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->unresolvedArgs;
	}

	bool Type::isComplete() const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->isMemberResolved()
			&& m_imp->unresolvedArgs == 0;

	}

	bool Type::hasModel () const
	{
		AIO_PRE_CONDITION(valid());
		return !m_imp->modelName.empty();
	}

	Type Type::model () const
	{
		AIO_PRE_CONDITION(valid());
		return Type(m_imp->modelType);
	}

	const string& Type::modelName () const
	{
		AIO_PRE_CONDITION(valid() && hasModel());
		return m_imp->modelName;
	}

	//alignment
	std::size_t Type::align () const
	{
		AIO_PRE_CONDITION (valid () && isMemberResolved());
		return m_imp->alignment;
	}
	//object payload.
	std::size_t Type::payload () const
	{
		AIO_PRE_CONDITION (valid () && isMemberResolved());
		return m_imp->payload;
	}

	//instance count
	std::size_t Type::referenceCount () const
	{
		AIO_PRE_CONDITION (valid () && isMemberResolved());
		return m_imp->referenceCount;
	}

	//instance count
	std::size_t Type::instanceCount () const
	{
		AIO_PRE_CONDITION (valid () && isMemberResolved());
		return m_imp->instanceCount;
	}

	bool Type::isPod () const
	{
		AIO_PRE_CONDITION (valid () && isMemberResolved());
		return m_imp->isPod;
	}

	TypeMethods & Type::methods () const 
	{
		AIO_PRE_CONDITION (valid () && isMemberResolved ());
		AIO_PRE_CONDITION (m_imp->methods != 0);
		return *m_imp->methods;
	}

	//first all sub-item. base + memebers.
	std::size_t Type::memberCount () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->members();
	}

	TypeItemRange Type::members () const
	{
		AIO_PRE_CONDITION (valid ());

		return TypeItemRange(
            TypeItemRange::iterator (TypeItemIteratorImp(m_imp->items.begin())),
			TypeItemRange::iterator (TypeItemIteratorImp(m_imp->items.end()))
				);
	}

	TypeItem Type::member (std::size_t idx) const
	{
		AIO_PRE_CONDITION (valid () && idx < m_imp->members() );
		return TypeItem(&m_imp->items[idx]);
	}

	TypeItem Type::member (const string& name) const
	{
		AIO_PRE_CONDITION (valid ());
		for (std::vector<TypeItemImp>::iterator itr(m_imp->items.begin() + m_imp->bases());
				itr != m_imp->items.end(); ++itr)
		{
			if (name == itr->name)
				return TypeItem(&*itr);
		}
		return TypeItem();
	}

	std::size_t Type::baseCount () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->bases();
	}


	std::size_t Type::argCount () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->typeArgs.size();
	}

	TypeArgRange Type::args () const
	{
		AIO_PRE_CONDITION (valid ());
		return TypeArgRange(
            TypeArgRange::iterator (TypeArgIteratorImp(m_imp->typeArgs.begin())),
			TypeArgRange::iterator (TypeArgIteratorImp(m_imp->typeArgs.end()))
				);
	}
	TypeArg Type::arg (std::size_t idx) const
	{
		AIO_PRE_CONDITION (valid ());
		AIO_PRE_CONDITION (idx < argCount());
		return TypeArg(&m_imp->typeArgs[idx]);
	}

	TypeArg Type::arg(const string& name) const
	{
		AIO_PRE_CONDITION (valid ());

		for (std::vector<TypeArgImp>::iterator itr = m_imp->typeArgs.begin();
				itr != m_imp->typeArgs.end(); ++itr)
		{
			if (name == itr->name)
				return TypeArg(&*itr);
		}
		return TypeArg();
	}

    Type Type::locateType(const string& n, char dim /*= '.'*/) const
	{
		AIO_PRE_CONDITION (valid ());

        if (!n.empty() && n[0] == dim)
            return parent().valid() ? parent().locateType(n, dim) : Type();

        aio::char_separator sep(dim, 0, aio::keep_empty_tokens);
        typedef boost::tokenizer<aio::char_separator, string::const_iterator, aio::const_range_string> tokenizer;
        tokenizer tokens(n, sep);

        tokenizer::iterator itr = tokens.begin();
		tokenizer::iterator end = tokens.end();

        Type res;

        if (itr != end)
        {
            TypeArg theArg = arg(*itr);
            if (!theArg.valid())
                return parent().valid() ? parent().locateType(n, dim) : Type();
            else
                res = theArg.type();
            ++itr;
        }

		for (; res.valid() && itr != end; ++itr)
		{
			TypeArg arg = res.arg(*itr);
            res = arg.valid() ? arg.type() : Type();
		}

		return res;
	}
	int Type::compare (const Type& rhs) const 
	{
        return comparePtr(m_imp, rhs.m_imp);
	}

	extern aio::archive::reader& operator&(aio::archive::reader& ar, Type& ti)
	{
		return ar;
	}

	extern aio::archive::writer& operator&(aio::archive::writer& ar, const Type& ti)
	{
		return ar;
	}

	TypeBuilder::TypeBuilder(TypeMethods* methods )
		: m_imp(0)
	{
		renew(methods);
	}
	TypeBuilder::~TypeBuilder()
	{
		if (m_imp)
			delete m_imp;
	}

	TypeBuilder& TypeBuilder::name(const string& name)
	{
		AIO_PRE_CONDITION(m_imp && !name.empty());
		m_imp->name = name;
		return *this;
	}

	TypeBuilder& TypeBuilder::modelFrom(Type from)
	{
		AIO_PRE_CONDITION(from.valid() );
		AIO_PRE_CONDITION(m_imp);
        AIO_PRE_CONDITION(m_stage < st_model);

        ImpAccessor<TypeImp>::getImp(from)->modelTo(*m_imp);
        m_stage = st_model;

		return *this;
	}

	TypeBuilder& TypeBuilder::setArg(const string& arg, const string& typeName, Type t)
	{
        AIO_PRE_CONDITION(!arg.empty());
        AIO_PRE_CONDITION(m_stage <= st_arg);
        
        TypeArgImp* target = 0;

        for (std::vector<TypeArgImp>::iterator itr = m_imp->typeArgs.begin();
				itr != m_imp->typeArgs.end(); ++itr)
		{
			if (arg == itr->name)
            {
                target = &*itr;
            }
		}
        
        if (target) // replace
        {
            if (!target->type)
                --m_imp->unresolvedArgs;
            target->typeName = typeName;
            target->type = ImpAccessor<TypeImp>::getImp(t);
            if (!target->type)
                ++m_imp->unresolvedArgs;
        }
        else
        {
            m_imp->typeArgs.resize(m_imp->typeArgs.size() + 1);
            m_imp->typeArgs.back().name = arg;
            m_imp->typeArgs.back().typeName = typeName;

            m_imp->typeArgs.back().type = ImpAccessor<TypeImp>::getImp(t);
            if (m_imp->typeArgs.back().type == 0) m_imp->unresolvedArgs++;
        }
        m_stage = st_arg;
		return *this;
	}

	TypeBuilder& TypeBuilder::addBase(const string& typeName, Type type)
	{
		AIO_PRE_CONDITION(m_imp);
        AIO_PRE_CONDITION(m_stage <= st_base);

		m_imp->items.resize(++m_imp->baseCount);
		TypeItemImp& m = m_imp->items.back();

		m.category = TypeItem::sub_base;
        m.typeName = typeName;
		m.type = ImpAccessor<TypeImp>::getImp(type);
        m.index = m_imp->items.size() - 1;

		TypeItem tim(&m);
		m_imp->methods->nextLayout(tim, m_imp->payload, m_offset, m_imp->alignment, m_imp->isPod);

		if (type.valid())
		{	
			m.offset = m_imp->payload - type.payload();
		}
		else
			m.offset = Type::unknown;

        m_stage = st_base;
		return *this;

	}	

	TypeBuilder& TypeBuilder::addMember(const string& name, const string& typeName, Type t)
	{
		AIO_PRE_CONDITION(m_imp);
        AIO_PRE_CONDITION(!name.empty() && !Type(*m_imp).member(name).valid());
        AIO_PRE_CONDITION(m_stage <= st_member);

		m_imp->items.resize(m_imp->items.size() + 1);
		TypeItemImp& m = m_imp->items.back();
		m.name = name;
		m.category = TypeItem::sub_member;
		m.typeName = typeName;
		m.type = ImpAccessor<TypeImp>::getImp(t);
        m.index = m_imp->items.size() - 1;

		TypeItem tim(&m);
		m_imp->methods->nextLayout(tim, m_imp->payload, m_offset, m_imp->alignment, m_imp->isPod);

		if (t.valid())
		{	
			m.offset = m_imp->payload - t.payload();
		}
		else
			m.offset = Type::unknown;
			

        m_stage = st_member;
		return *this;
	}

	TypeBuilder& TypeBuilder::endBuild(bool autoResolve /*= true*/, char dim /*= '.'*/)
	{
        AIO_PRE_CONDITION(m_stage < st_end);

        if (autoResolve && !get().isMemberResolved()){

            m_imp->methods->beginLayout(m_imp->payload, m_offset, m_imp->alignment, m_imp->isPod);
            for (std::vector < TypeItemImp >::iterator itr = m_imp->items.begin(); itr != m_imp->items.end(); ++itr)
            {
                if (!itr->type)
                {
                    Type t = get().locateType(itr->typeName, dim);
                    itr->type = ImpAccessor<TypeImp>::getImp(t);
                }

                if (itr->type && itr->type->isMemberResolved())
                {
                    TypeItem tim(&*itr);
                    itr->offset = m_offset;
                    m_imp->methods->nextLayout(tim, m_imp->payload, m_offset, m_imp->alignment, m_imp->isPod);
                }
                else
                {
                    m_imp->payload = Type::unknown;
                    break;
                }
            }
        }

        m_stage = st_end;

        return *this;
	}

	Type TypeBuilder::get() const
	{
		return Type(m_imp);
	}

	TypeBuilder& TypeBuilder::renew(TypeMethods* methods )
	{
		TypeImp* tmp = new TypeImp;
        tmp->methods = methods == 0? &DefaultMethods() : methods;

		if (m_imp)
			delete m_imp;
		m_imp = tmp;
        
        tmp->methods->beginLayout(m_imp->payload, m_offset, m_imp->alignment, m_imp->isPod);

        m_stage = st_renew;
		return *this;
	}

    Type TypeBuilder::adoptBy(Namespace ns)
    {
        AIO_PRE_CONDITION(!m_imp->name.empty());
        AIO_PRE_CONDITION(!ns.findType(m_imp->name).valid());
        AIO_PRE_CONDITION(m_stage == st_end);

        aio::unique_ptr<TypeImp> tmp (new TypeImp);
        Type current = get();

        ImpAccessor<NamespaceImp>::getImp(ns)->types.insert(std::make_pair(m_imp->name, m_imp));
        m_imp->parent = ImpAccessor<NamespaceImp>::getImp(ns);

        m_imp = tmp.release();
        m_stage = st_renew;

        return current;
    }

    TypeBuilder& TypeBuilder::parent(Namespace ns)
    {
        m_imp->parent = ImpAccessor<NamespaceImp>::getImp(ns);
        return *this;
    }

    Type TypeBuilder::adoptBy()
    {
        AIO_PRE_CONDITION(!m_imp->name.empty());
        Namespace ns(m_imp->parent);

        AIO_PRE_CONDITION(!ns.findType(m_imp->name).valid());
        AIO_PRE_CONDITION(m_imp->parent);
        AIO_PRE_CONDITION(m_stage == st_end);

        aio::unique_ptr<TypeImp> tmp (new TypeImp);
        Type current = get();

        ImpAccessor<NamespaceImp>::getImp(ns)->types.insert(std::make_pair(m_imp->name, m_imp));

        m_imp = tmp.release();
        m_stage = st_renew;

        return current;
    }

    const string& TypeBuilder::getName() const
    {
        return m_imp->name;
    }
    Namespace TypeBuilder::getParent() const
    {
        return m_imp->parent;
    }

    TypeBuilder::stage TypeBuilder::getStage() const
    {
        return this->m_stage;
    }
}


