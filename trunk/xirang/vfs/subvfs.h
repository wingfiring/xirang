#ifndef AIO_XIRANG_VFS_SUBVFS
#define AIO_XIRANG_VFS_SUBVFS

#include <xirang/vfs.h>

namespace xirang{ namespace vfs{

	class SubVfs : public IVfs
	{
	public:

		explicit SubVfs(IVfs& parent, sub_file_path dir);

		~SubVfs();

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
		virtual sub_file_path mountPoint() const;

		// \pre !absolute(path)
		virtual VfsNodeRange children(sub_file_path path) const;

		// \pre !absolute(path)
		virtual VfsState state(sub_file_path path) const;

        IVfs& parentFs() const;

		virtual void** do_create(unsigned long long mask,
				void* ret, unique_ptr<void>& owner, sub_file_path path, int flag);
	private:
		// if r == null, means unmount
		virtual void setRoot(RootFs* r);

        RootFs* m_root;
        IVfs& parent;
        file_path m_resource;
    

        SubVfs(const SubVfs&); //disable
        SubVfs& operator=(const SubVfs&);//disable

	};

}}

#endif //end AIO_XIRANG_VFS_SUBVFS

