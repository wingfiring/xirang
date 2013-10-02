#include <xirang/vfs.h>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <xirang/string_algo/string.h>


#include <unordered_map>

namespace xirang{ namespace vfs{
	IVfs::~IVfs(){}

    any IVfs::getopt(int /*id*/, const any & /*optdata = any() */) const 
    {
        return any();
    }
    any IVfs::setopt(int /*id*/, const any & /*indata*/,  const any & /*optdata = any()*/)
    {
        return any();
    }

    class RootFsImp
	{
		typedef boost::bimap<file_path, boost::bimaps::unordered_set_of<IVfs*> > mount_map;
		typedef mount_map::value_type mount_value;
		class MountInfoIterator
		{
			public:
				MountInfoIterator(mount_map::left_const_iterator pos)
					: m_pos(pos)
				{
				}

				const MountInfo& operator*() const
				{
					m_value.path = m_pos->first;
                    m_value.fs = m_pos->second;
                    return m_value;
				}
                const MountInfo* operator->() const
                {   
                    return &**this;
                }
				MountInfoIterator& operator++()	{ ++m_pos; return *this;}
				MountInfoIterator operator++(int) { 
					MountInfoIterator ret = *this; 
					++*this; 
					return ret;
				}

				MountInfoIterator& operator--(){ return *this;}
				MountInfoIterator operator--(int){ return *this;}

				bool operator==(const MountInfoIterator& rhs) const
				{
					return m_pos == rhs.m_pos;
				}
			private:
				mount_map::left_const_iterator m_pos;
                mutable MountInfo m_value;
		};
	public:
		RootFsImp(const string& res, RootFs* host)
			: m_resource(res), m_host(host)
		{
			AIO_PRE_CONDITION(host);
		}

		fs_error mount(sub_file_path dir, xirang::unique_ptr<IVfs>& vfs){
			AIO_PRE_CONDITION(vfs);
			auto ret = mount(dir, *vfs);
			if (ret == fs::er_ok){
				m_owned_fs[dir.str()] = std::move(vfs);
			}
			return ret;
		}
		// \pre is_absolute(dir) && is_normalized(dir)
		fs_error mount(sub_file_path dir, IVfs& vfs)
		{
			AIO_PRE_CONDITION(dir.is_absolute() && dir.is_normalized());
			AIO_PRE_CONDITION(!vfs.mounted());
			AIO_PRE_CONDITION(0 == mountPoint(dir));

			VfsState st = locate(dir);

			if (st.state == fs::st_dir || (dir.is_root() && m_mount_map.empty()) )
			{
				m_mount_map.insert(mount_value(dir, &vfs));
				vfs.setRoot(m_host);
				return fs::er_ok;
			}
			
			return fs::er_not_a_mount_point;
		}

		fs_error unmount(sub_file_path dir)
		{
			if (containMountPoint(dir))
				return fs::er_busy_mounted;

			mount_map::left_iterator pos = m_mount_map.left.find(dir);
			AIO_PRE_CONDITION(pos != m_mount_map.left.end());
            pos->second->setRoot(0);
			m_mount_map.left.erase(pos);
			m_owned_fs.erase(dir.str());
			return fs::er_ok;
		}

		void sync()
		{
            mount_map::right_iterator itr = m_mount_map.right.begin();
            mount_map::right_iterator iend = m_mount_map.right.end();
			for (;
					itr != iend; ++itr)
			{
				itr->first->sync();
			}
		}

		// query
		const string& resource() const {
			return m_resource;
		}

		// volume
		// return absolute name
		file_path mountPoint(const IVfs& p) const
		{
			IVfs* fs = const_cast<IVfs*>(&p);
			mount_map::right_const_iterator pos = m_mount_map.right.find(fs);
			return pos == m_mount_map.right.end()
                ? file_path()
				: pos->second;
		}
		IVfs* mountPoint(sub_file_path p) const
		{
			AIO_PRE_CONDITION(p.is_absolute());
			mount_map::left_const_iterator pos = m_mount_map.left.find(p);
			return pos == m_mount_map.left.end()
				? 0
				: pos->second;
		}

		bool containMountPoint(sub_file_path path) const
		{
			AIO_PRE_CONDITION(path.is_absolute());

            mount_map::left_const_iterator pos = m_mount_map.left.lower_bound(path);
			for(; pos != m_mount_map.left.end(); ++pos){
				if (pos->first.under(path))
					return true;
			}
			return false;
		}

		VfsRange mountedFS() const
		{
			return VfsRange(
                VfsRange::iterator(MountInfoIterator(m_mount_map.left.begin())),
				VfsRange::iterator(MountInfoIterator(m_mount_map.left.end()))
					);
		}

		VfsState locate(sub_file_path path) const
		{
			AIO_PRE_CONDITION(path.is_absolute());
            mount_map::left_const_iterator pos = m_mount_map.left.lower_bound(path);
			if (pos != m_mount_map.left.end()
					&& path == pos->first)
			{
				VfsState st = {
					{file_path(), pos->second},
					fs::st_mount_point,
					0
				};
				return st;
			}

			for(;pos != m_mount_map.left.begin(); -- pos)
			{
				if (pos->first == path || pos->first.contains(path))
				{
					sub_file_path rest(path.str().begin() + pos->first.str().size(), path.str().end());
					file_path relative_path = file_path(rest).remove_absolute();
					auto ret = pos->second->state(relative_path);
					ret.node.path = relative_path;
					return ret;
				}
			} 

			if (pos != m_mount_map.left.end()){
				if (pos->first == path || pos->first.contains(path))
				{
					sub_file_path rest(path.str().begin() + pos->first.str().size(), path.str().end());
					file_path relative_path = file_path(rest).remove_absolute();
					auto ret = pos->second->state(relative_path);
					ret.node.path = relative_path;
					return ret;
				}
			}

			return VfsState();
		}
	private:
		mount_map m_mount_map;
		std::unordered_map<string, xirang::unique_ptr<IVfs>, hash_string> m_owned_fs;
		string m_resource;
		RootFs* m_host;
	};

	RootFs::RootFs(const string& res )
		: m_imp(new RootFsImp(res, this))
	{
	}

	RootFs::~RootFs()
	{
		check_delete(m_imp);
	}

	// \pre dir must be absolute name
	fs_error RootFs::mount(sub_file_path dir, IVfs& vfs)
	{
		return m_imp->mount(dir, vfs);
	}

	fs_error RootFs::mount(sub_file_path dir, xirang::unique_ptr<IVfs>& vfs)
	{
		return m_imp->mount(dir, vfs);
	}

	fs_error RootFs::unmount(sub_file_path dir)
	{
		return m_imp->unmount(dir);
	}

	void RootFs::sync()
	{
		m_imp->sync();
	}

	// query
	const string& RootFs::resource() const
	{
		return m_imp->resource();
	}

	// volume
	// return absolute name
	file_path RootFs::mountPoint(const IVfs& p) const
	{
		return m_imp->mountPoint(p);
	}
	IVfs* RootFs::mountPoint(sub_file_path p) const
	{
		return m_imp->mountPoint(p);
	}

	VfsRange RootFs::mountedFS() const
	{
		return m_imp->mountedFS();
	}
	VfsState RootFs::locate(sub_file_path path) const
	{
		return m_imp->locate(path);
	}
	bool RootFs::containMountPoint(sub_file_path path) const
	{
		return m_imp->containMountPoint(path);
	}
	fs_error RootFs::remove(sub_file_path path){
		auto ret = locate(path);
		if (!ret.node.owner_fs)
			return fs::er_not_found;
		return ret.node.owner_fs->remove(ret.node.path);
	}
	fs_error RootFs::createDir(sub_file_path path){
		auto ret = locate(path);
		if (!ret.node.owner_fs)
			return fs::er_not_found;
		return ret.node.owner_fs->createDir(ret.node.path);
	}
	fs_error RootFs::copy(sub_file_path from, sub_file_path to) {
		auto ret = locate(from);
		if (!ret.node.owner_fs)
			return fs::er_not_found;
		return ret.node.owner_fs->copy(ret.node.path, to);
	}
	fs_error RootFs::truncate(sub_file_path path, long_size_t s){
		auto ret = locate(path);
		if (!ret.node.owner_fs)
			return fs::er_not_found;
		return ret.node.owner_fs->truncate(ret.node.path, s);
	}
	VfsNodeRange RootFs::children(sub_file_path path) const {
		auto ret = locate(path);
		if (!ret.node.owner_fs)
			return VfsNodeRange();
		return ret.node.owner_fs->children(ret.node.path);
	}
	VfsState RootFs::state(sub_file_path path) const{
		auto ret = locate(path);
		if (!ret.node.owner_fs)
			return ret;
		return ret.node.owner_fs->state(ret.node.path);

	}
	void** RootFs::do_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
		auto result = locate(path);
		if (!result.node.owner_fs)
			return 0;
		return result.node.owner_fs->do_create(mask, ret, owner, result.node.path, flag);
	}

    file_path temp_dir(IVfs& vfs, sub_file_path template_, sub_file_path parent_dir)
    {
		auto ret = vfs.state(parent_dir).state;
		if (ret == fs::st_not_found)
            AIO_THROW(fs::not_found_exception)("failed to locate the temp directory:")(parent_dir.str());

        if (ret != fs::st_dir)
            AIO_THROW(fs::not_dir_exception)("failed to locate the temp directory:")(parent_dir.str());

		file_path prefix = parent_dir / template_;

        const int max_try = 100;
        for(int i = 0; i < max_try ; ++i)
        {
            file_path fpath = fs::private_::gen_temp_name(prefix);
            if (vfs.createDir(fpath) == fs::er_ok)
                return fpath;
        }

        AIO_THROW(fs::create_exception)("failed to create temp file in directory:")(parent_dir.str());
    }

    fs_error recursive_remove(IVfs&vfs, sub_file_path path)
    {
        if (vfs.state(path).state == fs::st_dir)
        {
            auto rf = vfs.children(path);
            std::vector<VfsNode> files(rf.begin(), rf.end());

            for (auto& node : files)
            {
                fs_error ret = recursive_remove(*node.owner_fs, path / node.path);
                if (ret != fs::er_ok)
                    return ret;
            }
        }
        
        return vfs.remove(path);
    }

    fs_error recursive_create_dir(IVfs& vfs, sub_file_path path)
    {
		if (path.empty())
			return fs::er_ok;

		auto first(path.begin()), last(path.end());

		file_path current;
		for (auto & p : path){
			current /= p;

            fs::file_state st = vfs.state(current).state;
            if (st == fs::st_not_found)
            {
                fs_error err = vfs.createDir(current);
                if (err != fs::er_ok)
                    return err;
            }
            else if (st != fs::st_dir)
            {
                return fs::er_invalid;
            }
		}

		return fs::er_ok;
    }

}	//vfs
}	//xirang
