#ifndef XIRANG_TYPE_H
#define XIRANG_TYPE_H

#include <xirang/type/xrfwd.h>
#include <xirang/versiontype.h>

namespace xirang{ namespace type{
	class TypeItemImp;

    /// hold the info of type data member
	class TypeItem
	{
		public:
            /// ctor
			TypeItem (TypeItemImp * p = 0);

            /// ctor, iterator supporting
            TypeItem (TypeItemImp& p );

            /// return true if it is constructed with a not null TypeItemImp pointer
			bool valid () const;

            /// \return valid()
			explicit operator bool () const;

            /// get the type of this data member
            /// \pre valid()
			Type type () const;

            /// get the name of data member
            /// \pre valid()
			const string & name () const;

            /// get the type name of this data member
            /// \notes it can be different form type().name()
            /// \pre valid()
			const string & typeName () const;

            /// get the index of this item in the object which own this member directly
            /// \pre valid()
            size_t index() const;

            /// get the offset of this data member in the object which own this member directly
			/// \note if !isResolved(), return Type::unknown;
            /// \pre valid()
			std::size_t offset () const;

            /// return type.valid() && type().isMemberResolved()
            /// \pre valid()
			bool isResolved() const;

            /// compare the internal address only.
            /// return 0 if this and other refer to same data member.
			int compare (const TypeItem&) const;

		private:
			TypeItemImp * m_imp;

			friend class TypeBuilder;
	};
	DEFINE_COMPARE (TypeItem);

	class TypeArgImp;

    /// class to hold type argument info
	class TypeArg
	{
		public:
            /// default ctor
			TypeArg (TypeArgImp * imp = 0);

            /// ctor supporting iterator
			TypeArg (TypeArgImp & imp );

            /// return true if it is constructed with a not null TypeArgImp pointer
			bool valid () const;

            /// \return valid()
			explicit operator bool () const;

            /// get real type of this argument
            /// \pre valid()
			Type type () const;

            /// get type argument name
            /// \pre valid()
			const string & name () const;

            /// get literial type name of this argument
            /// \pre valid()
			const string & typeName () const;


            /// return type().valid()
            /// \pre valid()
			bool isBound() const;

            /// compare the internal address only.
            /// return 0 if this and other refer to same type argument.
			int compare(const TypeArg &) const;
		private:
			TypeArgImp * m_imp;

			friend class TypeBuilder;
	};
	DEFINE_COMPARE (TypeArg);

	class TypeImp;

    /// class to hold type info
	class Type
	{
		public:

            /// the size is unknown if a type contains any data member which type is unresolved.
			static const std::size_t no_size = -1;

            /// default ctor
			Type (TypeImp * imp = 0);

            /// ctor supporting iterator
			Type (TypeImp & imp);

            /// return true if it is constructed with a not null TypeImp
			bool valid () const;

            /// \return valid()
			explicit operator bool () const;

			/// type name without namespace path
            /// \pre valid()
			const string & name () const;

            /// type name with namespace path start from root
            /// \pre valid()
			string fullName () const;

            /// get the parent namespace
            /// \pre valid()
            Namespace parent() const;

			/// if user didn't name a temporary type, it has a internal name.
            /// return true if name start with '~'
            /// \pre valid()
			bool isInterim () const;

			/// return true if all data members are resolved.
            /// \pre valid()
			bool isMemberResolved () const;

			/// return the number of the type argument which type is not bound
            /// \pre valid()
			size_t unresolvedArgs () const;

            /// return isMemberResolved() && unresolvedArgs() == 0
            /// \pre valid()
			bool isComplete() const;

            /// return true if this type was created from a template type
            /// \pre valid()
			bool hasModel () const;

            /// return the template type
            /// \pre valid()
			Type model () const;

            /// name of the model type
            /// \pre valid()
			const string& modelName () const;

			/// get type alignment
            /// \pre valid() && isMemberResolved()
			std::size_t align () const;
			// get type payload
            /// \pre valid() && isMemberResolved()
			std::size_t payload () const;

            /// return true if type is C++ Plain Old Data
            /// \pre valid() && isMemberResolved()
			bool isPod () const;

            /// get the type method
            /// \pre valid()
			TypeMethods & methods () const;

			/// get the number of all data members, memebers.
            /// \pre valid()
			std::size_t memberCount () const;

            /// get all data members, memebers.
            /// \pre valid()
			TypeItemRange members () const;

            /// get the member info by given idx
            /// \pre valid() && idx < memberCount ()
			TypeItem member (std::size_t idx) const;

            /// get the member info by given name
            /// \pre valid()
            /// if not found, return.valid() is false
			TypeItem member (const string& name) const;

            /// get the number of arguments
            /// \pre valid()
			std::size_t argCount () const;

            /// get all arguments
            /// \pre valid()
			TypeArgRange args () const;

            /// get the argument by given index
            /// \pre valid()
			TypeArg arg (std::size_t idx) const;

            /// get the argument by given name
            /// \pre valid()
			TypeArg arg(const string& name) const;

            /// get the type via given name path, search from current type args
            /// \pre valid()
            Type locateType(const string& path, char dim='.') const;

            /// compare the internal address only, do not compare elements.
            /// return 0 if this and other refer to same type
			int compare (const Type &) const;

			// get version of this type.
			const version_type& version() const;
			// get the revision of this type
			revision_type revision() const;
		private:
			friend struct hasher<Type>;
			TypeImp * m_imp;
            friend class ImpAccessor<TypeImp>;
	};
	DEFINE_COMPARE (Type);

	// specialize for Type
	template<> struct hasher<Type> {
		static size_t apply(ConstCommonObject obj);
	};

	class TypeBuilder
	{
		public:
            enum stage
            {
                st_renew,
                st_model,
                st_arg,
                st_member,
                st_end,
            };

            /// default ctor
            /// \post st_renew == getStage()
			TypeBuilder(TypeMethods* methods = 0);

            /// dtor
			~TypeBuilder();

            /// name
            /// \pre !name.empty() && name not contain '.' && name not start with '~'
			TypeBuilder& name(const string& name);

            /// create type from model
            /// if t has no model, the model of this builder will be t
            /// otherwise this builder and t have the same model
            /// \pre t.valid() && getStage() < st_model
            /// \post getStage() == st_model
			TypeBuilder& modelFrom(Type t);

            /// set an arg. if the arg is existed, replace it, otherwise append at the end.
            /// \pre !arg.empty()
            /// \pre getStage <= st_arg
            /// \post getStage == st_arg
			TypeBuilder& setArg(const string& arg, const string& typeName, Type t);

            /// add a data member
            /// \pre getStage <= st_member
            /// \pre !get().member(name).valid()
            /// \post getStage == st_member
			TypeBuilder& addMember(const string& name, const string& typeName, Type t);

            /// end build
            /// \pre getStage < st_end
            /// \post getStage == st_end
			TypeBuilder& endBuild(bool autoResolve = true, char dim = '.');

            /// reset build process
            /// \param methods if methods is null, use default methods
            /// \post getStage == st_renew
			TypeBuilder& renew(TypeMethods* methods = 0);

            /// move the type into given namespce
            /// \pre getStage == st_end
            /// \pre !getParent().valid()
            /// \post getStage == st_renew
            Type adoptBy(Namespace ns);

            /// set the parent namespace
            TypeBuilder& parent(Namespace ns);

            /// move the type into given namespce
            /// \pre getStage == st_end
            /// \pre getParent().valid()
            /// \post getStage == st_renew
            Type adoptBy();

            /// get the in construction type
            Type get() const;

            /// get type name
            const string& getName() const;

            /// get paranet namespace
            Namespace getParent() const;

            /// get build stage
            stage getStage() const;

			revision_type revision() const;

			TypeBuilder& revision(revision_type newRevision);
		private:
			TypeBuilder(const TypeBuilder&) /*= delete*/;
			TypeBuilder& operator=(const TypeBuilder&) /*= delete*/;
			TypeImp* m_imp;
            size_t m_offset;
            stage m_stage;

			friend class NamespaceBuilder;
	};
}}

#endif				//end XIRANG_TYPE_H


