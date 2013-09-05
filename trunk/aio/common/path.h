#ifndef AIO_COMMON_PATH_H_
#define AIO_COMMON_PATH_H_
#include <aio/common/string.h>
#include <aio/common/string_algo/utf8.h>
#include <aio/common/string_algo/string.h>
#include <aio/common/operators.h>
namespace aio{

	enum path_process{
		pp_none,
		pp_normalize = 1,
		pp_utf8check = 2,
		pp_default = 3
	};
	class path : totally_ordered<path>
	{
		public:
			path();
			explicit path(const string& str, path_process pp = pp_default);
			explicit path(const wstring& str, path_process pp = pp_default);
			path(const path& rhs);
			path(path&& rhs);

			path& operator=(const path& rhs);
			path& operator=(path&& rhs);

			path parent() const;
			path ext() const;
			path stem() const;
			path filename() const;
			bool is_absolute() const;
			bool is_network() const;
			bool is_root() const;
			bool is_normalized() const;
			bool empty() const;


			path& normalize();

			path& operator/=(const path& rhs);
			path& replace_parent(const path& rhs);
			path& replace_ext(const path& rhs);
			path& replace_stem(const path& rhs);
			path& replace_filename(const path& rhs);
			path& to_absolute();
			path& remove_absolute();

			void swap(path& rhs);

			const aio::string& str() const;

			aio::string native_str() const;
			aio::wstring native_wstr() const;
			aio::wstring wstr() const;

			class iterator;
			typedef iterator const_iterator;

			iterator begin() const;
			iterator end() const;

		private:
			string m_str;
	};

	inline bool operator<(const path& lhs, const path& rhs){
		return lhs.str() < rhs.str();
	}
	inline bool operator==(const path& lhs, const path& rhs){
		return lhs.str() == rhs.str();
	}

	path operator/(const path& lhs, const path& rhs);

	//TODO:
	// a/b ==> a : b
	// /a/b ==> / : a : b
	// //a/b ==> //a : / : b
	// ///a/b ==> / : a : b
	class path::iterator{
		public:
			typedef std::bidirectional_iterator_tag iterator_category;
			typedef path 	value_type;
			typedef std::ptrdiff_t	difference_type;
			typedef const path*	pointer;
			typedef const path&	reference;

			iterator();
			void swap(iterator& rhs);

			iterator& operator++();
			iterator& operator--();
			iterator operator++(int);
			iterator operator--(int);

			bool operator==(const iterator& rhs);
			bool operator!=(const iterator& rhs);

			reference operator*() const;
			pointer operator->() const;

		private:
			string::const_iterator begin_() const;
			string::const_iterator end_() const;
			iterator(const path& path, string::const_iterator pos);
			friend class path;
			string::const_iterator m_pos;
			const path* m_path;
			mutable path m_cache;
	};
}

#endif //end AIO_COMMON_PATH_H_
