//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_MEMORY_H
#define AIO_MEMORY_H
#include <xirang/config.h>

#include <xirang/macro_helper.h>
#include <xirang/assert.h>

//STD
#include <cstddef>
#include <memory>
#include <xirang/backward/unique_ptr.h>

#include <xirang/config/abi_prefix.h>
namespace xirang
{
	/// helper class. it intends to init the global memory handler only once.
	/// it guarantees that the global memory handle will be inited before using
	/// and before the calling of main, so there are no thread safe guarantee.
	/// anyway, user should not start a thread before main function.
	struct global_heap_init_once;

	struct AIO_INTERFACE heap
	{
		/// allocate a memory blcok with specified size
		/// \param size bytes of required block
		/// \param alignment the alignment of allocated memory block. Default is 1, alignment must be 2^N 
		/// \param hint hint of allocation, it's used to improve the performance for some implementation. null means no hint.
		/// \return head address of allocated memory blcok
		/// \pre size >= 0
		/// \post return != 0
		/// \throw std::bad_alloc
		virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint ) = 0;

		/// free the memory block allocated by current handler instance or equivalent
		/// \param p head address of memory block
		/// \param alignment the alignment of allocated memory block. Default is 1, alignment must be 2^N, 0 means unknown
		/// \param size size of memory block. if size equals to 0, means unknown size and heap imp should check the size. The check may be slow.
		/// \pre size and alignment should match the malloc call or 0, and p should belong to this heap
		/// \throw nothrow
		/// \note if p is null, do nothing.
		virtual void free(void* p, std::size_t size, std::size_t alignment ) = 0;

		/// query the underling heap
		/// \return return the underling heap, can be null.
		virtual heap* underling() = 0;

		/// return true if two memory handlers are equivalent
		/// \param rhs the other memory handler reference
		/// \return true if equal otherwise false
		/// \throw nothrow
		virtual bool equal_to(const heap& rhs) const = 0;

		protected:
		virtual ~heap();
	};

	template<typename T> T* malloc_obj(heap& h){
		return h.malloc(sizeof(T), alignof(T), 0);
	}
	template<typename T> void free_obj(heap& h, T* p){
		h.free(p, sizeof(T), alignof(T));
	}

	template<typename T>
	struct heap_deletor
	{
		explicit heap_deletor(heap& h) :m_heap(&h){}
		void operator()(T* p) const
		{
			m_heap->free(p, sizeof(T), std::alignment_of<T>::value);
		}
		private:
		heap* m_heap;
	};

	template<typename T>
	struct uninitialized_heap_ptr
	{
		explicit uninitialized_heap_ptr(heap& h) 
			: m_p(malloc_obj<T>(h))
			, m_heap(&h)
		{}
		explicit uninitialized_heap_ptr(T* p, heap& h) : m_p(p), m_heap(&h){}
		~uninitialized_heap_ptr(){
			if (m_p) free_obj(*m_heap, m_p);
		}

		T* release()
		{
			T* ret = m_p;
			m_p = 0;
			return ret;
		}
		T* get() const { return m_p;}
		private:

		T* m_p;
		heap* m_heap;
	};

	template<typename T>
	struct heap_creator
	{
		typedef xirang::unique_ptr<T, heap_deletor<T> > pointer_type;
		explicit heap_creator(heap& h) : m_heap(&h){}
		template<typename ... Args>
		pointer_type create(Args && ... args)
		{
			uninitialized_heap_ptr<T> p (*m_heap);
			new (p.get()) T(args ...);
			return pointer_type(p.release(), heap_deletor<T>(*m_heap));
		}
		private:
		heap* m_heap;
	};
    
    struct AIO_INTERFACE ext_heap;

	namespace memory{

		/// return the max necessary alignment.
		AIO_COMM_API extern std::size_t max_alignment();

		/// return the global heap.
		AIO_COMM_API extern heap& get_global_heap();

        AIO_COMM_API extern ext_heap& get_global_ext_heap();

		/// set the global heap.
		AIO_COMM_API extern void set_global_heap(heap& newHeap);

		AIO_COMM_API void init_global_heap_once();

		/// define the thread poliy enumeration
		enum thread_policy
		{
			single_thread,
			multi_thread
		};
	}

	/// force to init global memory handler
	struct global_heap_init_once
	{
		global_heap_init_once() {memory::init_global_heap_once();}
	};

	namespace 
	{
		/// each compile units contains this head file will create a local object, so it'll init the global memory handler
		global_heap_init_once g_global_heap_init_once;
	}

	/// helper class, its used to save current global memory handler and restore it before leaving scope
	class heap_saver
	{
	public:
		///\ctor save current global memory handler
		heap_saver() : m_saved(memory::get_global_heap()){}

		///\ctor save current global memory handler and set with the new_handler
		///\param new_handler
		explicit heap_saver(heap& new_handler) 
			: m_saved(memory::get_global_heap())
		{
			memory::set_global_heap(new_handler);
		}

		///\dtor restor the current global memory handler with saved value.
		~heap_saver() {memory::set_global_heap(m_saved);}
	private:
		heap& m_saved;
	};

	typedef unsigned long long long_size_t;
	typedef long long long_offset_t;
	struct offset_range {
		long_offset_t begin() const;
		long_offset_t end() const;

		/// the default begin and end are zero
		offset_range();
		/// \pre b <= e
		explicit offset_range(long_offset_t b, long_offset_t e);

		/// \return end - begin
		long_size_t size() const;

		/// \return end == begin
		bool empty() const;

		/// \return !empty()
		explicit operator bool() const;

		bool in(const offset_range& rhs) const;
		bool contains(const offset_range& rhs) const;
		private:
		long_offset_t m_begin, m_end;
	};

	struct AIO_INTERFACE ext_heap : public heap
	{
	public: //heap methods
		typedef offset_range handle;
		virtual void* malloc(std::size_t size, std::size_t alignment, const void* hint ) = 0;

		virtual void free(void* p, std::size_t size, std::size_t alignment ) = 0;

		/// \return the underling ext_heap, can be null.
		virtual heap* underling() = 0;

		virtual bool equal_to(const heap& rhs) const = 0;

	public:

		/// allocate a block in external heap
		virtual handle allocate(std::size_t size, std::size_t alignment, handle hint) = 0;

		/// release an external block
		virtual void deallocate(handle p) = 0;

		/// map a block into memory, ref count internally.
		virtual void* track_pin(handle h) = 0;
		/// map a block into memory, just track the view, not handle
		virtual void* pin(handle h) = 0;

		virtual int track_pin_count(handle h) const= 0;
		virtual int view_pin_count(handle h) const= 0;

		/// unmap a block. 
		virtual int track_unpin(void* h) = 0;
		/// unmap a block. 
		virtual int unpin(void* h) = 0;

		/// TODO: is it necessary?
		/// write to external block directly. if h have been mapped into memory, update the memory.
		virtual std::size_t write(handle h, const void* src, std::size_t n) = 0;

		/// TODO: is it necessary?
		/// read from external block directly. if the block has been mapped, read the memory block.
		virtual std::size_t read(handle, void* dest, std::size_t) = 0;

		/// sync the memory to external, if h is invalid, sync all. 
		virtual void sync(handle h) = 0;

	protected:
		virtual ~ext_heap();
	};

	inline bool operator ==(ext_heap::handle lhs, ext_heap::handle rhs) 
	{
		return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
	}

	inline bool operator <(ext_heap::handle lhs, ext_heap::handle rhs) 
	{
		return lhs.begin() < rhs.begin() 
			|| (lhs.begin() == rhs.begin() && lhs.end() < rhs.end());
	}

	/// this allocator intends to be used crossing module. see std::allocator interface
	template<typename T>
	class abi_allocator
	{
	public:

		typedef T value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef const value_type* const_pointer;
		typedef const value_type& const_reference;

		typedef std::size_t		size_type;
		typedef std::ptrdiff_t  difference_type;

		template<typename Other>
		struct rebind
		{
			typedef abi_allocator<Other> other;
		};

		pointer address(reference val) const{ return (&val); }

		const_pointer address(const_reference val) const{ return (&val); }

		abi_allocator() : m_handle(&memory::get_global_heap()){}

		explicit abi_allocator(heap& h) : m_handle(&h){}

		template<class Other>
		abi_allocator(const abi_allocator<Other>& rhs) 
			: m_handle(rhs.m_handle)
		{
			AIO_PRE_CONDITION(rhs.m_handle != 0);
		}

		template<class Other>
		abi_allocator& operator=(const abi_allocator<Other>& rhs)
		{
			AIO_PRE_CONDITION(rhs.m_handle != 0);
			m_handle = rhs.m_handle;
			return (*this);
		}

		void deallocate(pointer p, size_type n)
		{				
			get_heap().free(p, n * sizeof(value_type), 0);
		}

		pointer allocate(size_type n, const void * hint = 0)
		{	
			return reinterpret_cast<pointer>( get_heap().malloc(n * sizeof (value_type), alignof(T), hint));
		}

		void construct(pointer p, const_reference val)
		{
			new (p) value_type(val);
		}

		void destroy(pointer p)
		{	
			(p)->~T();
		}

		size_type max_size() const 
		{
			size_type n = (size_type)(-1) / sizeof (value_type);
			return (0 < n ? n: 1);
		}

		/// get holded memory handler
		/// \return holded handler
		heap& get_heap() const { 
			AIO_PRE_CONDITION(m_handle != 0);
			return *m_handle;
		}

	private:
		template<typename Other>
		friend class abi_allocator;

		heap* m_handle;
	};


	template<>
	class abi_allocator<void>
	{
	public:
		typedef void *pointer;
		typedef const void *const_pointer;
		typedef void value_type;

		template<class Other>
		struct rebind
		{	
			typedef abi_allocator<Other> other;
		};

		abi_allocator() : m_handle(&memory::get_global_heap()){}

		explicit abi_allocator(heap& h) : m_handle(&h){}

		template<class Other>
		abi_allocator(const abi_allocator<Other>& rhs)
			: m_handle(rhs.m_handle)
		{ AIO_PRE_CONDITION(rhs.m_handle != 0);}

		template<class Other>
		abi_allocator& operator=(const abi_allocator<Other>& rhs)
		{
			AIO_PRE_CONDITION(rhs.m_handle != 0);
			m_handle = rhs.handle;
			return (*this);
		}

		/// get holded memory handler
		/// \return holded handler
		heap& get_heap() const { return *m_handle;}

	private:
		template<typename Other>
		friend class abi_allocator;

		heap* m_handle;
	};

	template<class T,class U> 
	inline bool operator==(const abi_allocator<T>& lhs, const abi_allocator<U>& rhs)
	{
		return lhs.get_heap().equal_to(rhs.get_heap());
	}

	template<class T,class U> 
	inline bool operator !=(const abi_allocator<T>& lhs, const abi_allocator<U>& rhs)
	{
		return !(lhs == rhs);
	}


	/**
	destruct policy
	@param T value type
	*/
	class simple_destruct
	{
	public:		
		/**
		destruct pointee object.
		@pre pt is allocated by alloc
		@pre pt != 0
		@pre pt pointer to object which was constructed
		@param alloc allocator
		@param pt pointer to the object allocated by alloc, can't be null
		@throw nothrow
		*/
		template<template <typename> class Alloc, typename T>
		void operator()(Alloc<T>& alloc, T* pt) const
		{
			AIO_PRE_CONDITION(pt != 0);
			alloc.destroy(pt);
		}
	};

	/// this class intends to provide thread safe of array initialization.
	/// 
	template
		<
		typename T,
		typename DestroyPolicy = simple_destruct,
		template <class> class Alloc = abi_allocator
		>
	class mem_multiple_initilizer
	{
		DISABLE_CLONE(mem_multiple_initilizer);
	public:
		typedef T element_type;			///< value type
		typedef Alloc<T> allocator_type;		///< allocator type		
		typedef typename allocator_type::pointer pointer;				///< pointer to value type
		typedef typename allocator_type::const_pointer const_pointer;	///< const pointer to value type
		typedef typename allocator_type::reference reference;			///< reference to value type
		typedef typename allocator_type::const_reference const_reference;	///< const reference to value type
		typedef DestroyPolicy	destruct_policy;
		typedef std::size_t size_type;

		typedef mem_multiple_initilizer<T, DestroyPolicy, Alloc> self_type;

		/** \ctor it accepts head pointer of an uninitialized array and allocator objects reference.
			\param al reference of allocator
			\param p head pointer of an uninitialized array.
			\pre p != 0
			@throw	nothrow
		*/
		explicit mem_multiple_initilizer(allocator_type& al, pointer p, const destruct_policy& dsp = destruct_policy())
			: m_alloc(al), m_pt(p), m_size(0), m_destructor(dsp)
		{
			AIO_PRE_CONDITION(p != 0);
		}


		/// if not committed, it'll destruct all initialized elements.
		/// if all seccess before leaving scope. user should call commit().
		/// \see commit
		~mem_multiple_initilizer()
		{
			if (m_size > 0)
				destroy_();
		}

		/// \return head pointer of the hold array
		/// \throw noyhrow
		pointer get() const
		{
			return m_pt;
		}

		/// if all secceeded before leaving scope, user should call this method to commit changes, 
		/// otherwise, all initialized elements will be destructed
		/// \return head pointer of the holded array
		/// \post committed() is true
		/// \throw nothrow
		pointer commit()
		{
			AIO_PRE_CONDITION(!committed());
			pointer tmp = m_pt;
			m_pt = 0;
			m_size = 0;
			return tmp;
		}

		/// increase initialized elements counter
		/// \pre !committed()
		mem_multiple_initilizer& operator++() { 
			AIO_PRE_CONDITION(m_pt != 0);
			++m_size;
			return *this;
		}

		/// increase initialized elements counter
		/// \pre !committed()
		mem_multiple_initilizer& operator++(int) { 
			AIO_PRE_CONDITION(m_pt != 0);
			++m_size;
			return *this;
		}

		/// decrease initialized elements counter
		/// \pre !committed() && size() > 0
		mem_multiple_initilizer& operator--() { 
			AIO_PRE_CONDITION(m_pt != 0);
			AIO_PRE_CONDITION(m_size > 0);
			--m_size;
			return *this;
		}

		/// decrease initialized elements counter
		/// \pre !committed() && size() > 0
		mem_multiple_initilizer& operator--(int) { 
			AIO_PRE_CONDITION(m_pt != 0);
			AIO_PRE_CONDITION(m_size > 0);
			--m_size;
			return *this;
		}

		///\return true if committed, otherwise false.
		///\throw nothrow
		bool committed() const { return m_pt == 0;}

		///\return number of initialized elements
		///\throw nothrow
		size_type size() const { return m_size;}

	private:
		// nothrow
		void destroy_()
		{
			AIO_PRE_CONDITION(m_size > 0);
			for (pointer p = m_pt + m_size - 1; p >= m_pt ; --p)
			{
				m_destructor(m_alloc, p);
			}
			m_pt = 0;
			m_size = 0;
		}

		allocator_type m_alloc;
		T* m_pt;
		size_type	m_size;
		destruct_policy m_destructor;
	};

	/// this class is used to hold uninitialize objects for thread safe
	template
		<
		typename T,
		template <class> class Alloc = abi_allocator
		>
	class uninitialized_allocator_ptr 
	{
		DISABLE_CLONE(uninitialized_allocator_ptr);

	public:

		typedef T element_type;			///< value type
		typedef Alloc<T> allocator_type;		///< allocator type		
		typedef typename allocator_type::pointer pointer;				///< pointer to value type
		typedef typename allocator_type::const_pointer const_pointer;	///< const pointer to value type
		typedef typename allocator_type::reference reference;			///< reference to value type
		typedef typename allocator_type::const_reference const_reference;	///< const reference to value type
		typedef std::size_t size_type;

		typedef uninitialized_allocator_ptr<T, Alloc> self_type;

		/**
		  \param al bounded allocator
		@throw	nothrow
		*/
		explicit uninitialized_allocator_ptr(allocator_type al)
			: m_pt(0), m_alloc(al), m_size(0)
		{
		}

		///\dtor deallocate holded uninitialized object
		~uninitialized_allocator_ptr(){
			deallocate();
		}

		/** deallocate holded unintialized object
		  @throw nothrow */
		void deallocate(){
			if (!is_null())
			{
				m_alloc.deallocate(m_pt, m_size);
				m_pt = 0;
				m_size = 0;
			}
		}

		/// \pre !is_null() && size > 0
		/// \post return != 0 
		/// \throw std::bad_alloc
		pointer allocate(size_type size = 1, const void* hint = 0){
			AIO_PRE_CONDITION(is_null());
			AIO_PRE_CONDITION(size > 0);
			m_pt = m_alloc.allocate(size, hint);
			m_size = size;
			return m_pt;
		}


		/**
		@pre !is_null()
		@post is_null()
		@note release holded pointer
		@return pointer to object
		@throw nothrow
		*/
		pointer release()
		{
			AIO_PRE_CONDITION(!is_null());
			pointer tmp = m_pt;
			m_pt = 0;
			m_size = 0;
			return tmp;
		}
		/**
		@return pointer to object
		@throw nothrow
		*/
		pointer get() const
		{
			return m_pt;
		}

		size_type size() const { return m_size;}

		bool is_null() { return m_pt == 0;}

		/**
		swap two allocator_ptr objects
		@param rsh another allocator_ptr object
		@throw nothrow
		*/
		void swap(self_type& rsh)
		{
			using std::swap;
			swap(m_alloc, rsh.m_alloc);
			swap(m_pt, rsh.m_pt);
			swap(m_size, rsh.m_size);
		}
		/**
		global swap function
		@param lsh
		@param rsh
		@throw nothrow
		*/
		inline friend void swap(self_type& lsh, self_type& rsh)
		{
			lsh.swap(rsh);
		}

		allocator_type get_allocator() const { return m_alloc;}
	private:
		T* m_pt;
		allocator_type m_alloc;
		size_type	m_size;
	};

	/**	
	allocator_ptr is a RAII pointer, help for allocator
	@param T value type
	@param Alloc allocator type
	@param DestroyPolicy destroy policy
	*/
	template
		<
		typename T, 		
		typename DestructPolicy = simple_destruct,
		template <class> class Alloc = abi_allocator
		>
	class allocator_ptr 
	{
		DISABLE_CLONE(allocator_ptr);
	public:
		typedef T element_type;			///< value type
		typedef Alloc<T> allocator_type;		///< allocator type		
		typedef typename allocator_type::pointer pointer;				///< pointer to value type
		typedef typename allocator_type::const_pointer const_pointer;	///< const pointer to value type
		typedef typename allocator_type::reference reference;			///< reference to value type
		typedef typename allocator_type::const_reference const_reference;	///< const reference to value type
		typedef std::size_t size_type;

		typedef allocator_ptr self_type;	///< allocator_ptr self type
		typedef DestructPolicy destruct_policy;	///< destroy policy
		typedef mem_multiple_initilizer<T, destruct_policy, Alloc> initializer_type;

		/**
		@construct
		@param al reference of allocator object
		@param pt pointer to object allocated by al
		@throw any
		*/		
		explicit allocator_ptr(uninitialized_allocator_ptr<T, Alloc>& init_ptr
			, const_reference var, destruct_policy dtor = destruct_policy())
			: m_alloc(init_ptr.get_allocator()), m_pt(init_ptr.get()), m_dtor(dtor), m_size(init_ptr.size())
		{
			if (m_pt != 0)
			{
				AIO_PRE_CONDITION(m_size > 0);

				initializer_type rb(m_alloc, m_pt);
				pointer p = m_pt;
				for (size_type i = 0; i < m_size; ++p, ++i, ++rb)
					m_alloc.construct(p, var);
				rb.release();
				init_ptr.release();
			}
		}

		/**
		@construct
		@param al reference of allocator object
		@param pt pointer to object allocated by al
		@throw any
		*/		
		explicit allocator_ptr(uninitialized_allocator_ptr<T, Alloc>& init_ptr, destruct_policy dtor = destruct_policy())
			: m_alloc(init_ptr.get_allocator()), m_pt(init_ptr.get()), m_dtor(dtor), m_size(init_ptr.size())
		{
			if (m_pt != 0)
			{
				AIO_PRE_CONDITION(m_size > 0);

				initializer_type rb(m_alloc, m_pt);
				pointer p = m_pt;
				element_type var;
				for (size_type i = 0; i < m_size; ++p, ++i, ++rb)
					m_alloc.construct(p, var);
				rb.commit();
				init_ptr.release();
			}
		}

		/**
		@construct
		@param al reference of allocator object
		@param pt pointer to object allocated by al
		@throw any
		*/		
		template<typename InputIterator>
		allocator_ptr(uninitialized_allocator_ptr<T, Alloc>& init_ptr, InputIterator itr, destruct_policy dtor = destruct_policy())
			: m_alloc(init_ptr.get_allocator()), m_pt(init_ptr.get()), m_dtor(dtor), m_size(init_ptr.size())
		{
			if (m_pt != 0)
			{
				AIO_PRE_CONDITION(m_size > 0);

				initializer_type rb(m_alloc, m_pt);
				pointer p = m_pt;
				for (size_type i = 0; i < m_size; ++p, ++i, ++rb, ++itr)
					m_alloc.construct(p, *itr);
				rb.release();
				init_ptr.release();
			}
		}

		explicit allocator_ptr(const allocator_type& alloc, destruct_policy dtor = destruct_policy())
			: m_alloc(alloc), m_pt(0), m_dtor(dtor), m_size(0)
		{
		}
		/**
		call destroy method which is implemented in destroy_policy
		*/
		~allocator_ptr()
		{
			reset_();
		}

		/** @return allocator
		@throw nothrow
		*/
		allocator_type get_allocator() const
		{
			return m_alloc;
		}

		/**
		@pre get() != 0
		@post get() == 0
		@note release holded pointer
		@return pointer to object
		@throw nothrow
		*/
		pointer release()
		{
			AIO_PRE_CONDITION(get() != 0);
			pointer tmp = m_pt;
			m_pt = 0;
			m_size = 0;
			return tmp;
		}
		/**
		@return pointer to object
		@throw nothrow
		*/
		pointer get() const
		{
			return m_pt;
		}

		bool is_null() const {
			return m_pt == 0;
		}

		size_type size() const
		{
			return m_size;
		}

		/**
		swap two allocator_ptr objects
		@param rsh another allocator_ptr object
		@throw nothrow
		*/
		void swap(self_type& rhs)
		{
			using std::swap;
			swap(m_alloc, rhs.m_alloc);
			swap(m_pt, rhs.m_pt);
			swap(m_dtor, rhs.m_dtor);
			swap(m_size, rhs.m_size);
		}
		/**
		global swap function
		@param lsh
		@param rsh
		@throw nothrow
		*/
		inline friend void swap(self_type& lsh, self_type& rsh)
		{
			lsh.swap(rsh);
		}
		/**
		member operator of object pointer
		@pre get() != 0;
		@return pointer to object
		@see operator*
		@throw nothrow
		*/
		pointer operator->() const
		{
			AIO_PRE_CONDITION(get() != 0);
			return get();
		}

		reference operator[](size_type index) const
		{
			AIO_PRE_CONDITION(get() != 0 && index < size());
			return get()[index];
		}

		/**
		dereference pointer
		@pre get() != 0;
		@return reference to object
		@see operator->
		@throw nothrow
		*/
		reference operator*() const
		{
			AIO_PRE_CONDITION(get() != 0);
			return *get();
		}

		void reset(pointer p = 0, size_type s = 1)
		{
			if (p != m_pt && m_pt != 0)
				reset_();
			m_pt = p;
			m_size = s;
		}

		void reset(uninitialized_allocator_ptr<T, Alloc>& init_ptr
			, const_reference var)
		{
			allocator_ptr(init_ptr, var, m_dtor).swap(*this);
		}

		void reset(uninitialized_allocator_ptr<T, Alloc>& init_ptr)
		{
			allocator_ptr(init_ptr, m_dtor).swap(*this);
		}

		template<typename InputIterator>
		void reset(uninitialized_allocator_ptr<T, Alloc>& init_ptr, InputIterator itr)
		{
			allocator_ptr(init_ptr, itr, m_dtor).swap(*this);
		}

	private:	///< disable copy constructor and assignment
		allocator_ptr* operator&();
		const allocator_ptr* operator&() const;

	private:
		/** set new pointer
		@param pt pointer to object which is allocated by allocator or be null
		@post get() == pt;
		@throw nothrow
		*/
		void reset_()
		{
			if (get() != 0 )
			{
				AIO_PRE_CONDITION(m_size > 0);
				for (pointer p = m_pt + m_size - 1; p >= m_pt; --p)
				{
					m_dtor(m_alloc, p);
				}
				m_alloc.deallocate(m_pt, m_size);
				m_pt = 0;
				m_size = 0;
			}
		}


		allocator_type		m_alloc;
		pointer     m_pt;
		destruct_policy  m_dtor;		
		size_type	m_size;
	};

}

#include <xirang/config/abi_suffix.h>

#endif //end AIO_STRING_H

