#include "xirang/fsutility.h"
#include "xirang/to_string.h"
#include "xirang/context_except.h"
#include "xirang/string_algo/utf8.h"
#include "xirang/string_algo/string.h"
#include "xirang/io/file.h"

#include <cstdlib>
#include <vector>
#include <sys/stat.h>

#ifdef MSVC_COMPILER_
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#endif

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <xirang/string_algo/char_separator.h>
#include <random>

namespace xirang {namespace fs{
    namespace{
        const auto onedot = literal(".");
        const auto twodot = literal("..");
    }

    namespace private_{
		struct random_generator{
			random_generator() : m_engin(time(0)){ }
			unsigned long yield(){
				return m_distribution(m_engin);
			}

			private:
			std::mt19937 m_engin;
			std::uniform_int_distribution<unsigned long> m_distribution;

		};
        static string gen_temp_name_()
        {
			static random_generator generator;
            unsigned long x = generator.yield();
            while(x == 0) x = generator.yield();

			string_builder name;
            while (x > 0)
            {
                unsigned long mod = x % 36;
                if (mod < 10)
                    name.push_back(mod + '0');
                else
                    name.push_back(mod - 10 + 'a');

                x /= 36;
            }
			return name.str();
		}

        file_path gen_temp_name(sub_file_path template_)
        {
			if (!contains(template_.str(), '?'))	//back compatibility
				return file_path(string(template_.str() << gen_temp_name_()), pp_none);

			return file_path(replace(template_.str(), literal("?"), gen_temp_name_().range_str()), pp_none);
        }
    }
#ifdef GNUC_COMPILER_
    fs_error move(const file_path& from_, const file_path& to_)
    {
		string from(from_.str());
		string to(to_.str());
		int res = link(from.c_str(), to.c_str());
		if (res == 0){
			unlink(from.c_str());
			return er_ok;
		}

		auto err = errno;
		if(EXDEV == err){
			try{
				copy(from_, to_);
				unlink(from.c_str());
			} catch(...){
				return er_system_error;
			}
			return er_ok;
		}
		switch(err){
			case EFAULT:
			case EACCES: return er_permission_denied;
			case EEXIST: return er_exist;
			default:
						 return er_system_error;
		}
    }

    fs_error remove(const file_path& path_)
    {
		string path(path_.str());
        if (state(path_).state == st_dir)
        {
            return rmdir(path.c_str()) == 0 
                ? er_ok
                : er_system_error;
        }
        else
        {
            return ::remove(path.c_str()) == 0 
                ? er_ok
                : er_system_error;
        }
    }
    fs_error create_dir(const file_path& path)
    {
        if (state(path).state != st_not_found)
            return er_exist;

        return (::mkdir(path.str().c_str(), S_IRWXU|S_IRWXG|S_IRWXO) != 0)
            ? er_system_error
            : er_ok;
    }

    fstate state(const file_path& path)
    {
        fstate fst = {path, st_not_found, 0};
        struct stat st;
        if (stat(path.str().c_str(), &st) == 0)
        {
            if (S_ISREG(st.st_mode))
                fst.state = st_regular;
            else if (S_ISDIR(st.st_mode))
                fst.state = st_dir;
            else if (S_ISLNK(st.st_mode))
                fst.state = st_symbol;
            else if (S_ISSOCK(st.st_mode))
                fst.state = st_socket;
            else if (S_ISFIFO(st.st_mode))
                fst.state = st_pipe;
            else 
                fst.state = st_unknown;

            fst.size = st.st_size;
        }

        return fst;
    }

    fs_error truncate(const file_path& path, long_size_t s)
    {
        int ret = ::truncate(path.str().c_str(), s);
        return ret == 0 ? er_ok : er_system_error;
    }
#elif defined(MSVC_COMPILER_)
    fs_error move(const file_path& from, const file_path& to)
    {
        wstring wfrom = from.native_wstr();
        wstring wto = to.native_wstr();
		return MoveFileW(wfrom.c_str(), wto.c_str()) ? er_ok : er_system_error;

    }

    fs_error remove(const file_path& path)
    {
        wstring wpath = path.native_wstr();
        if (state(path).state == st_dir)
        {
        
            return ::_wrmdir(wpath.c_str()) == 0 
                ? er_ok
                : er_system_error;
        }
        else
        {
            return ::_wremove(wpath.c_str()) == 0 
                ? er_ok
                : er_system_error;
        }
    }

    fs_error create_dir(const file_path& path)
    {
        if (state(path).state != st_not_found)
            return er_exist;

        wstring wpath = path.native_wstr();
        return (_wmkdir(wpath.c_str()) != 0)
            ? er_system_error
            : er_ok;
    }
    
    fstate state(const file_path& path)
    {   
        // NOTES: on windows, the pure disk letter or network dir MUST endded with '\'.
        // Network file MUST NOT end with '\'
        // A local dir or file path  MUST NOT ended with '\'.
        // I was freaked out!

        fstate fst = {path, st_not_found, 0};
        struct _stat st={};

		wstring wpath = path.native_wstr();	// try without end '/' first
        if (_wstat(wpath.c_str(), &st) == 0)
        {
            if (_S_IFREG & st.st_mode)
                fst.state = st_regular;
            else if (_S_IFDIR & st.st_mode)
                fst.state = st_dir;
            else 
                fst.state = st_unknown;
            fst.size = st.st_size;
        }
        else 
        {
			wpath = wpath << literal("/");	//try with ending '/'
            if (_wstat(wpath.c_str(), &st) == 0)
            {
                if (_S_IFREG & st.st_mode)
                    fst.state = st_regular;
                else if (_S_IFDIR & st.st_mode)
                    fst.state = st_dir;
                else 
                    fst.state = st_unknown;
                fst.size = st.st_size;
            }
        }

        return fst;
    }

    fs_error truncate(const file_path& path, long_size_t s)
    {
        wstring wpath = path.native_wstr();
        HANDLE hFile = CreateFileW(wpath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            return er_system_error;

        SetFilePointer(hFile, (LONG)s, 0, FILE_BEGIN);
        BOOL ret = SetEndOfFile(hFile);
        CloseHandle(hFile);
        return ret ? er_ok : er_system_error;
    }
#endif

    void copy(const file_path& from, const file_path& to)
    {
		io::file_reader src(from);
		io::file_writer dest(to, io::of_create_or_open);
		iref<io::reader> rd(src);
		iref<io::writer> wr(dest);

		long_size_t n = io::copy_data(rd.get<io::reader>(), wr.get<io::writer>());
		if (n != src.size())
			AIO_THROW(file_copy_error);
    }

    class file_iterator
	{
	public:
		file_iterator()
			: m_itr()
		{
        }

		explicit file_iterator(const file_path& rpath)
#ifdef MSVC_COMPILER_
			: m_itr(
					(rpath.is_network() || rpath.is_pure_disk())
					? wstring(rpath.native_wstr() << literal("/")).c_str()
					: rpath.native_wstr().c_str()
					)
#else
			: m_itr(rpath.str().c_str())
#endif
		{ 
        }

		const file_path& operator*() const
		{
            m_path = file_path(m_itr->path().leaf().c_str());
			return m_path;
		}

        const file_path* operator->() const
		{
            return &**this;
		}

		file_iterator& operator++()	{ ++m_itr; return *this;}
		file_iterator operator++(int) { 
			file_iterator ret = *this; 
			++*this; 
			return ret;
		}

		file_iterator& operator--(){ return *this;}
		file_iterator operator--(int){ return *this;}

		bool operator==(const file_iterator& rhs) const
		{
			return m_itr == rhs.m_itr;
		}
	private:
#ifdef MSVC_COMPILER_
		boost::filesystem::wdirectory_iterator m_itr;
#else
		boost::filesystem::directory_iterator m_itr;
#endif
        mutable file_path m_path;
	};

    file_range children(const file_path& path)
    {
        fstate st = state(path);
        return (st.state == st_dir)
            ? file_range(file_iterator(path), file_iterator())
            : file_range();
    }

	io::file temp_file(const file_path& template_/* = "tmpf"*/, int flag /*= io::of_create | io::of_remove_on_close*/, file_path* path/* = 0*/)
    {   
        const char* tmpdir = getenv("TMPDIR");
        if (!tmpdir)
            tmpdir = getenv("TMP");

        string prefix = as_range_string((tmpdir == 0) ? P_tmpdir : tmpdir);
        return temp_file(template_, file_path(prefix), flag, path);
    }
    
	io::file temp_file(const file_path& template_, const file_path& parent_dir, int flag /*= io::of_create | io::of_remove_on_close*/, file_path* path/* = 0*/)
    {
        AIO_PRE_CONDITION(flag == 0 ||  flag  == io::of_remove_on_close);
        flag |= io::of_create;

        if (state(parent_dir).state != st_dir)
            AIO_THROW(io::create_failed)("failed to locate the temp directory:")(parent_dir.str());

        const int max_try = 100;
        for(int i = 0; i < max_try ; ++i)
        {
			auto fpath = parent_dir / private_::gen_temp_name(template_);
			if (path)
				*path = fpath;
			return io::file(fpath, flag);
        }

        AIO_THROW(io::create_failed)("failed to create temp file in directory:")(parent_dir.str());
    }

    
    file_path temp_dir(const file_path& template_/* = "tmpd"*/)
    {
        const char* tmpdir = getenv("TMPDIR");
        if (!tmpdir)
            tmpdir = getenv("TMP");

        string prefix = as_range_string((tmpdir == 0) ? P_tmpdir : tmpdir);
        return temp_dir(template_, file_path(prefix));
    }

    
    file_path temp_dir(const file_path& template_, const file_path& parent_dir)
    {
        if (state(parent_dir).state != st_dir)
            AIO_THROW(io::create_failed)("failed to locate the temp directory:")(parent_dir.str());

        const int max_try = 100;
        for(int i = 0; i < max_try ; ++i)
        {
			file_path path = parent_dir / private_::gen_temp_name(template_);
            if (create_dir(path) == er_ok)
                return path;
        }

        AIO_THROW(io::create_failed)("failed to create temp file in directory:")(parent_dir.str());
    }

    bool exists(const file_path& file)
    {
        return state(file).state != st_not_found;
    }

    fs_error recursive_remove(const file_path& path)
    {
		for (auto& p: children(path))
			recursive_remove(path / p);
        
        return remove(path);
    }

    fs_error recursive_create_dir(const file_path& path)
    {
		if (path.empty())
			return er_ok;

		auto first(path.begin()), last(path.end());

		file_path current;

		for (;first != last; ++first){
			current /= *first;
            fs::file_state st = state(current).state;
            if (st == st_not_found)
            {
                fs_error err = create_dir(current);
                if (err != er_ok)
                    return err;
            }
            else if (st != st_dir)
            {
                return er_invalid;
            }

		}

		return er_ok;
    }

    io::file recursive_create(const file_path& path, int flag)
    {
        fs_error err = recursive_create_dir(path.parent());
        if (err != er_ok)
        {
			AIO_THROW(io::create_failed)(path.str());
        }
        return io::file(path, flag);
    }

}}
