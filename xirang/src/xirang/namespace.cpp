#include "namespaceimp.h"

#include <aio/xirang/namespace.h>
#include <aio/xirang/type.h>
#include <aio/xirang/typealias.h>
#include <aio/xirang/object.h>
#include "impaccessor.h"

#include <boost/tokenizer.hpp>
#include <aio/common/char_separator.h>

namespace xirang
{

	// ************
	//  Namespace
	// ************
	Namespace::Namespace (NamespaceImp * imp):m_imp (imp)
	{
	}

	const string & Namespace::name () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->name;
	}

    string Namespace::fullName(char dim /*='.'*/) const
    {
        std::vector <Namespace> parents;
        aio::string_builder name;
        if (!m_imp->parent)
        {
            name.push_back(dim);
            return string(name);
        }            

        std::size_t sz = m_imp->name.size();

        parents.push_back(*this);
        for (Namespace ns = m_imp->parent;
            ns.valid(); ns = ns.parent())
        {
            parents.push_back(ns);
            sz += ns.name().size();
        }
        parents.pop_back(); //pop root
        sz += parents.size(); //for dim space

        
        name.reserve(sz + m_imp->name.size() + 2);
        for (std::vector < Namespace >::reverse_iterator itr(parents.rbegin()),
            end (parents.rend()); itr != end; ++itr)
        {
            name.push_back(dim);
            name += itr->name();            
        }
        return string(name);
    }
	Type Namespace::locateType(const string& n, char dim) const
	{
		AIO_PRE_CONDITION (valid ());

        aio::char_separator sep(dim, 0, aio::keep_empty_tokens);
        typedef boost::tokenizer<aio::char_separator, string::const_iterator, aio::const_range_string> tokenizer;
        tokenizer tokens(n, sep);

        tokenizer::iterator itr = tokens.begin();
		tokenizer::iterator end = tokens.end();
		

		Namespace cur;

        if (!n.empty() && n[0] == dim) {    //is absolute
            ++itr;
            cur = root();
        }
        else if (itr != end)
        {
            cur = *this;
            string id = *itr;
            for (; cur.valid() ; cur = cur.parent())
            {
                if (cur.findNamespace(id).valid()
                    || cur.findType(id).valid()
                    || cur.findAlias(id).valid()
                    )
                    break;
            }
        }

		Type res;
		for (; itr != end; ++itr)
		{
			string id = *itr;
			
			Namespace tns = cur.findNamespace(id);
			if (tns.valid())
			{
				cur = tns;
				continue;
			}
			res = cur.findType(id);
			if (res.valid())
			{
				++itr;
				break;
			}
			TypeAlias tas = cur.findAlias(id);
			if (tas.valid())
			{
				res = tas.type();
				++itr;
				break;
			}
			return Type();
		}
		for (; res.valid() && itr != end; ++itr)
		{
			TypeArg arg = res.arg(*itr);
			res = arg.valid() ? arg.type() : Type();
		}

		return res;
	}

	Namespace Namespace::locateNamespace(const string& n, char dim) const
	{
		AIO_PRE_CONDITION (valid ());
		if (n.size() == 1 && *n.begin() == dim)
			return this->root();

        aio::char_separator sep(dim, 0, aio::keep_empty_tokens);
        typedef boost::tokenizer<aio::char_separator, string::const_iterator, aio::const_range_string> tokenizer;
        tokenizer tokens(n, sep);

		tokenizer::iterator itr = tokens.begin();
		tokenizer::iterator end = tokens.end();

		Namespace cur = *this;

		bool isAbsolute = false;
		if (itr != end && (*itr).empty())
		{
			cur = cur.root();
			++itr;
			isAbsolute = true;
		}

		for (; itr != end; ++itr)
		{
            string id = *itr;
			if (!isAbsolute)
			{
				for (; cur.valid() && !cur.findNamespace(id).valid(); cur = cur.parent())
					;
				isAbsolute = true;
				if (!cur.valid())
					break;
			}
			cur = cur.findNamespace(id);
			if (!cur.valid())
				break;
		}

		return cur;
	}

	Type Namespace::findType (const string & t) const
	{
		AIO_PRE_CONDITION (valid ());
		Type result = findRealType(t);
		if(result.valid())
		   return result;
		TypeAlias alias = findAlias(t);
		return alias.valid() ? alias.type() : Type();
	}

	Type Namespace::findRealType (const string & t) const
	{
		AIO_PRE_CONDITION (valid ());
		std::map < string, TypeImp* >::iterator pos = m_imp->types.find (t);

		return pos == m_imp->types.end() ? Type() : Type(pos->second);
	}

	Namespace Namespace::findNamespace (const string & ns) const
	{
		AIO_PRE_CONDITION (valid ());
		std::map < string, NamespaceImp* >::iterator pos =
			m_imp->children.find (ns);
		return pos == m_imp->children.end ()
			? Namespace () : Namespace (pos->second);
	}

	TypeAlias Namespace::findAlias (const string & t) const
	{
		AIO_PRE_CONDITION (valid ());
		std::map < string, TypeAliasImp* >::iterator pos = m_imp->alias.find (t);

		return pos == m_imp->alias.end() ? TypeAlias() : TypeAlias(pos->second);
	}

	TypeRange Namespace::types () const
	{
		AIO_PRE_CONDITION (valid ());
        return TypeRange (
            TypeRange::iterator(TypeIteratorImp(m_imp->types.begin())),
            TypeRange::iterator(TypeIteratorImp(m_imp->types.end()))
            );
	}

	NamespaceRange Namespace::namespaces () const
	{
		AIO_PRE_CONDITION (valid ());
        return NamespaceRange (
            NamespaceRange::iterator(NamespaceIteratorImp(m_imp->children.begin())),
            NamespaceRange::iterator(NamespaceIteratorImp(m_imp->children.end()))
            );
	}

	TypeAliasRange Namespace::alias () const
	{
		AIO_PRE_CONDITION (valid ());
		return TypeAliasRange (
            TypeAliasRange::iterator(TypeAliasIteratorImp(m_imp->alias.begin())),
            TypeAliasRange::iterator(TypeAliasIteratorImp(m_imp->alias.end()))
            );
	}

	ObjectRange Namespace::objects () const
	{
		AIO_PRE_CONDITION (valid ());
		return ObjectRange (
            ObjectRange::iterator(ObjIteratorImp(m_imp->objects.begin())),
            ObjectRange::iterator(ObjIteratorImp(m_imp->objects.end()))
            );
	}

    NameValuePair Namespace::findObject(const string& name) const
    {
        const static string last_version;
        return findObject(name, last_version);
    }

    NameValuePair Namespace::findObject(const string& name, const string& /*version*/ /*= ""*/) const
    {
        std::map < string, CommonObject>::iterator pos = m_imp->objects.find(name);
        NameValuePair ret = {0,CommonObject()};
        if (pos != m_imp->objects.end())
        {
            ret.name = &pos->first;
            ret.value = pos->second;
        }
        return ret;
    }

	bool Namespace::empty () const
	{
		AIO_PRE_CONDITION (valid ());
		return m_imp->types.empty ()
			&& m_imp->children.empty () && m_imp->objects.empty ()
			&& m_imp->alias.empty();
	}

	Namespace Namespace::parent () const
	{
		AIO_PRE_CONDITION (valid ());
		return Namespace (m_imp->parent);
	}

	Namespace Namespace::root() const
	{
		AIO_PRE_CONDITION (valid ());

		NamespaceImp* rt = m_imp;
		for (; rt->parent != 0; rt = rt->parent)
			;
		return Namespace(rt);
	}

	int Namespace::compare (Namespace other) const
	{
        return comparePtr(m_imp, other.m_imp);
	}

	bool Namespace::valid() const 
	{
		return m_imp != 0;
	}

	Namespace::operator bool() const 
	{
		return m_imp != 0;
	}

    NamespaceBuilder& NamespaceBuilder::createChild(const string& path, char dim /* = '.' */)
    {
        aio::char_separator sep(dim, 0, aio::keep_empty_tokens);
        typedef boost::tokenizer<aio::char_separator, string::const_iterator, aio::const_range_string> tokenizer;
        tokenizer tokens(path, sep);

        tokenizer::iterator itr = tokens.begin();
		tokenizer::iterator end = tokens.end();
        
        for (Namespace current(get()); itr != end; ++itr)
        {
            string name = *itr;
            if (name.empty())
                AIO_THROW(invalid_namespace_path)("path contains empty name.")(path);
            Namespace next = current.findNamespace(name);
            if (!next.valid())
                next = NamespaceBuilder().name(name).adoptBy(current);
            current = next;
        }
        return *this;
    }

	NamespaceBuilder& NamespaceBuilder::adopt(TypeBuilder& tb)
	{
		AIO_PRE_CONDITION(m_imp && tb.get().valid());
        
		tb.adoptBy(get());

		return *this;
	}

	NamespaceBuilder& NamespaceBuilder::adopt(NamespaceBuilder& tb)
	{
		AIO_PRE_CONDITION(m_imp && tb.get().valid());

        tb.adoptBy(get());
		
		return *this;
	}

	NamespaceBuilder& NamespaceBuilder::adopt(TypeAliasBuilder& tb)
	{
		AIO_PRE_CONDITION(m_imp && tb.get().valid());

        tb.adoptBy(get());

        return *this;
	}

	Namespace NamespaceBuilder::adoptBy(Namespace ns)
	{
		AIO_PRE_CONDITION(m_imp && ns.valid());
        AIO_PRE_CONDITION(!m_imp->parent);
        
        aio::unique_ptr<NamespaceImp> pnew (new NamespaceImp);

		string name = get().name();
		AIO_PRE_CONDITION(!name.empty() && !ns.findNamespace(name).valid());

        Namespace current = get();
        ImpAccessor<NamespaceImp>::getImp(ns)->children.insert(std::make_pair(name, m_imp));
		m_imp->parent = ImpAccessor<NamespaceImp>::getImp(ns);
		m_imp = pnew.release();

        return current;
	}

    template<typename Cont>
    static void append_(const Cont& from, Cont& dest)
    {
        for (typename Cont::const_iterator itr = from.begin(); itr != from.end(); ++itr)
        {
            if(dest.find(itr->first) != dest.end())
                AIO_THROW(conflict_namespace_child_name)("name existed in target namesapce")(itr->first);
            dest.insert(*itr);
        }
    }

    template<typename Cont>
    static void changeParent_(Cont& from, NamespaceImp* target)
    {
        for (typename Cont::iterator itr = from.begin(); itr != from.end(); ++itr)
        {
            AIO_PRE_CONDITION(itr->second->parent != target);
            itr->second->parent = target;
        }
        from.clear();
    }
    NamespaceBuilder& NamespaceBuilder::adoptChildrenBy(Namespace ns)
	{
		AIO_PRE_CONDITION(m_imp && ns.valid());
        AIO_PRE_CONDITION(!m_imp->parent);

        NamespaceImp* target = ImpAccessor<NamespaceImp>::getImp(ns);
        std::map < string, TypeImp* > types = target->types;
        std::map < string, NamespaceImp* > children = target->children;
        std::map < string, TypeAliasImp * > alias = target->alias;
        std::map < string, CommonObject> objects = target->objects;
        
        append_(m_imp->types, types);
        append_(m_imp->children, children);
        append_(m_imp->alias, alias);
        append_(m_imp->objects, objects);
        
        types.swap(target->types);
        children.swap(target->children);
        alias.swap(target->alias);
        objects.swap(target->objects);

        changeParent_(m_imp->types, target);
        changeParent_(m_imp->children, target);
        changeParent_(m_imp->alias, target);
        m_imp->objects.clear();

        return *this;
	}
    NamespaceBuilder& NamespaceBuilder::parent(Namespace ns)
    {
        AIO_PRE_CONDITION(m_imp && !m_imp->parent && ns.valid());
        m_imp->parent = ImpAccessor<NamespaceImp>::getImp(ns);
        return *this;
    }

    Namespace NamespaceBuilder::adoptBy()
	{
		AIO_PRE_CONDITION(m_imp && m_imp->parent);

        aio::unique_ptr<NamespaceImp> pnew (new NamespaceImp);

        Namespace ns(m_imp->parent);
		string name = getName();
		AIO_PRE_CONDITION(!name.empty() && !ns.findNamespace(name).valid());

        Namespace current = get();
        m_imp->parent->children.insert(std::make_pair(name, m_imp));
        m_imp = pnew.release();

        return current;
	}

	NamespaceBuilder::NamespaceBuilder() 
		: m_imp(new NamespaceImp)
	{	}

	NamespaceBuilder::~NamespaceBuilder()
	{
		if (m_imp)
			delete m_imp;
	}

	NamespaceBuilder& NamespaceBuilder::name(const string& n)
	{
		AIO_PRE_CONDITION(m_imp);
		m_imp->name = n;
		return *this;
	}
	Namespace NamespaceBuilder::get() const
	{
		return Namespace(m_imp);
	}

    const string& NamespaceBuilder::getName() const
    {
        return m_imp->name;
    }
    Namespace NamespaceBuilder::getParent() const
    {
        return Namespace (m_imp->parent);
    }

    void NamespaceBuilder::swap(NamespaceBuilder& other)
    {
        std::swap(m_imp, other.m_imp);
    }
}
