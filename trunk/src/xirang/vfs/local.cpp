#include <aio/xirang/vfs/local.h>
#include <aio/xirang/xrbase.h>
#include <aio/common/archive/file_archive.h>
#include <aio/common/string_algo/utf8.h>

// BOOST
#include <boost/filesystem.hpp>

#include <sys/stat.h>

#include <aio/xirang/vfs/vfs_common.h>
#include <aio/common/string_algo/utf8.h>

#ifdef MSVC_COMPILER_
#include <direct.h>
#endif

namespace xirang{ namespace fs{
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
            m_node.path = m_itr->path().leaf().c_str() ;
			return m_node;
		}

        const VfsNode* operator->() const
		{
            m_node.path = m_itr->path().leaf().c_str() ;
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
        if (ret != aiofs::er_ok)
            return ret;
        if (path.empty())
            return aio::fs::er_invalid;
        return aio::fs::remove(m_resource + path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error LocalFs::createDir(const  string& path){
        if (path.empty())
            return aio::fs::er_invalid;
		return aio::fs::create_dir(m_resource + path);
	}

	aio::io::file LocalFs::create(const string& path, int flag) {
        AIO_PRE_CONDITION(!is_absolute(path));
        string real_path = m_resource + path;
        return aio::io::file(real_path, flag);

	}
	aio::io::file_reader LocalFs::read_open(const string& path){
        AIO_PRE_CONDITION(!is_absolute(path));
        string real_path = m_resource + path;
        return aio::io::file_reader(real_path);
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
        return xirang::fs::copyFile(from_node, to_node);
	}

	fs_error LocalFs::truncate(const string& path, aio::long_size_t s) {
		AIO_PRE_CONDITION(!is_absolute(path));
			string real_path = m_resource + path;
            return aio::fs::truncate(m_resource + path, s);
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
	string LocalFs::mountPoint() const { return m_root ? m_root->mountPoint(*this) : "";}

	// \pre !absolute(path)
	VfsNodeRange LocalFs::children(const string& path) const{
        VfsState st = state(path);
        if (st.state == aiofs::st_dir)
        {
            string real_path = m_resource + path;
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

        string real_path = m_resource + path;

        aio::fs::fstate st = aio::fs::state(real_path);
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
