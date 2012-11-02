#ifndef AIO_XIRANG_INMEMORY_FS
#define AIO_XIRANG_INMEMORY_FS

#include <aio/xirang/vfs.h>

namespace xirang{ namespace fs{

	class InMemoryImp;
	class InMemory : public IVfs
	{
	public:

        explicit InMemory(const string& resource = aio::empty_str);

		~InMemory();

		// common operations of dir and file
		// \pre !absolute(path)
		virtual fs_error remove(const string& path);

		// dir operations
		// \pre !absolute(path)
		virtual fs_error createDir(const  string& path);

		// file operations
		virtual archive_ptr create(const string& path, int mode, int flag);

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
		InMemoryImp* m_imp;
	};

}
}
#endif //end AIO_XIRANG_INMEMORY_FS



