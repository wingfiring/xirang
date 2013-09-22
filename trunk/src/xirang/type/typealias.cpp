#include "typealiasimp.h"
#include <aio/xirang/typealias.h>
#include <aio/xirang/namespace.h>
#include "namespaceimp.h"

#include <memory>
#include "impaccessor.h"

namespace xirang
{
	TypeAlias::TypeAlias(TypeAliasImp* imp )
		: m_imp(imp)
	{}


	bool TypeAlias::valid() const
	{
		return m_imp != 0;
	}

	TypeAlias::operator bool() const
	{
		return m_imp != 0;
	}

	const string& TypeAlias::name() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->name;
	}

	const string& TypeAlias::typeName() const
	{
		AIO_PRE_CONDITION(valid());
		return m_imp->typeName;
	}

	Type TypeAlias::type() const
	{
		AIO_PRE_CONDITION(valid());
		return Type(m_imp->type);
	}

	int TypeAlias::compare (TypeAlias other) const
	{
        return comparePtr(m_imp, other.m_imp);
	}

	TypeAliasBuilder::TypeAliasBuilder() 
		: m_imp(0)
	{ 
		renew();
	}

	TypeAliasBuilder::~TypeAliasBuilder()
	{
		if (m_imp)
			aio::check_delete(m_imp);
	}
	TypeAliasBuilder& TypeAliasBuilder::name(const string& alias)
	{
		m_imp->name = alias;
		return *this;
	}
	TypeAliasBuilder& TypeAliasBuilder::typeName(const string& typeName)
	{
		m_imp->typeName = typeName;
		return *this;
	}
	TypeAliasBuilder& TypeAliasBuilder::setType(Type t)
	{
        m_imp->type = ImpAccessor<TypeImp>::getImp(t);
		return *this;
	}

	TypeAliasBuilder& TypeAliasBuilder::renew()
	{
		TypeAliasImp *tmp(new TypeAliasImp);
		tmp->parent = 0;
		tmp->type= 0;
		if (m_imp)
			aio::check_delete(m_imp);
		m_imp = tmp;

		return *this;
	}
	TypeAlias TypeAliasBuilder::get() const
	{
		return TypeAlias(m_imp);
	}

    TypeAlias TypeAliasBuilder::adoptBy(Namespace ns)
    {
        AIO_PRE_CONDITION(!m_imp->name.empty());
        AIO_PRE_CONDITION(!ns.findAlias(m_imp->name).valid());

        aio::unique_ptr<TypeAliasImp> tmp (new TypeAliasImp);
        TypeAlias current = get();

        ImpAccessor<NamespaceImp>::getImp(ns)->alias.insert(std::make_pair(m_imp->name, m_imp));
        m_imp->parent = ImpAccessor<NamespaceImp>::getImp(ns);

        m_imp = tmp.release();

        return current;
    }

    TypeAliasBuilder& TypeAliasBuilder::parent(Namespace ns)
    {
        AIO_PRE_CONDITION(m_imp && !m_imp->parent && ns.valid());
        m_imp->parent = ImpAccessor<NamespaceImp>::getImp(ns);
        return *this;
    }

    TypeAlias TypeAliasBuilder::adoptBy()
    {
        AIO_PRE_CONDITION(!m_imp->name.empty() && m_imp->parent);
        Namespace ns(m_imp->parent);

        AIO_PRE_CONDITION(!ns.findAlias(m_imp->name).valid());

        aio::unique_ptr<TypeAliasImp> tmp (new TypeAliasImp);
        TypeAlias current = get();

        ImpAccessor<NamespaceImp>::getImp(ns)->alias.insert(std::make_pair(m_imp->name, m_imp));
        m_imp = tmp.release();

        return current;
    }

    
    Namespace TypeAliasBuilder::getParent() const
    {
        return m_imp->parent;
    }

    const string& TypeAliasBuilder::getName() const
    {
        return m_imp->name;
    }

    const string& TypeAliasBuilder::getTypeName() const
    {
        return m_imp->typeName;
    }
}
