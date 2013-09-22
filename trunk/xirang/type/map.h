#ifndef XIRANG_MAP_H
#define XIRANG_MAP_H

#include <xirang/type.h>
#include <xirang/object.h>
#include <xirang/typebinder.h>

//STL
#include <iterator>

namespace xirang
{
	class MapImp;
	class MapIteratorImp;
	class Map
	{
	public:
		struct const_iterator : public std::iterator<std::random_access_iterator_tag, ConstCommonObject>
		{
			typedef std::iterator<std::random_access_iterator_tag, ConstCommonObject> base_type;
			typedef base_type::iterator_category iterator_category;
			typedef base_type::value_type value_type;
			typedef base_type::difference_type difference_type;
			typedef value_type pointer;
			typedef value_type reference;

			const_iterator();

			const_iterator(MapIteratorImp* imp);

			const_iterator(const const_iterator& rhs );
			~const_iterator();

			value_type first() const;
			value_type second() const;

			const_iterator& operator++();
			const_iterator operator++(int);

			const_iterator& operator--();
			const_iterator operator--(int);

			bool operator==(const const_iterator& rhs) const;
			bool operator!=(const const_iterator& rhs) const;

			bool valid() const;

		protected:
			MapIteratorImp* m_imp;
			friend class MapImp;
			friend class MapIteratorImp;
		};

		struct iterator : public const_iterator
		{
			typedef std::iterator<std::random_access_iterator_tag, CommonObject> base_type;
			typedef base_type::iterator_category iterator_category;
			typedef base_type::value_type value_type;
			typedef base_type::difference_type difference_type;
			typedef value_type pointer;
			typedef value_type reference;

			iterator();

			iterator(MapIteratorImp* );

			value_type first() const;
			value_type second() const;
			iterator& operator++();
			iterator operator++(int);
			iterator& operator--();
			iterator operator--(int);
		};

		Map();
		Map(heap& h, ext_heap& eh, Type key, Type value);

		Map(const Map& other);

		~Map();
		
		void assign(const Map& other);
		Map& operator=(const Map& other);

		void swap(Map& other);

		bool valid() const;

		Type keyType() const;
		Type valueType() const;

		heap& get_heap() const;
		ext_heap& get_ext_heap() const;

		std::size_t size() const;
		bool empty() const;

		const_iterator begin() const;
		iterator begin() ;
		const_iterator end() const;
		iterator end() ;

		iterator find(ConstCommonObject key);
		const_iterator find(ConstCommonObject key) const;

		iterator lower_bound(ConstCommonObject key);
		const_iterator lower_bound(ConstCommonObject key) const;
		iterator upper_bound(ConstCommonObject key);
		const_iterator upper_bound(ConstCommonObject key) const;


		CommonObject operator[](ConstCommonObject key) ;

		void insert(ConstCommonObject k, ConstCommonObject v);
		iterator erase(const iterator& pos);

		void clear();
	private:
		MapImp* m_imp;
	};

	//bool operator==(const Map& lhs, const Map& rhs); 
	//bool operator<(const Map& lhs, const Map& rhs); 
}

#endif //end XIRANG_MAP_H

