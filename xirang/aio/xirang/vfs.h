#ifndef AIO_XIRANG_VFS_H
#define AIO_XIRANG_VFS_H

#include <aio/xirang/xrbase.h>
#include <aio/common/iarchive.h>
#include <aio/common/fsutility.h>

// STD
#include <memory>

namespace xirang { namespace fs
{
	class IVfs;
	class RootFs;
	class RootFsImp;

    using aio::fs::fs_error;
    using aio::fs::file_state;
    using aio::any;

    namespace aiofs = aio::fs;

	struct VfsNode
	{
		string path;
		IVfs* owner_fs;
	};

	struct MountInfo
	{
		string path;
		IVfs* fs;
	};

	struct VfsState
	{
		VfsNode node;
		file_state state;
		aio::long_size_t  size;
	};
    enum vfs_option
    {
        vo_default = 0,
        vo_readonly,
        vo_sync_on_destroy,
        vo_user = 1 << 16
    };

	typedef RangeT<aio::const_itr_traits<MountInfo> > VfsRange;
	typedef RangeT<aio::const_itr_traits<VfsNode> > VfsNodeRange;

    using aio::archive::iarchive;
    using aio::archive::archive_deletor;
	using aio::archive::archive_ptr;

    /// \notes 1. all vfs modifications will return er_ok or not null archive_ptr if succeeded.
    ///     2. the path or file name must be aio style, separated by "/".
    ///     3. supported file name character set is depends on vfs implementation.
    ///     4. the returned fs_error is implementation defined, if user just depends on IVfs, should not assume 
    ///         which error code will be returned, except return er_ok case. but, for a known vfs imp, the error code should be determinated.
    /// 
	class IVfs
	{
		public:
		/// common operations of dir and file
        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \notes the result of removeing a opended file is depends on implementation capability.. 
		virtual fs_error remove(const string& path) = 0;

		/// create dir
        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \notes if the parent of path is not exist, failed.
        /// \notes the result depends on implementation capability.
		virtual fs_error createDir(const  string& path) = 0;

		/// create or open file
        /// \param compound flag of aio::archive::archive_mode
        /// \param flag one of aio::archive::open_flag
        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \notes the capability of returned archive_ptr must greater than or equal to given mode. 
        ///         but if user code just depends on IVfs, should not assume the addational capability not specified by given mode
        /// \notes if the parent of path is not exist, failed.
        /// \notes the result depends on implementation capability.
		virtual archive_ptr create(const string& path, int mode, int flag) = 0;


        /// copy file via file path
        /// \pre path must not end with '/'. 
		/// \pre !absolute(to)
        /// \pre !absolute(from) || mounted()
		/// if from and to in same fs, it may have a more effective implementation
        /// \notes copy to self is imp defined.
		virtual fs_error copy(const string& from, const string& to) = 0;

        /// truncate a file with given size
        /// \pre path must not end with '/'. 
		/// \pre !absolute(to)
		virtual fs_error truncate(const string& path, aio::long_size_t s) = 0;

        /// flush buffered data to underly media. implementation defined.
		virtual void sync() = 0;
		
		/// query
		virtual const string& resource() const = 0;

		/// volume
		/// if !mounted, return null
		virtual RootFs* root() const = 0;

		/// \post mounted() && root() || !mounted() && !root()
		virtual bool mounted() const = 0;

		/// \return mounted() ? absolute() : empty() 
		virtual string mountPoint() const = 0;

        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \return returned path in VfsNode is file name, not full path.
		virtual VfsNodeRange children(const string& path) const = 0;

        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \return returned path in VfsNode is full path. it's different from children()
		virtual VfsState state(const string& path) const = 0;
	
        /// get the option with given id and data
        /// if failed, return a empty any, otherwise depends on the input value.
        /// all options are implementation defined
        virtual any getopt(int id, const any & optdata = any() ) const ;

        /// set the option with given id, option data
        /// if failed, return a empty any, otherwise depends the input value.
        /// all options are implementation defined
        virtual any setopt(int id, const any & optdata,  const any & indata= any());

		virtual ~IVfs();

	private:
		// if r == null, means unmount, used by RootFs only
		virtual void setRoot(RootFs* r) = 0;

		friend class RootFsImp;
	};

	class RootFs 
	{
	public:	
		explicit RootFs(const string& res);
		~RootFs();

		// \pre dir must be absolute name
		fs_error mount(const string& dir, IVfs& vfs);
		fs_error unmount(const string& dir);

		void sync();

		// query
		const string& resource() const;

		// volume
		// return absolute name
		string mountPoint(const IVfs& p) const;
		IVfs* mountPoint(const string& p) const;

		VfsRange mountedFS() const;
		VfsState locate(const string& path) const;

        // return true if path contains mount pointer in direct/indirect subdir, excluding self.
		bool containMountPoint(const string& path) const;

	private:
		RootFsImp* m_imp;
	};

    using aio::fs::append_tail_slash;
	using aio::fs::normalize;
	using aio::fs::is_normalized;
    using aio::fs::is_filename;
    using aio::fs::dir_filename;
    using aio::fs::ext_filename;

    // return true if p[0] is '/';
	extern bool is_absolute(const string& p);

    archive_ptr temp_file(IVfs& vfs, aio::const_range_string template_ , aio::const_range_string dir, int flag = aio::archive::of_remove_on_close, string* path = 0);
    string temp_dir(IVfs& vfs, aio::const_range_string template_, aio::const_range_string parent_dir);
    fs_error recursive_remove(IVfs&vfs, const string& path);
    fs_error recursive_create_dir(IVfs&vfs, const string& path);
    archive_ptr recursive_create(IVfs&vfs, const string& path, int mode, int flag);
}
}

#endif //end AIO_XIRANG_VFS_H

