#ifndef SRC_XIRANG_VFS_VFS_COMMON_H
#define SRC_XIRANG_VFS_VFS_COMMON_H

#include <aio/xirang/vfs.h>
namespace xirang{ namespace fs{

    extern fs_error remove_check(IVfs& fs, const string& path);
	extern fs_error copyFile(const VfsNode& from, const VfsNode& to);
}}
#endif //end SRC_XIRANG_VFS_VFS_COMMON_H

