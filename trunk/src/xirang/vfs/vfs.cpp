#include <xirang/vfs.h>
#include <boost/tokenizer.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>


#include <deque>
#include <xirang/string_algo/char_separator.h>

namespace xirang{ namespace fs{
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
		typedef boost::bimap<string, boost::bimaps::unordered_set_of<IVfs*> > mount_map;
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

		// \pre is_absolute(dir) && is_normalized(dir)
		fs_error mount(const string& dir, IVfs& vfs)
		{
			AIO_PRE_CONDITION(is_absolute(dir) && is_normalized(dir));
			AIO_PRE_CONDITION(!vfs.mounted());
			AIO_PRE_CONDITION(0 == mountPoint(dir));

			string pw = append_tail_slash(dir);

			VfsState st = locate(pw);

			if (st.state == aiofs::st_dir || (pw == aio::literal("/") && m_mount_map.empty()) )
			{
				m_mount_map.insert(mount_value(pw, &vfs));
				vfs.setRoot(m_host);
				return aiofs::er_ok;
			}
			
			return aiofs::er_not_a_mount_point;
		}

		fs_error unmount(const string& dir)
		{
			string pw = append_tail_slash(dir);
			if (containMountPoint(pw))
				return aiofs::er_busy_mounted;

			mount_map::left_iterator pos = m_mount_map.left.find(pw);
			AIO_PRE_CONDITION(pos != m_mount_map.left.end());
            pos->second->setRoot(0);
			m_mount_map.left.erase(pos);
			return aiofs::er_ok;
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
		string mountPoint(const IVfs& p) const
		{
			IVfs* fs = const_cast<IVfs*>(&p);
			mount_map::right_const_iterator pos = m_mount_map.right.find(fs);
			return pos == m_mount_map.right.end()
                ? aio::empty_str
				: pos->second;
		}
		IVfs* mountPoint(const string& p) const
		{
			AIO_PRE_CONDITION(is_absolute(p));
			mount_map::left_const_iterator pos = m_mount_map.left.find(p);
			return pos == m_mount_map.left.end()
				? 0
				: pos->second;
		}

		bool containMountPoint(const string& path) const
		{
			AIO_PRE_CONDITION(is_absolute(path));

			string pw = append_tail_slash(path);
            mount_map::left_const_iterator pos = m_mount_map.left.lower_bound(pw);
			if (pos != m_mount_map.left.end()
					&& pw == pos->first)
				++pos;
			return pos != m_mount_map.left.end()
				&& pw.size() <= pos->first.size()
				&& std::equal(pw.begin(), pw.end(), pos->first.begin());
		}

		VfsRange mountedFS() const
		{
			return VfsRange(
                VfsRange::iterator(MountInfoIterator(m_mount_map.left.begin())),
				VfsRange::iterator(MountInfoIterator(m_mount_map.left.end()))
					);

		}

		VfsState locate(const string& path) const
		{
			AIO_PRE_CONDITION(is_absolute(path));
			if (m_mount_map.empty())	//not initialized
			{
				return VfsState();
			}
			string pw = append_tail_slash(path);

            mount_map::left_const_iterator pos = m_mount_map.left.lower_bound(pw);
			if (pos != m_mount_map.left.end()
					&& pw == pos->first)
			{
				VfsState st = {
					{aio::const_range_string(pw.begin() + pos->first.size(), pw.end()), pos->second},
					aiofs::st_mount_point,
					0
				};
				return st;
			}

			AIO_INVARIANT(pos != m_mount_map.left.begin());

			for(;;)
			{
				--pos;
				if (pw.size() >= pos->first.size()
					&& std::equal(pos->first.begin(), pos->first.end(), pw.begin()) )
				{
					return pos->second->state(aio::const_range_string(path.begin() + pos->first.size(), path.end()));
				}
			} 

			AIO_PRE_CONDITION(false);
			return VfsState();
		}
	private:
		mount_map m_mount_map;
		string m_resource;
		RootFs* m_host;
	};

	RootFs::RootFs(const string& res )
		: m_imp(new RootFsImp(res, this))
	{
	}

	RootFs::~RootFs()
	{
		aio::check_delete(m_imp);
	}

	// \pre dir must be absolute name
	fs_error RootFs::mount(const string& dir, IVfs& vfs)
	{
		return m_imp->mount(dir, vfs);
	}
	fs_error RootFs::unmount(const string& dir)
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
	string RootFs::mountPoint(const IVfs& p) const
	{
		return m_imp->mountPoint(p);
	}
	IVfs* RootFs::mountPoint(const string& p) const
	{
		return m_imp->mountPoint(p);
	}

	VfsRange RootFs::mountedFS() const
	{
		return m_imp->mountedFS();
	}
	VfsState RootFs::locate(const string& path) const
	{
		return m_imp->locate(path);
	}
	bool RootFs::containMountPoint(const string& path) const
	{
		return m_imp->containMountPoint(path);
	}

    // return true if p[0] is '/';
	bool is_absolute(const string& p)
	{
		return !p.empty() && p[0] == '/';
	}

    string temp_dir(IVfs& vfs, aio::const_range_string template_, aio::const_range_string parent_dir)
    {
        if (vfs.state(parent_dir).state != aio::fs::st_dir)
            AIO_THROW(aio::io::create_failed)("failed to locate the temp directory:")(parent_dir);

        string prefix =  parent_dir.empty() ? string(template_) : append_tail_slash(parent_dir) << template_;

        const int max_try = 100;
        for(int i = 0; i < max_try ; ++i)
        {
            string file_path = aio::fs::private_::gen_temp_name(prefix);
            if (vfs.createDir(file_path) == aio::fs::er_ok)
                return file_path;
        }

        AIO_THROW(aio::io::create_failed)("failed to create temp file in directory:")(parent_dir);
    }
    fs_error recursive_remove(IVfs&vfs, const string& path)
    {
        if (vfs.state(path).state == aio::fs::st_dir)
        {
            VfsNodeRange rf = vfs.children(path);
            aio::string pathprefix = append_tail_slash(path);
            std::vector<VfsNode> files;

            for (VfsNodeRange::iterator itr = rf.begin(); itr != rf.end(); ++itr){
                files.push_back(*itr);
            }
            for (std::vector<VfsNode>::iterator itr = files.begin(); itr != files.end(); ++itr)
            {
                aio::string newpath = pathprefix << (*itr).path;
                fs_error ret = recursive_remove(*itr->owner_fs, newpath);
                if (ret != aio::fs::er_ok)
                    return ret;
            }
        }
        
        return vfs.remove(path);
    }
    fs_error recursive_create_dir(IVfs&vfs, const string& path)
    {
        aio::char_separator<char> sep('/');
        typedef boost::tokenizer<aio::char_separator<char>, string::const_iterator, aio::const_range_string> tokenizer;

        aio::string sp(normalize(path));
		tokenizer tokens(sp, sep);

        tokenizer::iterator itr = tokens.begin();
        string mpath;
        if (itr != tokens.end() && itr->empty())
        {
            mpath = aio::literal("/");
            ++itr;
        }

		for (; itr != tokens.end(); ++itr)
		{
            mpath = mpath << *itr ;
            aio::fs::file_state st = vfs.state(mpath).state;
            if (st == aio::fs::st_not_found)
            {
                aio::fs::fs_error err = vfs.createDir(mpath);
                if (err != aio::fs::er_ok)
                    return err;
            }
            else if (st != aio::fs::st_dir)
            {
                return aio::fs::er_invalid;
            }

            mpath = mpath << aio::literal("/");
		}
		return aio::fs::er_ok;
    }

}	//fs
}	//xirang
