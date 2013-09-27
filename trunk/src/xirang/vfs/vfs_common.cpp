#include <xirang/vfs/vfs_common.h>

namespace xirang{ namespace vfs{ 
    fs_error remove_check(IVfs& fs, sub_file_path path)
	{
		AIO_PRE_CONDITION(!path.is_absolute());

		if (fs.mounted())
		{
			auto mount_point = fs.root()->mountPoint(fs);
			if (fs.root()->containMountPoint(mount_point / path))
				return fs::er_busy_mounted;
		}

		VfsState st = fs.state(path);

		if (st.state == fs::st_invalid)
			return fs::er_invalid;

		if (st.state == fs::st_not_found)
			return fs::er_not_found;

		return fs::er_ok;
	}

	fs_error copyFile(const VfsNode& from, const VfsNode& to)
	{
		using io::reader;
		using io::writer;
		using io::sequence;

		auto src = from.owner_fs->create<reader, sequence>(from.path, io::of_open);
		auto dest = to.owner_fs->create<writer>(to.path, io::of_create_or_open);

		long_size_t copied_size = copy_data(src.get<reader>(), dest.get<writer>());
		if (copied_size != src.get<sequence>().size())
			return fs::er_system_error;
		return fs::er_ok;
	}
}}


