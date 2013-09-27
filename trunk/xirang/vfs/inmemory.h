#ifndef AIO_XIRANG_INMEMORY_FS
#define AIO_XIRANG_INMEMORY_FS

#include <xirang/vfs.h>
#include <xirang/io/memory.h>

namespace xirang{ namespace vfs{

	class InMemoryImp;
	class InMemory : public IVfs
	{
	public:

        explicit InMemory(const string& resource = string());

		~InMemory();

		// common operations of dir and file
		// \pre !absolute(path)
		virtual fs_error remove(sub_file_path path);

		// dir operations
		// \pre !absolute(path)
		virtual fs_error createDir(sub_file_path path);

		// file operations
		io::buffer_io writeOpen(sub_file_path path, int flag);
		io::buffer_in readOpen(sub_file_path path);

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

        virtual any getopt(int id, const any & optdata = any() ) const ;
        virtual any setopt(int id, const any & optdata,  const any & indata= any());

		virtual void** do_create(unsigned long long mask,
				void** base, unique_ptr<void>& owner, sub_file_path path, int flag);
	private:
		// if r == null, means unmount
		virtual void setRoot(RootFs* r);
		InMemoryImp* m_imp;
	};

}}
#endif //end AIO_XIRANG_INMEMORY_FS



