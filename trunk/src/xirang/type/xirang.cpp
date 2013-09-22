#include "xirangimp.h"
#include <xirang/type/namespace.h>
#include <xirang/type/type.h>
#include <xirang/type/typealias.h>
#include <xirang/type/object.h>

#include <cctype>

namespace xirang{ namespace type{
	Xirang::Xirang (const string& name, heap& al, ext_heap& eh)
		: m_imp(new XirangImp(name, al, eh))
	{ }

	Xirang::~Xirang ()
	{
		check_delete(m_imp);
	}

	const string & Xirang::name () const
	{
		return m_imp->name;
	}

	CommonObject Xirang::trackNew (Type t, Namespace ns, const string& name)
	{
		AIO_PRE_CONDITION(t.valid() && ns.valid());
		return m_imp->trackNew (t, ns, name);
	}

	CommonObject Xirang::untrackNew (Type t)
	{
		AIO_PRE_CONDITION(t.valid());
		return m_imp->untrackNew (t);
	}

	void Xirang::trackDelete (CommonObject t, Namespace ns, const string& name)
	{
		AIO_PRE_CONDITION(t.valid() && ns.valid());
		m_imp->trackDelete (t, ns, name);
	}

	void Xirang::untrackDelete (CommonObject t)
	{
		AIO_PRE_CONDITION(t.valid());
		m_imp->untrackDelete (t);
	}

	bool Xirang::removeChild(Namespace ns, string name)
	{
		return m_imp->removeChild(ns,name);
	}
	bool Xirang::removeObject(Namespace ns, string name)
	{
		return m_imp->removeObject(ns, name);
	}

	void Xirang::removeAllChildren(Namespace ns)
	{
		 m_imp->removeAllChildren(ns);
	}

	void Xirang::removeAllObjects(Namespace ns)
	{
		 m_imp->removeAllObjects(ns);
	}

	void Xirang::removeAll(Namespace ns)
	{
		m_imp->removeAll(ns);
	}

    CommonObject Xirang::detachObject(Namespace ns, string name)
    {
        return m_imp->detachObject(ns, name);
    }
	Namespace Xirang::root () const
	{
		return Namespace(&m_imp->root);
	}

    heap& Xirang::get_heap() const
    {
        return *m_imp->alloc;
    }
    ext_heap& Xirang::get_ext_heap() const
    {
        return *m_imp->m_ext_heap;
    }

	
}}

