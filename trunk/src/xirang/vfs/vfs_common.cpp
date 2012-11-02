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
		aio::unique_ptr<iarchive, archive_deletor> src ( from.owner_fs->create(from.path, aio::archive::mt_read, aio::archive::of_open));
		if (!src)
			return aiofs::er_open_failed;

		aio::unique_ptr<iarchive, archive_deletor> dest ( to.owner_fs->create(to.path, aio::archive::mt_write, aio::archive::of_create_or_open));
		if (!dest )
			return aiofs::er_open_failed;

		unsigned long long copied_size = copy_archive(*src, *dest);
		return copied_size == src->query_sequence()->size()
			? aiofs::er_ok
			: aiofs::er_system_error;
	}
}}


