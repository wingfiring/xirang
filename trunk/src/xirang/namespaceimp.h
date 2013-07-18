#ifndef XIRANG_DETAIL_NAMESPACE_IMP_H
#define XIRANG_DETAIL_NAMESPACE_IMP_H

#include "typeimp.h"
#include "typealiasimp.h"
#include <aio/xirang/object.h>
#include <map>

namespace xirang
{

	class TypeImp;
	class NamespaceImp
	{
		public:
			NamespaceImp() : parent(0){}
			~NamespaceImp() 
			{
				clear();
			}

			void clear()
			{
				AIO_PRE_CONDITION(objects.empty());

                name.clear();

				typedef std::map < string, NamespaceImp* >::iterator ns_iterator;
				for (ns_iterator itr(children.begin()),
						end(children.end()); itr != end; ++itr)
				{
					aio::check_delete(itr->second);
				}
                children.clear();

				
				for (std::map < string, TypeImp* >::iterator itr(types.begin()),
						end(types.end()); itr != end; ++itr)
				{
					aio::check_delete(itr->second);
				}
				types.clear();

                for (std::map < string, TypeAliasImp * >::iterator itr = alias.begin(); itr != alias.end(); ++itr)
                    aio::check_delete(itr->second);
				alias.clear();

				parent = 0;
			}

			string name;

			std::map < string, TypeImp* > types;
			std::map < string, NamespaceImp* > children;
			std::map < string, TypeAliasImp * > alias;
			std::map < string, CommonObject> objects;

			NamespaceImp *parent;

	};

	struct TypeIteratorImp
	{
		typedef std::map < string, TypeImp* >::iterator RealIterator;
		TypeIteratorImp(const RealIterator& itr) : rpos(itr){}

		const Type& operator*() const { return *reinterpret_cast<Type*>(&rpos->second);}
        const Type* operator->() const { return &**this;}

		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const TypeIteratorImp& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
	};

	struct TypeAliasIteratorImp
	{
		typedef std::map < string, TypeAliasImp* >::iterator RealIterator;
		TypeAliasIteratorImp(const RealIterator& itr) : rpos(itr){}

		const TypeAlias& operator*() const { return *reinterpret_cast<TypeAlias*>(&rpos->second);}
        const TypeAlias* operator->() const { return &**this;}
		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const TypeAliasIteratorImp& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
	};

	struct NamespaceIteratorImp
	{
		typedef std::map < string, NamespaceImp* >::iterator RealIterator;
		NamespaceIteratorImp(const RealIterator& itr) : rpos(itr){}

		const Namespace& operator*() const { return *reinterpret_cast<Namespace*>(&rpos->second);}
        const Namespace* operator->() const { return &**this;}
		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const NamespaceIteratorImp& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
	};

	struct ObjIteratorImp
	{
		typedef	std::map < string, CommonObject >::iterator RealIterator;

		ObjIteratorImp(const RealIterator& itr) : rpos(itr)
        {
            value.name = 0;
        }

        const NameValuePair& operator*() const { 
            value.name = &rpos->first;
            value.value = rpos->second;
            return value;
        }
        
        const NameValuePair* operator->() const { 
            value.name = &rpos->first;
            value.value = rpos->second;
            return &value;
        }

		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const ObjIteratorImp& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
        mutable NameValuePair value;
	};

}

#endif				//end XIRANG_DETAIL_NAMESPACE_IMP_H
