#ifndef AIO_XIRANG_VFS_SUBVFS
#define AIO_XIRANG_VFS_SUBVFS

#include <aio/xirang/vfs.h>

namespace xirang{ namespace fs{

	class SubVfs : public IVfs
	{
	public:

		explicit SubVfs(IVfs& parent, const string& dir);

		~SubVfs();

		// common operations of dir and file
		// \pre !absolute(path)
		virtual fs_error remove(const string& path);

		// dir operations
		// \pre !absolute(path)
		virtual fs_error createDir(const  string& path);

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

        IVfs& parentFs() const;

		virtual void** do_create(unsigned long long mask,
				void* ret, aio::unique_ptr<void>& owner, const string& path, int flag);
	private:
		// if r == null, means unmount
		virtual void setRoot(RootFs* r);

        RootFs* m_root;
        IVfs& parent;
        string m_resource;
    

        SubVfs(const SubVfs&); //disable
        SubVfs& operator=(const SubVfs&);//disable

	};

}}

#endif //end AIO_XIRANG_VFS_SUBVFS

