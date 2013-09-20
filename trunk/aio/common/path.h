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
		pp_default = 3,
		pp_winfile = 4,
		pp_localfile = 7,	//pp_default | pp_winfile

	};

	class sub_file_path : totally_ordered<sub_file_path>
	{
		public:
			static const char dim;
			sub_file_path();
			explicit sub_file_path(string::const_iterator first,
					string::const_iterator last);
			sub_file_path& operator=(const sub_file_path& rhs);
			void swap(sub_file_path& rhs);

			sub_file_path parent() const;
			sub_file_path ext() const;
			sub_file_path stem() const;
			sub_file_path filename() const;

			bool is_absolute() const;
			bool is_network() const;
			bool is_root() const;
			bool is_normalized() const;
			bool empty() const;

			const_range_string str() const;
			aio::string native_str() const;
			aio::wstring native_wstr() const;
			aio::wstring wstr() const;

			class iterator;
			typedef iterator const_iterator;

			iterator begin() const;
			iterator end() const;
		private:
			const_range_string m_str;
	};

	inline bool operator<(const sub_file_path& lhs, const sub_file_path& rhs){
		return lhs.str() < rhs.str();
	}
	inline bool operator==(const sub_file_path& lhs, const sub_file_path& rhs){
		return lhs.str() == rhs.str();
	}

	// a/b ==> a : b
	// /a/b ==> / : a : b
	// //a/b ==> //a : / : b
	// ///a/b ==> / : a : b
	class sub_file_path::iterator{
		public:
			typedef std::bidirectional_iterator_tag iterator_category;
			typedef sub_file_path 	value_type;
			typedef std::ptrdiff_t	difference_type;
			typedef const sub_file_path*	pointer;
			typedef const sub_file_path&	reference;

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
			sub_file_path path_() const;
			iterator(const_range_string path, string::const_iterator pos);

			friend class sub_file_path;
			string::const_iterator m_pos;
			const_range_string m_path;
			mutable sub_file_path m_cache;
	};

	class file_path : totally_ordered<file_path>
	{
		public:
			static const char dim;
			typedef sub_file_path::iterator iterator;
			typedef sub_file_path::const_iterator const_iterator;

			file_path();
			file_path(const sub_file_path& str);

			explicit file_path(const string& str, path_process pp = pp_default);
			explicit file_path(const wstring& str, path_process pp = pp_default);
			file_path(const file_path& rhs);
			file_path(file_path&& rhs);

			file_path& operator=(const file_path& rhs);
			file_path& operator=(file_path&& rhs);

			sub_file_path parent() const;
			sub_file_path ext() const;
			sub_file_path stem() const;
			sub_file_path filename() const;
			bool is_absolute() const;
			bool is_network() const;
			bool is_root() const;
			bool is_normalized() const;
			bool empty() const;


			file_path& normalize(path_process pp = pp_default);

			file_path& operator/=(const file_path& rhs);
			file_path& replace_parent(const file_path& rhs);
			file_path& replace_ext(const file_path& rhs);
			file_path& replace_stem(const file_path& rhs);
			file_path& replace_filename(const file_path& rhs);
			file_path& to_absolute();
			file_path& remove_absolute();

			void swap(file_path& rhs);

			const aio::string& str() const;

			aio::string native_str() const;
			aio::wstring native_wstr() const;
			aio::wstring wstr() const;

			operator sub_file_path() const;
			sub_file_path as_sub_path() const;

			iterator begin() const;
			iterator end() const;

		private:
			string m_str;
	};

	inline bool operator<(const file_path& lhs, const file_path& rhs){
		return lhs.str() < rhs.str();
	}
	inline bool operator==(const file_path& lhs, const file_path& rhs){
		return lhs.str() == rhs.str();
	}

	file_path operator/(const file_path& lhs, const file_path& rhs);



	class simple_path : totally_ordered<simple_path>
	{
		public:
			static const char dim;
			simple_path();
			explicit simple_path(const string& str, path_process pp = pp_utf8check);
			simple_path(const simple_path& rhs);
			simple_path(simple_path&& rhs);

			simple_path& operator=(const simple_path& rhs);
			simple_path& operator=(simple_path&& rhs);

			simple_path parent() const;
			simple_path filename() const;
			bool is_absolute() const;
			bool is_root() const;
			bool empty() const;

			simple_path& operator/=(const simple_path& rhs);
			simple_path& replace_parent(const simple_path& rhs);
			simple_path& replace_filename(const simple_path& rhs);
			simple_path& to_absolute();
			simple_path& remove_absolute();

			void swap(simple_path& rhs);

			const aio::string& str() const;

			class iterator;
			typedef iterator const_iterator;

			iterator begin() const;
			iterator end() const;

		private:
			string m_str;
	};

	inline bool operator<(const simple_path& lhs, const simple_path& rhs){
		return lhs.str() < rhs.str();
	}
	inline bool operator==(const simple_path& lhs, const simple_path& rhs){
		return lhs.str() == rhs.str();
	}

	simple_path operator/(const simple_path& lhs, const simple_path& rhs);

	// a/b ==> a : b
	// /a/b ==> / : a : b
	// //a/b ==> //a : / : b
	// ///a/b ==> / : a : b
	class simple_path::iterator{
		public:
			typedef std::bidirectional_iterator_tag iterator_category;
			typedef simple_path 	value_type;
			typedef std::ptrdiff_t	difference_type;
			typedef const simple_path*	pointer;
			typedef const simple_path&	reference;

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
			iterator(const simple_path& simple_path, string::const_iterator pos);
			friend class simple_path;
			string::const_iterator m_pos;
			const simple_path* m_path;
			mutable simple_path m_cache;
	};
}

#endif //end AIO_COMMON_PATH_H_
