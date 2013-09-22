#include <xirang/vfs/local.h>
#include <xirang/type/xrbase.h>
#include <xirang/io/file.h>
#include <xirang/string_algo/utf8.h>

// BOOST
#include <boost/filesystem.hpp>

#include <sys/stat.h>

#include <xirang/vfs/vfs_common.h>
#include <xirang/string_algo/utf8.h>

#ifdef MSVC_COMPILER_
#include <direct.h>
#endif

namespace xirang{ namespace vfs{
	class LocalFileIterator
	{
	public:
		LocalFileIterator()
			: m_itr()
		{
            m_node.owner_fs = 0;
        }

		explicit LocalFileIterator(const string& rpath, IVfs* fs)
			: m_itr(rpath.c_str())
		{ 
            m_node.owner_fs = fs;
        }

		const VfsNode& operator*() const
		{
            m_node.path = m_itr->path().leaf().string() ;
			return m_node;
		}

        const VfsNode* operator->() const
		{
            m_node.path = m_itr->path().leaf().string() ;
			return &m_node;
		}

		LocalFileIterator& operator++()	{ ++m_itr; return *this;}
		LocalFileIterator operator++(int) { 
			LocalFileIterator ret = *this; 
			++*this; 
			return ret;
		}

		LocalFileIterator& operator--(){ return *this;}
		LocalFileIterator operator--(int){ return *this;}

		bool operator==(const LocalFileIterator& rhs) const
		{
			return m_itr == rhs.m_itr;
		}
	private:
		boost::filesystem::directory_iterator m_itr;
        mutable VfsNode m_node;
	};

	LocalFs::LocalFs(const string& dir)
		: m_root(0), m_resource(append_tail_slash(dir))
	{
	}
	LocalFs::~LocalFs()
	{	
	}

	// common operations of dir and file
	// \pre !absolute(path)
	fs_error LocalFs::remove(const string& path) { 
        fs_error ret = remove_check(*this, path);
        if (ret != fs::er_ok)
            return ret;
        if (path.empty())
            return fs::er_invalid;
        return fs::remove(m_resource << path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error LocalFs::createDir(const  string& path){
        if (path.empty())
            return fs::er_invalid;
		return fs::create_dir(m_resource << path);
	}

	io::file LocalFs::open_create(const string& path, int flag) {
        AIO_PRE_CONDITION(!is_absolute(path));
        string real_path = m_resource << path;
        return io::file(real_path, flag);

	}
	io::file_reader LocalFs::open(const string& path){
        AIO_PRE_CONDITION(!is_absolute(path));
        string real_path = m_resource << path;
        return io::file_reader(real_path);
	}
	void** LocalFs::do_create(unsigned long long mask,
			void** base, unique_ptr<void>& owner, const string& path, int flag){
		using namespace io;

		void** ret = 0;
		if (mask & detail::get_mask<io::writer, io::write_view>::value ){ //write open
			unique_ptr<io::file> ar(new io::file(get_cobj<LocalFs>(this).open_create(path, flag)));
			iref<reader, writer, io::random, ioctrl, read_map, write_map> ifile(*ar);
			ret = copy_interface<reader, writer, io::random, ioctrl, read_map, write_map >::apply(mask, base, ifile, (void*)ar.get()); 
			unique_ptr<void>(std::move(ar)).swap(owner);
		}
		else{ //read open
			unique_ptr<io::file_reader> ar(new io::file_reader(get_cobj<LocalFs>(this).open(path)));
			iref<reader, io::random, read_map> ifile(*ar);
			ret = copy_interface<reader, io::random, read_map>::apply(mask, base, ifile, (void*)ar.get()); 
			unique_ptr<void>(std::move(ar)).swap(owner);
		}
		return ret;
	}

	// \pre !absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	fs_error LocalFs::copy(const string& from, const string& to){
		
        VfsNode from_node = { from, this};
        if (is_absolute(from))
        {
            AIO_PRE_CONDITION(mounted());
            from_node = m_root->locate(from).node;
        }

        VfsNode to_node = { to, this};
        return xirang::vfs::copyFile(from_node, to_node);
	}

	fs_error LocalFs::truncate(const string& path, long_size_t s) {
		AIO_PRE_CONDITION(!is_absolute(path));
			string real_path = m_resource << path;
            return fs::truncate(m_resource << path, s);
	}

	void LocalFs::sync() { 
#ifndef MSVC_COMPILER_
        ::sync();
#endif
    }

	// query
	const string& LocalFs::resource() const { return m_resource; }

	// volume
	// if !mounted, return null
	RootFs* LocalFs::root() const { return m_root; }

	// \post mounted() && root() || !mounted() && !root()
	bool LocalFs::mounted() const { 
        return m_root != 0;
    }

	// \return mounted() ? absolute() : empty() 
	string LocalFs::mountPoint() const { return m_root ? m_root->mountPoint(*this) : string();}

	// \pre !absolute(path)
	VfsNodeRange LocalFs::children(const string& path) const{
        VfsState st = state(path);
        if (st.state == fs::st_dir)
        {
            string real_path = m_resource << path;
            return VfsNodeRange(
                VfsNodeRange::iterator(LocalFileIterator(real_path, const_cast<LocalFs*>(this))),
                VfsNodeRange::iterator(LocalFileIterator())
                );
        }
        return VfsNodeRange();
	}

	// \pre !absolute(path)
	VfsState LocalFs::state(const string& path) const {
        AIO_PRE_CONDITION(!is_absolute(path));

        string real_path = m_resource << path;

        fs::fstate st = fs::state(real_path);
        VfsState fst =
        {
            { path, const_cast<LocalFs*>(this)},
            st.state, st.size
        };

        return fst;
	}

	// if r == null, means unmount
	void LocalFs::setRoot(RootFs* r) { 
        AIO_PRE_CONDITION(!mounted() || r == 0);
        m_root = r;
	}

}
}
