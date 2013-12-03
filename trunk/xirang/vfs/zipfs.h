#ifndef XIRANG_ZIP_VFS_H_
#define XIRANG_ZIP_VFS_H_

#include <xirang/vfs.h>
#include <xirang/io.h>

namespace xirang{ namespace vfs{

	class ZipFsImp;
	class ZipFs : public IVfs
	{
	public:
        explicit ZipFs(const iref<io::read_map, io::write_map>& ar, IVfs* cache, const string& resource = string());
        explicit ZipFs(iauto<io::read_map, io::write_map> ar, IVfs* cache, const string& resource = string());
        explicit ZipFs(const iref<io::read_map>& ar, IVfs* cache, const string& resource = string());
        explicit ZipFs(iauto<io::read_map> ar, IVfs* cache, const string& resource = string());

		~ZipFs();

		// common operations of dir and file
		// \pre !absolute(path)
		virtual fs_error remove(sub_file_path path);

		// dir operations
		// \pre !absolute(path)
		virtual fs_error createDir(sub_file_path path);

		// \pre !absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		// otherwise, from should be a
		virtual fs_error copy(sub_file_path from, sub_file_path to);

		virtual fs_error truncate(sub_file_path path, long_size_t s);

		virtual void sync();

		// query
		virtual const string& resource() const;

		// volume
		// if !mounted, return null
		virtual RootFs* root() const;

		// \post mounted() && root() || !mounted() && !root()
		virtual bool mounted() const;

		// \return mounted() ? absolute() : empty() 
		virtual file_path mountPoint() const;

		// \pre !absolute(path)
		virtual VfsNodeRange children(sub_file_path path) const;

		// \pre !absolute(path)
		virtual VfsState state(sub_file_path path) const;

		virtual void** do_create(unsigned long long mask,
				void** base, unique_ptr<void>& owner, sub_file_path path, int flag);

	private:
		// if r == null, means unmount
		virtual void setRoot(RootFs* r);

		unique_ptr<ZipFsImp> m_imp;

		ZipFs(const ZipFs&) = delete;
		ZipFs& operator=(const ZipFs&) = delete;

	};

}}

#endif //end XIRANG_ZIP_VFS_H_

