#include <xirang/vfs/subvfs.h>

namespace xirang{ namespace vfs{

	SubVfs::SubVfs(IVfs& parent_, sub_file_path dir)
		: m_root(0), parent(parent_), m_resource(dir)
	{
	}

	SubVfs::~SubVfs()
	{	
	}

	// common operations of dir and file
	// \pre !absolute(path)
	fs_error SubVfs::remove(sub_file_path path) { 
        AIO_PRE_CONDITION(!path.is_absolute());
        return parent.remove(m_resource / path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error SubVfs::createDir(sub_file_path path){
        AIO_PRE_CONDITION(!path.is_absolute());
        return parent.createDir(m_resource / path);
	}

	// \pre !absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	fs_error SubVfs::copy(sub_file_path from, sub_file_path to){
        AIO_PRE_CONDITION(!to.is_absolute());
		return parent.copy(from, m_resource / to);
	}

	fs_error SubVfs::truncate(sub_file_path path, long_size_t s) {
        AIO_PRE_CONDITION(!path.is_absolute());
        return parent.truncate(m_resource / path, s);
	}

	void SubVfs::sync() { parent.sync();  }

	// query
	const string& SubVfs::resource() const { return m_resource.str(); }

	// volume
	// if !mounted, return null
	RootFs* SubVfs::root() const { return m_root; }

	// \post mounted() && root() || !mounted() && !root()
	bool SubVfs::mounted() const { 
        return m_root != 0;
    }

	// \return mounted() ? absolute() : empty() 
    file_path SubVfs::mountPoint() const { return m_root ? m_root->mountPoint(*this) : file_path();}

	// \pre !absolute(path)
	VfsNodeRange SubVfs::children(sub_file_path path) const{
        AIO_PRE_CONDITION(!path.is_absolute());
        return parent.children(m_resource / path);
	}

	// \pre !absolute(path)
	VfsState SubVfs::state(sub_file_path path) const {
        AIO_PRE_CONDITION(!path.is_absolute());
        return parent.state(m_resource / path);
	}

	// if r == null, means unmount
	void SubVfs::setRoot(RootFs* r) { 
        AIO_PRE_CONDITION(!mounted() || r == 0);
        m_root = r;
	}
	void** SubVfs::do_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
        AIO_PRE_CONDITION(!path.is_absolute());
        return parent.do_create(mask, ret, owner, m_resource / path,  flag);
	}

} }

