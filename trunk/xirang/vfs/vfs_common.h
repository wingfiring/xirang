#ifndef SRC_XIRANG_VFS_VFS_COMMON_H
#define SRC_XIRANG_VFS_VFS_COMMON_H

#include <xirang/vfs.h>
namespace xirang{ namespace vfs{

    extern fs_error remove_check(IVfs& fs, sub_file_path path);
	extern fs_error copyFile(const VfsNode& from, const VfsNode& to);
}}
#endif //end SRC_XIRANG_VFS_VFS_COMMON_H

