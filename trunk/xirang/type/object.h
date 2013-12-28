#ifndef XIRANG_OBJECT_H
#define XIRANG_OBJECT_H

#include <xirang/type/type.h>

namespace xirang { namespace type{
	class SubObjIterator;
	class ConstSubObjIterator;
	class TypeImp;

    /// hold the const object created from a Type
	class ConstCommonObject
	{
		public:

            /// ctor
			ConstCommonObject (Type t = Type(), const void *p = 0);

            /// return false if !type().valid()
			bool valid () const;

            /// return true if valid()
			operator bool () const;

            /// get the type of this object
			Type type () const;

            /// get all data members
            /// \pre valid()
			ConstSubObjRange members() const;

            /// get the data member by given index
            /// \pre valid()
            ConstSubObject getMember(size_t idx) const;

            /// get the data member by given name
            /// \pre valid()
            ConstSubObject getMember(const string& name) const;

            /// treat this object as a ConstSubObject
            /// \pre valid()
			ConstSubObject  asSubObject () const;

            /// get the data blob address of this object
            /// \return null if !valid()
			const void* data() const;

            /// compare data() and other.data() but no precondition check
            /// \return return 0 if internal data pointers are equals
			int compare (const ConstCommonObject & other) const;

		protected:
			Type m_type;
			void *m_obj;
	};
	DEFINE_COMPARE (ConstCommonObject);

    /// hold the modifiable object created from a Type
	class CommonObject : public ConstCommonObject
	{
		public:
            /// ctor
			CommonObject (Type t = Type(), void *p = 0);

            /// get all data members
            /// \pre valid()
			SubObjRange members() const;

            /// treat this object as a SubObject
            /// \pre valid()
			SubObject  asSubObject () const;

            /// get the data member by given index
            /// \pre valid()
            SubObject getMember(size_t idx) const;

            /// get the data member by given name
            /// \pre valid()
            SubObject getMember(const string& name) const;

            /// get the data blob address of this object
            /// \return null if !valid()
			void* data() const;

            /// assign the value from obj to this
            /// \pre valid() && obj.valid() && type() == obj.type()
			void assign(ConstCommonObject obj);
	};
	template<typename Handler, typename Obj, typename ... Args>
	inline void VisitObjectMember(Handler& h, Obj obj, Args... args)
	{
		if (obj.type().memberCount > 0){
			auto children = obj.members();
			for (auto i: children)
				VisitObjectMember(h, i.asCommonObject(), args...);
		}
		h(obj, args...);
	}

    struct NameValuePair
    {
        const string* name;
        CommonObject value;
    };

    struct ConstNameValuePair
    {
        const string* name;
        ConstCommonObject value;
    };
    //. reference to data member of compound type
	class ConstSubObject
	{
		public:
			const static std::size_t PhonyIndex = -1;

            /// ctor
            /// \param t type of the object which own this member directly
            /// \pre (p == 0 && !t.valid() && idx == 0) || (p != 0 && t.valid()
			///		&& (idx < t.memberCount() || idx == PhonyIndex))
            /// \pre p == 0 || idx == PhonyIndex || t.member(idx).type().valid()
			ConstSubObject (Type t = Type(), const void *p = 0, std::size_t idx = 0);

            /// true if type
			bool valid () const;

            /// return true if valid()
			operator bool () const;

            /// get the type of this member
			Type type () const;

            /// get the type of object which own this member
			Type ownerType () const;

            /// convert this to a ConstCommonObject
            /// \pre valid()
			ConstCommonObject asCommonObject() const;

            /// get the address of data blob
            /// \pre valid()
			const void* data() const;

            /// get the address of object which own this member
			const void* ownerData () const;

            /// return 0 if both index and address of blob are equal
			int compare (const ConstSubObject &) const;

            /// get the index
            /// \pre valid()
            size_t index() const;
		protected:
			bool regular_() const;
			bool isPhony_() const;
			Type m_type;
			void *m_obj;
			std::size_t m_index;

			friend class ConstSubObjIterator;
			friend class SubObjIterator;
	};
	DEFINE_COMPARE (ConstSubObject);

	class SubObject : public ConstSubObject
	{
		public:
            /// ctor
            /// \pre (p == 0 && !t.valid() && idx == 0) || (p != 0 && t.valid()
			///		&& (idx < t.memberCount() || idx == PhonyIndex))
            /// \pre p == 0 || idx == PhonyIndex || t.member(idx).type().valid()
			SubObject (Type t = Type(), void *p = 0, std::size_t idx = 0);

            /// convert this to an CommonObject
            /// \pre valid()
			CommonObject asCommonObject() const;

            /// get the address of data blob
            /// \pre valid()
			void* data() const;
	};

    /// data member const iterator
	class ConstSubObjIterator
	{
		public:
            /// default ctor
			ConstSubObjIterator ();

            /// point to given SubObject
            /// \pre rhs.valid()
			ConstSubObjIterator (const ConstSubObject & rhs);

			const ConstSubObject& operator* () const;

            /// const member accessor
            const ConstSubObject* operator->() const;
			ConstSubObjIterator & operator++ ();
			ConstSubObjIterator operator++ (int);
			ConstSubObjIterator & operator-- ();
			ConstSubObjIterator operator-- (int);

            /// return true if point to same data member
			bool equals (const ConstSubObjIterator &) const;
		protected:
			SubObject m_subObj;
	};

	inline bool operator==(const ConstSubObjIterator& lhs, const ConstSubObjIterator& rhs)
	{ return lhs.equals(rhs);}

	inline bool operator!=(const ConstSubObjIterator& lhs, const ConstSubObjIterator& rhs)
	{ return !lhs.equals(rhs);}

    /// data member iterator
	class SubObjIterator : public ConstSubObjIterator
	{
		public:
            /// ctor
			SubObjIterator ();

            /// point to given SubObject
            /// \pre rhs.valid()
			SubObjIterator (const SubObject & rhs);

			const SubObject&  operator* () const;
            const SubObject* operator->() const;
			SubObjIterator & operator++ ();
			SubObjIterator operator++ (int);
			SubObjIterator & operator-- ();
			SubObjIterator operator-- (int);
	};

    /// Facotry pattern
    class ObjectFactory
	{
    public:
        /// ctor just initilize with (xi.get_heap(), xi.get_ext_heap())
        explicit ObjectFactory (const Xirang& xi);

        /// ctor
        ObjectFactory (heap & al, ext_heap& eh) ;

        /// create an object with given type
        /// \pre t.valid()
        /// \post return.valid()
        CommonObject create(Type t);

        /// create an object with given type, and put it into given namespace
        /// \pre t.valid() && ns.valid && !name.empty() && ns.findObject(name).name == 0
        /// \post return.valid()
        CommonObject create(Type t, Namespace ns, const string& name);

        /// create an object from given obj
        /// \pre obj.valid()
        /// \post return.valid()
        CommonObject clone(ConstCommonObject obj);

        /// create an object from given obj, and put it into given namespace
        /// \pre obj.valid() && ns.valid && !name.empty() && ns.findObject(name).name == 0
        /// \post return.valid()
        CommonObject clone(ConstCommonObject obj, Namespace ns, const string& name);

        /// get the heap
        heap& getHeap() const;

        /// get the ext_heap
        ext_heap& getExtHeap() const;
    private:
        heap* m_alloc;
        ext_heap* m_ext_alloc;
	};


    /// functor used to destroy object
    class ObjectDeletor
	{
    public:
        /// ctor
        explicit ObjectDeletor(heap & al) ;

        /// destroy the given object
        /// \pre obj.valid()
        /// \throw nothrow
        void destroy(CommonObject obj) const;

        /// destroy the given object
        /// \pre obj.valid()
        /// \throw nothrow
        void operator()(CommonObject obj) const;

        /// destroy the given object and remove it from namespace
        /// \pre obj.valid() && *ns.findObject(name).second == obj
        /// \throw nothrow
        void destroy(CommonObject obj,  Namespace ns, const string& name) const;

        /// destroy the given object and remove it from namespace
        /// \pre obj.valid() && *ns.findObject(name).second == obj
        /// \throw nothrow
        void operator()(CommonObject obj,  Namespace ns, const string& name) const;

        heap& getHeap() const;
    private:
        heap* m_alloc;
	};


    /// an auto ptr used to handle object blob during creation.
    /// it's used to write exception safety code
    class UninitObjectPtr
    {
    public:

        /// ctor
        /// \pre t.valid()
        UninitObjectPtr(Type t, heap& al);

        /// free the allocated memory
        /// if enableDtor(), it'll destruct the object at the memory.
        ~UninitObjectPtr();

        /// release the ownership of the memory
        void* release();

        /// get the memory address
        void* get() const;

        /// allocate a new memory block
        /// \pre !get() && !enableDtor()
        void reset();

        /// query dtor enable flag,
        bool dtorEnabled() const;

        /// set dtor enable flag.
        /// \pre !dtorEnabled()
        /// \see ~UninitObjectPtr()
        void enableDtor();
    private:
        UninitObjectPtr(const UninitObjectPtr&) /* = delete*/;
        UninitObjectPtr& operator=(const UninitObjectPtr&)/* = delete*/;

        Type m_type;
        heap& m_al;
        void* m_data;
        bool m_dtor_enabled;
    };

    /// an auto ptr used to handle object during creation.
    /// it's used to write exception safety code
    class ScopedObjectCreator
    {
    public:
        /// move construct
        ScopedObjectCreator(ScopedObjectCreator&& rhs);

        /// move assignment
        ScopedObjectCreator& operator=(ScopedObjectCreator&& rhs);

        ///ctor
        ScopedObjectCreator(Type t, const Xirang& xi);

        /// ctor
        /// \pre t.valid()
        ScopedObjectCreator(Type t, heap & al, ext_heap& eh);

        /// free the allocated object
        ~ScopedObjectCreator();

        /// release the ownership of the object
        CommonObject release();

        /// clean the internal object
        /// \post return.valid()
        CommonObject reset();

        /// clean the internal object
        /// \post return.valid()
        CommonObject reset(Type t);

        CommonObject adoptBy(Namespace ns, const string& name);

        CommonObject adoptBy(Namespace ns, const string& name, const string& version);

        /// get the memory address
        CommonObject get() const;

        heap& getHeap() const;
        ext_heap& getExtHeap() const;
        void swap(ScopedObjectCreator& rhs);

    private:
        ScopedObjectCreator(const ScopedObjectCreator&) /* = delete*/;
        ScopedObjectCreator& operator=(const ScopedObjectCreator&)/* = delete*/;

        Type m_type;
        heap* m_al;
        ext_heap* m_ext_al;
        void* m_data;
    };
}}
#endif				//end XIRANG_OBJECT_H
