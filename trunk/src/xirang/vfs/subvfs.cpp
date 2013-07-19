#include <aio/xirang/vfs/subvfs.h>

namespace xirang{ namespace fs{

	SubVfs::SubVfs(IVfs& parent_, const string& dir)
		: m_root(0), parent(parent_), m_resource(dir.empty()? dir : append_tail_slash(dir))
	{
	}

	SubVfs::~SubVfs()
	{	
	}

	// common operations of dir and file
	// \pre !absolute(path)
	fs_error SubVfs::remove(const string& path) { 
        return parent.remove(m_resource << path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error SubVfs::createDir(const  string& path){
        return parent.createDir(m_resource << path);
	}

	// \pre !absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	fs_error SubVfs::copy(const string& from, const string& to){
		return parent.copy(from, m_resource << to);
	}

	fs_error SubVfs::truncate(const string& path, aio::long_size_t s) {
		AIO_PRE_CONDITION(!is_absolute(path));
        return parent.truncate(m_resource << path, s);
	}

	void SubVfs::sync() { parent.sync();  }

	// query
	const string& SubVfs::resource() const { return m_resource; }

	// volume
	// if !mounted, return null
	RootFs* SubVfs::root() const { return m_root; }

	// \post mounted() && root() || !mounted() && !root()
	bool SubVfs::mounted() const { 
        return m_root != 0;
    }

	// \return mounted() ? absolute() : empty() 
    string SubVfs::mountPoint() const { return m_root ? m_root->mountPoint(*this) : aio::empty_str;}

	// \pre !absolute(path)
	VfsNodeRange SubVfs::children(const string& path) const{
        return parent.children(m_resource << path);
	}

	// \pre !absolute(path)
	VfsState SubVfs::state(const string& path) const {
        AIO_PRE_CONDITION(!is_absolute(path));
        return parent.state(m_resource << path);
	}

	// if r == null, means unmount
	void SubVfs::setRoot(RootFs* r) { 
        AIO_PRE_CONDITION(!mounted() || r == 0);
        m_root = r;
	}
	void** SubVfs::do_create(unsigned long long mask,
			void* ret, aio::unique_ptr<void>& owner, const string& path, int flag){
        AIO_PRE_CONDITION(!is_absolute(path));
        return parent.do_create(mask, ret, owner, m_resource << path,  flag);
	}

} }

