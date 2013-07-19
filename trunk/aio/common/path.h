#ifndef AIO_COMMON_PATH_H_
#define AIO_COMMON_PATH_H_
#include <aio/common/string.h>
namespace aio{

	enum path_process{
		pp_none,
		pp_normalize
	};
	class path{
		public:
			explicit path(const string& str, path_process pp = pp_normalize);
			explicit path(const char* str, path_process pp = pp_normalize);
			explicit path(const wchar_t* str, path_process pp = pp_normalize);
			path(const path& rhs);
			path(path&& rhs);

			path& operator=(const path& rhs);
			path& operator=(path&& rhs);


			path parent() const;
			path ext() const;
			path stem() const;
			path filename() const;
			bool is_absolute() const;
			

			path& normalize();


			path& operator+=(const path& rhs);
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
	};
	friend operator/(const path& lhs, const path& rhs);
}

#endif //end AIO_COMMON_PATH_H_
