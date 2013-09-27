#ifndef AIO_XIRANG_VFS_LOCAL_FS
#define AIO_XIRANG_VFS_LOCAL_FS

#include <xirang/vfs.h>
#include <xirang/io/file.h>

namespace xirang{ namespace vfs{

	class LocalFs : public IVfs
	{
	public:

		explicit LocalFs(file_path dir);

		~LocalFs();

		// common operations of dir and file
		// \pre !absolute(path)
		virtual fs_error remove(file_path path);

		// dir operations
		// \pre !absolute(path)
		virtual fs_error createDir(file_path path);

		// file operations
		io::file writeOpen(file_path path, int flag);
		io::file_reader readOpen(file_path path);

		// \pre !absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		// otherwise, from should be a
		virtual fs_error copy(file_path from, const string& to);

		virtual fs_error truncate(file_path path, long_size_t s);

		virtual void sync();

		// query
		virtual const string& resource() const;

		// volume
		// if !mounted, return null
		virtual RootFs* root() const;

		// \post mounted() && root() || !mounted() && !root()
		virtual bool mounted() const;

		// \return mounted() ? absolute() : empty() 
		virtual sub_file_path mountPoint() const;

		// \pre !absolute(path)
		virtual VfsNodeRange children(file_path path) const;

		// \pre !absolute(path)
		virtual VfsState state(file_path path) const;

		virtual void** do_create(unsigned long long mask,
				void** base, unique_ptr<void>& owner, file_path path, int flag);
	private:
		// if r == null, means unmount
		virtual void setRoot(RootFs* r);

		RootFs* m_root;
		file_path m_resource;


		LocalFs(const LocalFs&); //disable
		LocalFs& operator=(const LocalFs&);//disable

	};

}}

#endif //end AIO_XIRANG_VFS_LOCAL_FS

