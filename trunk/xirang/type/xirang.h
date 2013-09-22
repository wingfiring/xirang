#ifndef XIRANG_XIRANG_H
#define XIRANG_XIRANG_H

#include <xirang/xrfwd.h>
#include <xirang/type.h>
#include <xirang/namespace.h>
#include <xirang/typealias.h>

namespace xirang{ namespace type{
	class XirangImp;

	class Xirang
	{
	public:

		Xirang (const string & name, heap & al, ext_heap& eh);
		~Xirang ();

		const string & name () const;

        //deprecated
		CommonObject trackNew (Type t, Namespace ns, const string& name);
        //deprecated
		CommonObject untrackNew (Type t);
        //deprecated
		void trackDelete (CommonObject t, Namespace ns, const string& name);
        //deprecated
		void untrackDelete (CommonObject t);

		Namespace root () const;

        heap& get_heap() const;
        ext_heap& get_ext_heap() const;

		/// remove the sub namespace with given name.
		/// return true if remove successfully
		bool removeChild(Namespace ns, string name);
		/// remove the sub object with given name.
		/// return true if remove successfully
		bool removeObject(Namespace ns, string name);

		/// remove all sub namespaces.
		void removeAllChildren(Namespace ns);

		/// remove all sub objects.
		void removeAllObjects(Namespace ns);

		/// remove all sub objects and namespaces.
		void removeAll(Namespace ns);

        CommonObject detachObject(Namespace ns, string name);
	private:
		//non-copyable
		Xirang (Xirang &);
		void operator= (Xirang &);

		XirangImp *m_imp;
	};
	
	void SetupXirang(Xirang& xr);	
}}
#endif				//end XIRANG_XIRANG_H
