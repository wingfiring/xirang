//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_FS_UTILITY_H
#define AIO_COMMON_FS_UTILITY_H

#include <xirang/config.h>
#include <xirang/string.h>
#include <xirang/exception.h>
#include <xirang/io.h>
#include <xirang/iterator.h>
#include <xirang/io/file.h>
#include <xirang/path.h>

namespace xirang{ namespace fs{
    namespace private_{
        /// \note never use the returned name as file or dir name without check, otherwise may encouter security risk.
        /// It intends to be used to implement temp file/dir only.
        /// \post return start with template_
        file_path gen_temp_name(sub_file_path template_);
    }

//********************** begin core fs functions
    enum fs_error
    {
        er_ok,		///< no error
		er_invalid, 	///< invalid parameter
		er_busy_mounted,	
		er_not_found,
		er_exist,
		er_used_mount_point,
		er_not_a_mount_point,
		er_unmount_root,
		er_fs_not_found,
		er_system_error,
		er_open_failed,
		er_file_busy,
		er_not_regular,
		er_not_dir,
		er_permission_denied,
        er_not_empty,
		er_data_error,
		er_file_type,
		er_create,
		er_is_root,
    };
	AIO_EXCEPTION_TYPE(invalid_exception);
	AIO_EXCEPTION_TYPE(busy_mounted_exception);
	AIO_EXCEPTION_TYPE(not_found_exception);
	AIO_EXCEPTION_TYPE(exist_exception);
	AIO_EXCEPTION_TYPE(used_mount_point_exception);
	AIO_EXCEPTION_TYPE(not_a_mount_point_exception);
	AIO_EXCEPTION_TYPE(unmount_root_exception);
	AIO_EXCEPTION_TYPE(fs_not_found_exception);
	AIO_EXCEPTION_TYPE(system_error_exception);
	AIO_EXCEPTION_TYPE(open_failed_exception);
	AIO_EXCEPTION_TYPE(file_busy_exception);
	AIO_EXCEPTION_TYPE(not_regular_exception);
	AIO_EXCEPTION_TYPE(not_dir_exception);
	AIO_EXCEPTION_TYPE(permission_denied_exception);
	AIO_EXCEPTION_TYPE(not_empty_exception);
	AIO_EXCEPTION_TYPE(file_type_exception);
	AIO_EXCEPTION_TYPE(create_exception);
	AIO_EXCEPTION_TYPE(is_root_exception);

    enum file_state
	{
		st_invalid,
		st_regular,
		st_dir,
		st_symbol,
		st_socket,
		st_pipe,
		st_mount_point,
		st_not_found,

		st_unknown
	};

    fs_error remove(const file_path& path);
    fs_error create_dir(const file_path& path);

	AIO_EXCEPTION_TYPE(file_copy_error);
    void copy(const file_path& from, const file_path& to);
    fs_error move(const file_path& from, const file_path& to);

    fs_error truncate(const file_path& path, long_size_t s);

    struct fstate
    {
        file_path path;
        file_state state;
		long_size_t  size;
    };

    fstate state(const file_path& path);

    typedef range<bidir_iterator<const_itr_traits<file_path> > > file_range;
    file_range children(const file_path& path);

// temp file related 

    // \throw xirang::io::create_failed
	io::file temp_file(const file_path& template_ = file_path(literal("tmpf")), int flag = io::of_remove_on_close, file_path* result_path = 0);

    // \throw xirang::io::create_failed
	io::file temp_file(const file_path& template_, const file_path& parent_dir, int flag = io::of_remove_on_close, file_path* result_path = 0);

    // \throw xirang::io::create_failed
    file_path temp_dir(const file_path& template_ = file_path(literal("tmpd")));
    
    // \throw xirang::io::create_failed
    file_path temp_dir(const file_path& template_, const file_path& parent_dir);

//********************** end core fs functions

//********************** begin helpers
    bool exists(const file_path& file);

    // if path is not a dir, remove it.
    // if the path is dir, remove the dir and children, recursilvey
    fs_error recursive_remove(const file_path& path);

    // create a dir, if the some dir of the paths are not exist, create them
    fs_error recursive_create_dir(const file_path& path);

    // create a file, if the some dir of the paths are not exist, create them
    // \throw xirang::io::create_failed
	io::file recursive_create(const file_path& path, int flag);
}}

#endif //end AIO_COMMON_FS_UTILITY_H
