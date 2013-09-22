#ifndef XIRANG_TYPE_ALIAS_H
#define XIRANG_TYPE_ALIAS_H

#include <xirang/xrfwd.h>
#include <xirang/type.h>

namespace xirang { namespace type{
	class TypeAliasImp;

    /// type alias 
	class TypeAlias
	{
		public:
			TypeAlias(TypeAliasImp* imp = 0);			
			
			bool valid() const;

			operator bool () const;

			const string& name() const;
			const string& typeName() const;
			Type type() const;
			int compare (TypeAlias other) const;
		private:
			TypeAliasImp* m_imp;
	};
	DEFINE_COMPARE (TypeAlias);

	class TypeAliasBuilder
	{
	public:
		TypeAliasBuilder();
		~TypeAliasBuilder();
		TypeAliasBuilder& name(const string& alias);
		TypeAliasBuilder& typeName(const string& typeName);
		TypeAliasBuilder& setType(Type t);
		TypeAliasBuilder& renew();
        TypeAliasBuilder& parent(Namespace ns);
        TypeAlias adoptBy();
        TypeAlias adoptBy(Namespace ns);

        TypeAlias get() const;
        Namespace getParent() const;
        const string& getName() const;
        const string& getTypeName() const;

	private:
		TypeAliasImp* m_imp;

		friend class NamespaceBuilder;
	};
}}
#endif //end XIRANG_TYPE_ALIAS_H
