//XIRANG_LICENSE_PLACE_HOLDER

#ifndef AIO_COMMON_FS_UTILITY_H
#define AIO_COMMON_FS_UTILITY_H

#include <aio/common/config.h>
#include <aio/common/string.h>
#include <aio/common/exception.h>
#include <aio/common/io.h>
#include <aio/common/iterator.h>
#include <aio/common/io/file.h>

namespace aio{ namespace fs{
    namespace private_{
        /// \note never use the returned name as file or dir name without check, otherwise may encouter security risk.
        /// It intends to be used to implement temp file/dir only.
        /// \post return start with template_
        string gen_temp_name(const_range_string template_);
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
    };

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

    fs_error remove(const string& path);
    fs_error create_dir(const  string& path);
	AIO_EXCEPTION_TYPE(file_copy_error);
    void copy(const string& from, const string& to);
    fs_error move(const string& from, const string& to);

    fs_error truncate(const string& path, long_size_t s);

    struct fstate
    {
        string path;
        file_state state;
		long_size_t  size;
    };

    fstate state(const string& path);

    typedef range<bidir_iterator<const_itr_traits<string> > > file_range;
    file_range children(const string& path);

// temp file related 

    // \throw aio::io::create_failed
	io::file temp_file(const_range_string template_ = literal("tmpf"), int flag = io::of_remove_on_close, string* result_path = 0);

    // \throw aio::io::create_failed
	io::file temp_file(const_range_string template_, const_range_string parent_dir, int flag = io::of_remove_on_close, string* result_path = 0);

    // \throw aio::io::create_failed
    string temp_dir(const_range_string template_ = literal("tmpd"));
    
    // \throw aio::io::create_failed
    string temp_dir(const_range_string template_, const_range_string parent_dir);

//********************** end core fs functions

//********************** begin helpers
    bool exists(const_range_string file);

    //if p end with '/', do nothing. otherwise add '/' at the end
	string append_tail_slash(const string& p);

    //if p end with '/', remove it.
    string remove_tail_slash(const string& p);

	bool has_tail_slash(const string& p);

    // remove all '.' and '..' name from the path.
	// note: p can contain arbitrary number of "." or ".."
	// 		path like "/../.." and "../.." are valid
	// 		"/.." will be "/", ".." will be ""
	// 		"/../path" will be "/path", "../path" will be "path"
	// if is_absolute(p), the return must be is_absolute
	// if !is_absolute(p), the return must be !is_absolute
	// if the returned path can be distinguished as a directory, should end with '/'
	// in other words, if a node is followed by '/' in p, and the result contains 
	// this node, then it will still be followed by '/'.
	//    "made/everything/simple/.." ==> "made/everything/"
	//    "made/everything/../simple" ==> "made/simple"
	//    "made/everything/../simple/" ==> "made/simple/"
    string normalize(const string& p);

    // return true if not contain any '.' or '..' name
	bool is_normalized(const string& p);

    //return true if p doesn't contains '/'
    bool is_filename(const string& p);

    // to OS style path
    string to_native_path(const string& p);

    // to aio style path
    // it's ok if p is an aio style path
    string to_aio_path(const string& p);

    // if path is not a dir, remove it.
    // if the path is dir, remove the dir and children, recursilvey
    fs_error recursive_remove(const string& path);


    // create a dir, if the some dir of the paths are not exist, create them
    fs_error recursive_create_dir(const string& path);

    // create a file, if the some dir of the paths are not exist, create them
    // \throw aio::io::create_failed
	io::file recursive_create(const string& path, int flag);

    // split the path into dir part and file name part. return the dir part, fill the file name into out parameter file.
    string dir_filename(const string& path, string* file = 0);

    // split the file path into dir and file name part,  and ext file name part. return the ext, fill the file path into parameter file.
    string ext_filename(const string& path, string* file = 0);

    string ext_filename(char separator, const string& path, string* file = 0);
}}

#endif //end AIO_COMMON_FS_UTILITY_H
