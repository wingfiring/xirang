#ifndef XIRANG_DETAIL_TYPE_ALIAS_IMP_H
#define XIRANG_DETAIL_TYPE_ALIAS_IMP_H

#include <xirang/type/xrfwd.h>
namespace xirang{ namespace type{
	class TypeImp;
	class NamespaceImp;
	class TypeAliasImp
	{
	public:
		TypeAliasImp() : type(0), parent(0){}
		string name;
		string typeName;
		TypeImp* type;
		NamespaceImp* parent;
	};
}}

#endif //XIRANG_DETAIL_TYPE_ALIAS_IMP_H;
