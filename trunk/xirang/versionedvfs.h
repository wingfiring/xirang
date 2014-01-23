#ifndef XIRANG_VERSIONED_VFS_H
#define XIRANG_VERSIONED_VFS_H

#include <xirang/vfs.h>
#include <xirang/versiontype.h>

namespace xirang{ namespace vfs{
	typedef string MimeType;

	enum BlobType{
		bt_tree,
		bt_file,
		bt_submission,
	};

	struct Submision{
		version_type version;
		string name;
		string description;
	};

	struct FileInfo{
		version_type version;
		version_type submission;
		MimeType type;
		file_path name;	//full path name in that submission
	};

	// history is a list of submission.
	class FileHistory{	// only the history of specifed file path;
	};

	typedef BiRangeT<const_itr_traits<file_path> > FilePathRange;
	typedef BiRangeT<const_itr_traits<Submision> > SubmissionRange;

	class IWorkspace;
	class IRepository;

	// the IVfs part works on repository only.
	// a/~repo/b/c/#ver
	// a/~repo/b/c/#ver/d
	// IVfs should avoid forward the call to IRepository. init IRepository maybe expensive.
	class IRepository : public IVfs{
		public:
		// history of root will get all submission;
		virtual const FileHistory& history(const file_path& p) const = 0;
		virtual const Submision& getHead() const = 0;

		virtual version_type getFileVersion(const version_type& commit_id, const file_path& p) = 0;
		virtual void push() = 0;
		// advanced API
		virtual void pull() = 0;

		//TODO: need more detail design
		virtual void fetch(const version_type& ver, int level) = 0;

		virtual IVfs& getOwnerFs() const = 0;

		/// requires to create workspace on demand, so this method is not const
		/// TODO: The returned workspace should be shared_ptr
		virtual IWorkspace* getWorkspace() = 0;
		protected:
			virtual ~IRepository();
	};

	extern void checkOut(IRepository&, const file_path& p, const version_type& ver);
	extern void checkOut(IRepository&, const file_path& p);

	class IWorkspace : public IVfs{
		public:
		// return commit id;
		// it'll commit and earse workspace;
		// default Workspace will be recreated automatically.
		// after commit, getBase() will return commit id
		// if the IVfs is empty and removedList() is empty too, this method do nothing and just return base;
		virtual Submision commit(const string& description) = 0;

		// reserved
		// Currently, IRepository only  works against HEAD. in the future, it may work base any submission
		// Set base of workspace
		// Set the base submission id
		virtual bool setBase(const version_type& ver) = 0;
		virtual const version_type& getBase() const = 0;

		// mark p will be removed in next commit.
		// the function only remove the file from next commit, not remove it from workspace
		// if syncWorkspace, it'll remove the file from workspace.
		// after this call
		virtual fs_error removeFromBase(const file_path& p, bool syncFs) = 0;
		virtual fs_error cancelRemoveFromBase(const file_path& p) = 0;
		virtual bool isMarkedRemove(const file_path& p) const = 0;
		virtual FilePathRange removedList() const = 0;

		virtual	IRepository& getRepository() const = 0;
		protected:
		virtual ~IWorkspace();

	};


	/// This class works on local disk, it will be used by server.
	class LocalVersionedFs : public IVfs
	{
	public:
		// IVfs API
		virtual fs_error remove(sub_file_path path);
		virtual fs_error createDir(sub_file_path path);
		virtual fs_error copy(sub_file_path from, sub_file_path to);
		virtual fs_error truncate(sub_file_path path, long_size_t s);
		virtual void sync();
		virtual const string& resource() const;
		virtual RootFs* root() const;
		virtual bool mounted() const;
		virtual file_path mountPoint() const;
		virtual VfsNodeRange children(sub_file_path path) const;
		virtual VfsState state(sub_file_path path) const;
        virtual any getopt(int id, const any & optdata = any() ) const ;
        virtual any setopt(int id, const any & optdata,  const any & indata= any());
		virtual void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag);

		virtual IRepository* getRepository(const file_path& p, file_path* rest, file_path* repo_path);
		virtual void releaseRepository(const file_path& p);
	private:
		virtual void setRoot(RootFs* r);
	};

	class LocalRepository : public IRepository{
	public:
		virtual const FileHistory& history(const file_path& p) const;
		virtual const Submision& getHead() const;
		virtual version_type getFileVersion(const version_type& commit_id, const file_path& p);
		virtual void push();
		virtual void pull();
		virtual void fetch(const version_type& ver, int level);
		virtual IVfs& getOwnerFs() const;
		virtual IWorkspace* getWorkspace();
	};


	/// this interface design is optimized for network access.
	/// it provide coarse-grained methods, such as batch access.
	/// NetVersionedFs should expose these APIs indirectly with/without caching (controled by app)
	/// so that app can control for more smooth UI workflow.
	class INetVersionedFsDriver
	{
		public:
	};
	class NetVersionedFs : public IVfs
	{
	public:
		// IVfs API
		virtual fs_error remove(sub_file_path path);
		virtual fs_error createDir(sub_file_path path);
		virtual fs_error copy(sub_file_path from, sub_file_path to);
		virtual fs_error truncate(sub_file_path path, long_size_t s);
		virtual void sync();
		virtual const string& resource() const;
		virtual RootFs* root() const;
		virtual bool mounted() const;
		virtual file_path mountPoint() const;
		virtual VfsNodeRange children(sub_file_path path) const;
		virtual VfsState state(sub_file_path path) const;
        virtual any getopt(int id, const any & optdata = any() ) const ;
        virtual any setopt(int id, const any & optdata,  const any & indata= any());
		virtual void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag);

		virtual IRepository* getRepository(const file_path& p, file_path* rest, file_path* repo_path);
		virtual void releaseRepository(const file_path& p);
	private:
		virtual void setRoot(RootFs* r);

	};

	/// this class can be used by both client & proxy.
	class NetRepository : public IRepository
	{
		public:
		// net repository, the history() should not expect complete list?
		virtual const FileHistory& history(const file_path& p) const;
		virtual const Submision& getHead() const;
		virtual version_type getFileVersion(const version_type& commit_id, const file_path& p);
		virtual void push();
		virtual void pull();
		virtual void fetch(const version_type& ver, int level);
		virtual IVfs& getOwnerFs() const;
		virtual IWorkspace* getWorkspace();

		virtual INetVersionedFsDriver& getDriver() const;
	};

}}

#endif //end XIRANG_VERSIONED_VFS_H

