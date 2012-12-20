#ifndef AIO_XIRANG_INMEMORY_FS
#define AIO_XIRANG_INMEMORY_FS

#include <aio/xirang/vfs.h>
#include <aio/common/archive/mem_archive.h>

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
		aio::io::buffer_io create(const string& path, int flag);
		aio::io::buffer_in readOpen(const string& path);

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

		virtual void** do_create(unsigned long long mask,
				void** base, aio::unique_ptr<void>& owner, const string& path, int flag){
			using namespace aio::io;

			void** ret = 0;
			if (mask & io::get_mask<writer, write_map>::value ){ //write open
				aio::unique_ptr<buffer_io> ar(new buffer_io(aio::get_cobj<InMemory>(this).create(path, flag)));
				ret = copy_interface<reader, writer, random, ioctrl, read_map, write_map>::apply(mask, base, *ar, (void*)ar.get()); 
				aio::unique_ptr<void>(std::move(ar)).swap(owner);
			}
			else{ //read open
				aio::unique_ptr<buffer_in> ar(new buffer_in(aio::get_cobj<InMemory>(this).readOpen(path)));
				ret = copy_interface<reader, random, ioctrl, read_map>::apply(mask, base, *ar, (void*)ar.get()); 
				aio::unique_ptr<void>(std::move(ar)).swap(owner);
			}
			return ret;
		}
	private:
		// if r == null, means unmount
		virtual void setRoot(RootFs* r);
		InMemoryImp* m_imp;
	};

}}
#endif //end AIO_XIRANG_INMEMORY_FS



