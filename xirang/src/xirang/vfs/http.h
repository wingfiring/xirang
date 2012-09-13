#ifndef AIO_XIRANG_VFS_HTTP_FS
#define AIO_XIRANG_VFS_HTTP_FS

namespace xirang{ namespace fs{

	class HttpFsImp;
	class HttpFs : public IVfs
	{
	public:

		explicit HttpFs(const string& dir);

		~HttpFs();

		// common operations of dir and file
		// \pre !absolute(path)
		virtual fs_error remove(const string& path);

		// dir operations
		// \pre !absolute(path)
		virtual fs_error createDir(const  string& path);

		// file operations
		virtual archive_ptr create(const string& path, archive_mode mode, open_flag flag);

		// \pre !absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		// otherwise, from should be a
		virtual fs_error copy(const string& from, const string& to);

		virtual fs_error truncate(const string& path, long_size_t s);

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
	private:
		// if r == null, means unmount
		virtual void setRoot(RootFs* r);
		HttpFsImp* m_imp;
	};

}
}
#endif //end AIO_XIRANG_VFS_HTTP_FS


