#include <xirang/vfs/local.h>
#include <xirang/type/xrbase.h>
#include <xirang/io/file.h>
#include <xirang/string_algo/utf8.h>
#include <xirang/vfs/vfs_common.h>

// BOOST
#include <boost/filesystem.hpp>

#include <unistd.h>
namespace xirang{ namespace vfs{
	class LocalFileIterator
	{
	public:
		LocalFileIterator()
			: m_itr()
		{
            m_node.owner_fs = 0;
        }

		explicit LocalFileIterator(const file_path& rpath, IVfs* fs)
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
            m_node.owner_fs = fs;
        }

		const VfsNode& operator*() const
		{
            m_node.path = file_path(m_itr->path().leaf().c_str());
			return m_node;
		}

        const VfsNode* operator->() const
		{
			return &**this;
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
#ifdef MSVC_COMPILER_
		boost::filesystem::wdirectory_iterator m_itr;
#else
		boost::filesystem::directory_iterator m_itr;
#endif
        mutable VfsNode m_node;
	};

	LocalFs::LocalFs(const file_path& dir)
		: m_root(0), m_resource(dir)
	{
	}
	LocalFs::~LocalFs()
	{	
	}

	// common operations of dir and file
	// \pre !absolute(path)
	fs_error LocalFs::remove(sub_file_path path) { 
        AIO_PRE_CONDITION(!path.is_absolute());
        fs_error ret = remove_check(*this, path);
        if (ret != fs::er_ok)
            return ret;
        if (path.empty())
            return fs::er_invalid;
        return fs::remove(m_resource / path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error LocalFs::createDir(sub_file_path path){
        AIO_PRE_CONDITION(!path.is_absolute());
        if (path.empty())
            return fs::er_invalid;
		return fs::create_dir(m_resource / path);
	}

	io::file LocalFs::writeOpen(sub_file_path path, int flag) {
        AIO_PRE_CONDITION(!path.is_absolute());
        return io::file(m_resource / path, flag);

	}
	io::file_reader LocalFs::readOpen(sub_file_path path){
        AIO_PRE_CONDITION(!path.is_absolute());
        return io::file_reader(m_resource / path);
	}
	void** LocalFs::do_create(unsigned long long mask,
			void** base, unique_ptr<void>& owner, sub_file_path path, int flag){
		using namespace io;

		void** ret = 0;
		if (mask & detail::get_mask<io::writer, io::write_view>::value ){ //write open
			unique_ptr<io::file> ar(new io::file(writeOpen(path, flag)));
			iref<reader, writer, io::random, ioctrl, read_map, write_map> ifile(*ar);
			ret = copy_interface<reader, writer, io::random, ioctrl, read_map, write_map >::apply(mask, base, ifile, (void*)ar.get()); 
			unique_ptr<void>(std::move(ar)).swap(owner);
		}
		else{ //read open
			unique_ptr<io::file_reader> ar(new io::file_reader(readOpen(path)));
			iref<reader, io::random, read_map> ifile(*ar);
			ret = copy_interface<reader, io::random, read_map>::apply(mask, base, ifile, (void*)ar.get()); 
			unique_ptr<void>(std::move(ar)).swap(owner);
		}
		return ret;
	}

	// \pre !absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	fs_error LocalFs::copy(sub_file_path from, sub_file_path to){
        AIO_PRE_CONDITION(!to.is_absolute());
		
        VfsNode from_node = { from, this};
        if (from.is_absolute())
        {
            AIO_PRE_CONDITION(mounted());
            from_node = m_root->locate(from).node;
        }

        VfsNode to_node = { to, this};
        return xirang::vfs::copyFile(from_node, to_node);
	}

	fs_error LocalFs::truncate(sub_file_path path, long_size_t s) {
        AIO_PRE_CONDITION(!path.is_absolute());
		return fs::truncate(m_resource / path, s);
	}

	void LocalFs::sync() { 
        ::sync();
    }

	// query
	const string& LocalFs::resource() const { return m_resource.str(); }

	// volume
	// if !mounted, return null
	RootFs* LocalFs::root() const { return m_root; }

	// \post mounted() && root() || !mounted() && !root()
	bool LocalFs::mounted() const { 
        return m_root != 0;
    }

	// \return mounted() ? absolute() : empty() 
	file_path LocalFs::mountPoint() const { return m_root ? m_root->mountPoint(*this) : file_path();}

	// \pre !absolute(path)
	VfsNodeRange LocalFs::children(sub_file_path path) const{
        AIO_PRE_CONDITION(!path.is_absolute());
        VfsState st = state(path);
        if (st.state == fs::st_dir)
        {
            return VfsNodeRange(
                VfsNodeRange::iterator(LocalFileIterator(m_resource/path, const_cast<LocalFs*>(this))),
                VfsNodeRange::iterator(LocalFileIterator())
                );
        }
        return VfsNodeRange();
	}

	// \pre !absolute(path)
	VfsState LocalFs::state(sub_file_path path) const {
        AIO_PRE_CONDITION(!path.is_absolute());

        fs::fstate st = fs::state(m_resource/path);
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
