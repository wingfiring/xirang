#ifndef XIRANG_DETAIL_XIRANG_IMP_H
#define XIRANG_DETAIL_XIRANG_IMP_H

#include <xirang/type/xirang.h>
#include "typeimp.h"
#include "namespaceimp.h"

#include <xirang/type/typebinder.h>
#include <xirang/type/object.h>

#include <memory>
#include <algorithm>
#include <set>
#include <map>
#include "impaccessor.h"
namespace xirang{ namespace type{
	class XirangImp
	{
		public:
			XirangImp (const string& n, heap & al, ext_heap& eh) 
				: alloc(&al), m_ext_heap(&eh)
			{
				name = n;
			}

			~XirangImp ()
			{
				destroy_();
			};

			CommonObject trackNew (Type t, Namespace ns, const string& name)
			{
				AIO_PRE_CONDITION(t.isMemberResolved());
                
                return ObjectFactory(*alloc, *m_ext_heap).create(t, ns, name);
			}

			CommonObject untrackNew (Type t)
			{
				AIO_PRE_CONDITION(t.isMemberResolved());

                return ObjectFactory(*alloc, *m_ext_heap).create(t);
			}

			void trackDelete (CommonObject obj, Namespace ns, const string& name)
			{
				AIO_PRE_CONDITION(obj.valid());
                AIO_PRE_CONDITION(ns.findObject(name).name == 0);

                ObjectDeletor(*alloc).destroy(obj, ns, name);
			}

			void untrackDelete (CommonObject obj)
			{
				AIO_PRE_CONDITION(obj.valid());
                ObjectDeletor(*alloc).destroy(obj);
			}

			bool removeChild(Namespace ns, string name)
			{
				NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
				std::map < string, NamespaceImp* >::iterator iter = pImp->children.find(name);
				if(iter!=pImp->children.end())
				{
					NamespaceImp* pChild = iter->second;
					destroyObj_(*pChild);
					pImp->children.erase(iter);		
					check_delete(pChild);
					return true;
				}
				return false;
			}

			bool removeObject(Namespace ns, string name)
			{
				NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
				std::map < string, CommonObject >::iterator iter = pImp->objects.find(name);
				if(iter!=pImp->objects.end())
				{
					untrackDelete(iter->second);
					pImp->objects.erase(iter);
					return true;
				}
				return false;
			}

			void removeAllChildren(Namespace ns)
			{
				NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
				typedef std::map < string, NamespaceImp* >::iterator ns_iterator;
				for (ns_iterator itr(pImp->children.begin()),
						end(pImp->children.end()); itr != end; ++itr)
				{
					destroyObj_(*itr->second);
					check_delete(itr->second);
				}
				pImp->children.clear();
			}

			void removeAllObjects(Namespace ns)
			{
				NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
				destroyObj_(*pImp);
			}

			void removeAll(Namespace ns)
			{
				removeAllChildren(ns);
				removeAllObjects(ns);
			}
            CommonObject detachObject(Namespace ns, string name)
            {
                NamespaceImp* pImp = ImpAccessor<NamespaceImp>::getImp(ns);
                CommonObject ret;
				std::map < string, CommonObject >::iterator  iter = pImp->objects.find(name);
				if(iter != pImp->objects.end())
				{
                    ret = iter->second;
					pImp->objects.erase(iter);
				}
				return ret;
            }

			string name;
			heap* alloc;
			ext_heap* m_ext_heap;
			NamespaceImp root;
		private:

			void destroy_()
			{
				destroyObj_(root);
				
				root.clear();
			}

			void destroyObj_(NamespaceImp& ns)
			{
				typedef std::map < string, NamespaceImp* >::iterator ns_iterator;
				for (ns_iterator itr(ns.children.begin()),
						end(ns.children.end()); itr != end; ++itr)
				{
					destroyObj_(*itr->second);
				}
				

				for (std::map < string, CommonObject >::iterator itr(ns.objects.begin()), end(ns.objects.end());
						itr != end; ++itr)
				{
					untrackDelete(itr->second);
				}

				ns.objects.clear();
			}
	};
}}

#endif //end XIRANG_DETAIL_XIRANG_IMP_H
