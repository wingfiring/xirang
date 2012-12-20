#include <aio/xirang/vfs/vfs_common.h>

namespace xirang{ namespace fs{ 
    fs_error remove_check(IVfs& fs, const string& path)
	{
		AIO_PRE_CONDITION(!is_absolute(path));

		if (fs.mounted())
		{
			string mount_point = fs.root()->mountPoint(fs);
			if (fs.root()->containMountPoint(mount_point + path))
				return aio::fs::er_busy_mounted;
		}

		VfsState st = fs.state(path);

		if (st.state == aio::fs::st_invalid)
			return aio::fs::er_invalid;

		if (st.state == aio::fs::st_not_found)
			return aio::fs::er_not_found;

		return aio::fs::er_ok;
	}

	fs_error copyFile(const VfsNode& from, const VfsNode& to)
	{
		using aio::io::reader;
		using aio::io::writer;
		using aio::io::sequence;

		auto src = from.owner_fs->create<reader, sequence>(from.path, aio::io::of_open);
		auto dest = to.owner_fs->create<writer>(to.path, aio::io::of_create_or_open);

		aio::long_size_t copied_size = copy_data(src.get<reader>(), dest.get<writer>());
		if (copied_size != src.get<sequence>().size())
			return aio::fs::er_system_error;
		return aio::fs::er_ok;
	}
}}


