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

        string gen_temp_name(const_range_string template_)
        {
			if (!contains(template_, '?'))	//back compatibility
				return template_ << gen_temp_name_();

			return replace(string(template_).range_str(), literal("?"), gen_temp_name_().range_str());
        }
    }
#ifdef GNUC_COMPILER_
    fs_error move(const string& from, const string& to)
    {
		int res = link(from.c_str(), to.c_str());
		if (res == 0){
			unlink(from.c_str());
			return er_ok;
		}

		auto err = errno;
		if(EXDEV == err){
			try{
				copy(from, to);
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

    fs_error remove(const string& path)
    {
        if (state(path).state == st_dir)
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
    fs_error create_dir(const  string& path)
    {
        if (state(path).state != st_not_found)
            return er_exist;

        return (::mkdir(path.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) != 0)
            ? er_system_error
            : er_ok;
    }

    fstate state(const string& path)
    {
        fstate fst = {path, st_not_found, 0};
        struct stat st;
        if (stat(path.c_str(), &st) == 0)
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

    fs_error truncate(const string& path, long_size_t s)
    {
        int ret = ::truncate(path.c_str(), s);
        return ret == 0 ? er_ok : er_system_error;
    }
#elif defined(MSVC_COMPILER_)
    fs_error move(const string& from, const string& to)
    {
        wstring wfrom = utf8::decode_string(to_native_path(from));
        wstring wto = utf8::decode_string(to_native_path(to));
		return MoveFileW(wfrom.c_str(), wto.c_str()) ? er_ok : er_system_error;

    }

    fs_error remove(const string& path)
    {
        wstring wpath = utf8::decode_string(to_native_path(path));
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

    fs_error create_dir(const  string& path)
    {
        if (state(path).state != st_not_found)
            return er_exist;

        wstring wpath = utf8::decode_string(to_native_path(path));
        return (_wmkdir(wpath.c_str()) != 0)
            ? er_system_error
            : er_ok;
    }
    
    fstate state(const string& path)
    {   
        // NOTES: on windows, the pure disk letter or network dir MUST endded with '\'.
        // Network file MUST NOT end with '\'
        // A local dir or file path  MUST NOT ended with '\'.
        // I was freaked out!

        bool is_disk = path.size() == 2 && path[1] == ':';
        bool is_network = !path.empty() && path[0] == '/';
        bool is_dir_or_network = is_disk || is_network;

        
        string tmppath = is_dir_or_network? append_tail_slash(path) : remove_tail_slash(path);
        fstate fst = {remove_tail_slash(path), st_not_found, 0};

        struct _stat st;
        wstring_builder wpath;

        wpath.reserve(tmppath.size() + 1);
        utf8::decode(to_range(tmppath), std::back_inserter(wpath));
        //wstring wpath = utf8::decode_string(to_native_path(tmppath)); // for network path, try as dir at first
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
        else if (is_network) //retry as a file
        {
            wpath.clear();
            utf8::decode(to_range(to_native_path(fst.path)), std::back_inserter(wpath));
            //wstring wpath = utf8::decode_string(to_native_path(fst.path));
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

    fs_error truncate(const string& path, long_size_t s)
    {
        wstring wpath = utf8::decode_string(to_native_path(path));
        HANDLE hFile = CreateFileW(wpath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            return er_system_error;

        SetFilePointer(hFile, (LONG)s, 0, FILE_BEGIN);
        BOOL ret = SetEndOfFile(hFile);
        CloseHandle(hFile);
        return ret ? er_ok : er_system_error;
    }
#endif

    void copy(const string& from, const string& to)
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

		explicit file_iterator(const string& rpath)
			: m_itr(rpath.c_str())
		{   
        }

		const string& operator*() const
		{
            m_path = m_itr->path().leaf().string() ;
			return m_path;
		}

        const string* operator->() const
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
		boost::filesystem::directory_iterator m_itr;
        mutable string m_path;
	};

    file_range children(const string& path)
    {
        fstate st = state(path);
        return (st.state == st_dir)
            ? file_range(file_iterator(path), file_iterator())
            : file_range();
    }

	io::file temp_file(const_range_string template_/* = "tmpf"*/, int flag /*= io::of_create | io::of_remove_on_close*/, string* path/* = 0*/)
    {   
        const char* tmpdir = getenv("TMPDIR");
        if (!tmpdir)
            tmpdir = getenv("TMP");

        string prefix = as_range_string((tmpdir == 0) ? P_tmpdir : tmpdir);
        return temp_file(template_, to_xirang_path(prefix), flag, path);
    }
    
	io::file temp_file(const_range_string template_, const_range_string parent_dir, int flag /*= io::of_create | io::of_remove_on_close*/, string* path/* = 0*/)
    {
        AIO_PRE_CONDITION(flag == 0 ||  flag  == io::of_remove_on_close);
        flag |= io::of_create;

        if (state(parent_dir).state != st_dir)
            AIO_THROW(io::create_failed)("failed to locate the temp directory:")(parent_dir);

        string parent =  append_tail_slash(parent_dir);

        const int max_try = 100;
        for(int i = 0; i < max_try ; ++i)
        {
            string_builder file_path = parent;
            file_path += private_::gen_temp_name(template_);

			if (path)
				*path = file_path;
			return io::file(file_path.str(), flag);
        }

        AIO_THROW(io::create_failed)("failed to create temp file in directory:")(parent_dir);
    }

    
    string temp_dir(const_range_string template_/* = "tmpd"*/)
    {
        const char* tmpdir = getenv("TMPDIR");
        if (!tmpdir)
            tmpdir = getenv("TMP");

        string prefix = as_range_string((tmpdir == 0) ? P_tmpdir : tmpdir);
        return temp_dir(template_, to_xirang_path(prefix));
    }

    
    string temp_dir(const_range_string template_, const_range_string parent_dir)
    {
        if (state(parent_dir).state != st_dir)
            AIO_THROW(io::create_failed)("failed to locate the temp directory:")(parent_dir);

        string prefix =  append_tail_slash(parent_dir) << template_;

        const int max_try = 100;
        for(int i = 0; i < max_try ; ++i)
        {
            string file_path = private_::gen_temp_name(prefix);
            if (create_dir(file_path) == er_ok)
                return file_path;
        }

        AIO_THROW(io::create_failed)("failed to create temp file in directory:")(parent_dir);
    }

    bool exists(const_range_string file)
    {
        return state(file).state != st_not_found;
    }

    string append_tail_slash(const string& p)
	{
		return p.empty() || *(p.end() - 1) != '/'
			? p << literal("/")
			: p;
	}

    string remove_tail_slash(const string& p){
        return !p.empty() && *(p.end() - 1) == '/'
			? string(make_range(p.begin(), p.end() - 1))
			: p;
    }
	bool has_tail_slash(const string& p){
		return !p.empty() && *(p.end() - 1) == '/';
	}

    string normalize(const string& p)
	{
        bool is_abs = !p.empty() && *p.begin() == '/';
		bool end_slash = !p.empty() && *(p.end() - 1) == '/';

		char_separator<char> sep('/');
        typedef boost::tokenizer<char_separator<char>, string::const_iterator, const_range_string> tokenizer;
        tokenizer tokens(p, sep);

		std::deque<string> stack;
		bool append_slash = false;
		for (tokenizer::iterator itr = tokens.begin(); itr != tokens.end(); ++itr)
		{
			if (*itr == onedot)
			{
				append_slash = true;
				continue;
			}
			if (*itr == twodot)
			{
				if (!stack.empty()) stack.pop_back();
				append_slash = true;
				continue;
			}
			if (!itr->empty())
				stack.push_back(string(*itr));
			append_slash = end_slash;
		}

		string_builder result;
		if (is_abs){
			result.push_back('/');
            if (p.size() > 1 && p[1] == '/') // start with "//" 
                result.push_back('/');
        }

		for (std::deque<string>::iterator itr = stack.begin(); itr != stack.end(); ++itr)
		{
			result += *itr;
			result.push_back('/');
		}
		if (!append_slash && !stack.empty() )	//remove end slash if p doesn't end with '/'
			result.resize(result.size() - 1);
		return string(result);
	}

    bool is_normalized(const string& p)
	{
		char_separator<char> sep('/');
        typedef boost::tokenizer<char_separator<char>, string::const_iterator, const_range_string> tokenizer;
		tokenizer tokens(p, sep);

        
		for (tokenizer::iterator itr = tokens.begin(); itr != tokens.end(); ++itr)
		{
            const_range_string tmp = *itr;
			if (tmp == onedot || tmp == twodot)
				return false;
		}
		return true;

	}

	bool is_filename(const string& p)
	{
		return find(p, '/') == p.end();
	}

    string to_native_path(const string& p)
    {
#ifndef MSVC_COMPILER_
        return replace(p, '\\', '/');
#endif
        return replace(p, '/', '\\');
        
    }

    string to_xirang_path(const string& p)
    {
        return replace(p, '\\', '/');
    }

    fs_error recursive_remove(const string& path)
    {
        if (state(path).state == st_dir)
        {
            file_range rf = children(path);
            string pathprefix = append_tail_slash(path);
            for (file_range::iterator itr = rf.begin(); itr != rf.end(); ++itr){
                recursive_remove(pathprefix << *itr);
            }
        }
        
        return remove(path);
    }

    fs_error recursive_create_dir(const string& path)
    {
        char_separator<char> sep('/', 0, keep_empty_tokens);
        typedef boost::tokenizer<char_separator<char>, string::const_iterator, const_range_string> tokenizer;
        string normalized_path = normalize(path);
		tokenizer tokens(normalized_path, sep);

        tokenizer::iterator itr = tokens.begin();
        string_builder mpath;
        if (itr != tokens.end() && itr->empty())
        {
            mpath.push_back('/');
            ++itr;
        }

		for (; itr != tokens.end(); ++itr)
		{
            mpath.append(*itr);
            string fpath(mpath);
            fs::file_state st = state(fpath).state;
            if (st == st_not_found)
            {
                fs_error err = create_dir(fpath);
                if (err != er_ok)
                    return err;
            }
            else if (st != st_dir)
            {
                return er_invalid;
            }

            mpath.push_back('/');
		}
		return er_ok;
    }

    io::file recursive_create(const string& path, int flag)
    {
        fs_error err = recursive_create_dir(dir_filename(path));
        if (err != er_ok && err != er_exist)
        {
			AIO_THROW(io::create_failed)(path);
        }
        return io::file(path, flag);
    }

    string dir_filename(const string& path, string* file/* = 0*/)
    {
        string::const_iterator pos = rfind(path, '/');
        string::const_iterator fileStart = pos;

        if (pos == path.end()) 
        {
             pos = path.begin();
             fileStart = pos;
        }
        else
        {
            ++fileStart;
        }
        string dir (make_range(path.begin(), pos));
        if (file)
        {
            *file = string(make_range(fileStart, path.end()));
        }

        return dir;
    }

    string ext_filename(const string& p, string* file/* = 0*/)
    {
        return ext_filename('.', p, file);
    }

    string ext_filename(char seprator, const string& path, string* file /*= 0*/)
    {
        string::const_iterator pos = rfind(path, seprator);
        string::const_iterator extStart = pos;

        if (pos != path.end()) 
            ++extStart;

        string ext = string(make_range(extStart, path.end()));
        if (file)
            *file = string (make_range(path.begin(), pos));
            
        return ext;
    }
}}
