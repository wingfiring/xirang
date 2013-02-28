#ifndef AIO_XIRANG_VFS_ZIP_FS
#define AIO_XIRANG_VFS_ZIP_FS

#include <aio/xirang/vfs.h>

namespace xirang{ namespace fs{

	class ZipFsImp;
	enum CachePolicy
	{
		cp_flat,
		cp_tree
	};
	class ZipFs : public IVfs
	{
	public:

        explicit ZipFs(aio::iref<aio::io::read_map> file, IVfs& cache, const string& resource = aio::empty_str, CachePolicy cp = cp_flat);
        explicit ZipFs(aio::iref<aio::io::read_map, aio::io::write_map> file, IVfs& cache
				, const string& resource = aio::empty_str, bool sync_on_destroy = true, CachePolicy cp = cp_flat);

		~ZipFs();

		// common operations of dir and file
		// \pre !absolute(path)
		virtual fs_error remove(const string& path);

		// dir operations
		// \pre !absolute(path)
		virtual fs_error createDir(const  string& path);

		virtual void** do_create(unsigned long long mask,
				void** base, aio::unique_ptr<void>& owner, const string& path, int flag);

		// \pre !absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		// otherwise, from should be a
		virtual fs_error copy(const string& from, const string& to);

		virtual fs_error truncate(const string& path, aio::long_size_t s);

		virtual void sync();

		// query
		virtual const string& resource() const;

		// volume
		// if !mounted, return null
		virtual RootFs* root() const;

		// \post mounted() && root() || !mounted() && !root()
		virtual bool mounted() const;

		// \return mounted() ? absolute() : empty() 
		virtual string mountPoint() const;

		// \pre !absolute(path)
		virtual VfsNodeRange children(const string& path) const;

		// \pre !absolute(path)
		virtual VfsState state(const string& path) const;

        virtual any getopt(int id, const any & optdata = any() ) const ;

        virtual any setopt(int id, const any & optdata,  const any & indata= any());
	private:
		// if r == null, means unmount
		virtual void setRoot(RootFs* r);
		ZipFsImp* m_imp;
	};

	AIO_EXCEPTION_TYPE(bad_central_file_header_signature);
	AIO_EXCEPTION_TYPE(bad_no_enough_head_content);
	AIO_EXCEPTION_TYPE(bad_end_central_dir_signature);
	AIO_EXCEPTION_TYPE(bad_central_dir_offset_or_size);
	AIO_EXCEPTION_TYPE(bad_central_dir);
	AIO_EXCEPTION_TYPE(bad_local_file_header_signature);
	AIO_EXCEPTION_TYPE(bad_local_file_header_offset_or_size);
	AIO_EXCEPTION_TYPE(archive_io_fatal_error);
	AIO_EXCEPTION_TYPE(duplicated_file_name);
	AIO_EXCEPTION_TYPE(archive_runtime_error);
	AIO_EXCEPTION_TYPE(vfs_runtime_error);
	AIO_EXCEPTION_TYPE(bad_zip_entry_name);

}
}

#endif //end AIO_XIRANG_VFS_ZIP_FS

