#ifndef XIRANG_DETAIL_TYPE_IMP_H
#define XIRANG_DETAIL_TYPE_IMP_H

#include <xirang/type.h>

#include <map>
#include <vector>

namespace xirang
{
	class TypeImp;
	class TypeItemImp
	{
		public:
			TypeItemImp() : type(0), offset(0){}
			const static std::size_t unknown_offset = std::size_t (-1);

			string name;		//member name. should be valid name.
			string typeName;		//the type name of this member, should be valid name.
			TypeImp *type;		//can be null if unresolved. type->name may diffrent from typeName; since alias, type args.
			std::size_t offset;	//offset in host type.
            size_t index;
	};

	class TypeArgImp
	{
		public:
			TypeArgImp() : type(0){}
			string 		name;
			string      typeName;
			TypeImp* 	type;
	};

	class NamespaceImp;
	class TypeImp
	{
		public:
			TypeImp()
				: modelType(0)
				, payload(Type::no_size), alignment(0), instanceCount(0), referenceCount(0)
				, unresolvedArgs(0)
				, isPod(true)
				, parent(0), methods(0)

			{}
			std::vector < TypeItemImp > 	items;
			//std::map < string, size_t > 	memberIndexByName;	//just members

			std::vector < TypeArgImp>		typeArgs;
			//std::map < string, size_t >	argIndexByName;	//map of parameter types.

			string		name;
			string		modelName;
			TypeImp*	modelType;

			std::size_t payload;	// cached value
			std::size_t alignment;	// cached value;
			std::size_t instanceCount;
			std::size_t referenceCount;
			std::size_t unresolvedArgs;	//cached value
            bool isPod;
			NamespaceImp *parent;
			TypeMethods *methods;

			std::size_t members() const { return items.size(); }
			bool isMemberResolved() const { return payload != Type::no_size; }

            void modelTo(TypeImp& other)
            {
                other.items = items;
                other.typeArgs = typeArgs;
                other.modelName = modelName.empty() ? name : modelName;
                other.modelType = modelName.empty() ? this : modelType;

                other.payload = payload;
                other.alignment = alignment;
                other.instanceCount = instanceCount ;
                other.referenceCount = referenceCount;
                other.unresolvedArgs = unresolvedArgs;
                other.isPod = isPod;
                other.methods = methods;
            }
	};

    struct TypeItemIteratorImp
	{
		typedef std::vector < TypeItemImp >::iterator RealIterator;
		TypeItemIteratorImp(const RealIterator& itr) : rpos(itr){}

		const TypeItem& operator*() const { 
            cache = TypeItem(&*rpos);
            return cache;
        }
        const TypeItem* operator->() const { return &**this;}

		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const TypeItemIteratorImp& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
        mutable TypeItem cache;
	};

    struct TypeArgIteratorImp
	{
		typedef std::vector < TypeArgImp >::iterator RealIterator;
		TypeArgIteratorImp(const RealIterator& itr) : rpos(itr){}

		const TypeArg& operator*() const { 
            cache = TypeArg(&*rpos);
            return cache;
        }
        const TypeArg* operator->() const { return &**this;}

		void operator++ () { ++rpos;}
		void operator-- () { --rpos;}
		bool operator== (const TypeArgIteratorImp& rhs) const { return rpos == rhs.rpos;}

		RealIterator rpos;
        mutable TypeArg cache;
	};
}
#endif				//end XIRANG_DETAIL_TYPE_IMP_H

