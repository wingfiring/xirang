#ifndef XIRANG_VERSIONED_VFS_H
#define XIRANG_VERSIONED_VFS_H

#include <xirang/vfs.h>
#include <xirang/versiontype.h>

namespace xirang{ namespace vfs{
	typedef string MimeType;

	enum blob_type{
		bt_file,
		bt_tree,
		bt_submission
	};
	struct Submission {
		uint32_t flag;
		version_type prev;
		version_type tree;
		int64_t time;
		string author, submitter, description;
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
	typedef BiRangeT<const_itr_traits<Submission> > SubmissionRange;

	class IWorkspace;
	class IRepository;

	class IVersionedFs : public IVfs{
	public:
		virtual IVfs& underlying() const = 0;
	protected:
		virtual ~IVersionedFs(){}
	};

	class IRepoManager{
	public:
		virtual std::shared_ptr<IRepository> getRepo(IVfs& vfs, sub_file_path repoPath) = 0;
		/// \param path resource path for detect
		/// \param repoPath  return the path of repo. if the path doesn't contain repo, repoPath will be most match part.
		/// \param pathInRepo if path contains a repo, it return the rest part of path. if it doesn't contain repo, it's the mismatch part.
		/// \return return true if path contains a repo, if not return false
		virtual bool extractRepoPath(IVfs& vfs, sub_file_path path, file_path* repoPath, file_path* pathInRepo) const = 0;
	};

	class RepoManager : public IRepoManager{
	public:
		virtual std::shared_ptr<IRepository> getRepo(IVfs& vfs, sub_file_path repoPath);
		virtual bool extractRepoPath(IVfs& vfs, sub_file_path path, file_path* repoPath, file_path* pathInRepo) const;
	};

	AIO_EXCEPTION_TYPE(bad_repository_exception);

	// the IVfs part works on repository only.
	// a/~repo/b/c/#ver
	// a/~repo/b/c/#ver/d
	// IVfs should avoid forward the call to IRepository. init IRepository maybe expensive.
	class IRepository : public IVfs{
		public:
		// history of root will get all submission;
		virtual const FileHistory& history(const file_path& p) const = 0;
		virtual const Submission* getSubmission(const version_type& ver) const = 0;

		virtual version_type getFileVersion(const version_type& commit_id, const file_path& p) = 0;
		virtual void push() = 0;
		// advanced API
		virtual void pull() = 0;

		//TODO: need more detail design
		virtual void fetch(const version_type& ver, int level) = 0;

		virtual IVfs& underlying() const = 0;
		virtual const file_path& prefix() const = 0;

		// return commit id;
		// it'll commit and earse workspace;
		// default Workspace will be recreated automatically.
		// after commit, getBase() will return commit id
		// if the IVfs is empty and removedList() is empty too, this method do nothing and just return base;
		virtual const Submission& commit(IWorkspace& wk, const string& description, const Submission& base) = 0;
		protected:
			virtual ~IRepository();
	};

	class IWorkspace : public IVfs{
		public:
		// mark p will be removed in next commit.
		// the function only remove the file from next commit, not remove it from workspace
		// if syncWorkspace, it'll remove the file from workspace.
		// after this call
		virtual fs_error markRemove(const file_path& p) = 0;
		virtual fs_error unmarkRemove(const file_path& p) = 0;
		virtual bool isMarkedRemove(const file_path& p) const = 0;
		virtual FilePathRange removedList() const = 0;

		protected:
		virtual ~IWorkspace();

	};

	/// This class works on local disk, it will be used by server.
	class LocalVersionedFsImp;
	class LocalVersionedFs : public IVersionedFs
	{
	public:
		LocalVersionedFs(IRepoManager& repoManager, IVfs& local, const file_path& prefix);
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

		/// \param path can be "~repo/a/b/#version"
		/// FUTURE: or  "~repo/<#submission or root version>/a/b" or "~repo/a/<#tree version>/b"
		virtual VfsNodeRange children(sub_file_path path) const;
		virtual VfsState state(sub_file_path path) const;
        virtual any getopt(int id, const any & optdata = any() ) const ;
        virtual any setopt(int id, const any & optdata,  const any & indata= any());
		virtual void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag);

		virtual std::shared_ptr<IRepository> getRepository(const file_path& p, file_path* rest, file_path* repo_path);

		virtual IVfs& underlying() const;

		virtual ~LocalVersionedFs();
	private:
		virtual void setRoot(RootFs* r);
		LocalVersionedFs(const LocalVersionedFs&) = delete;
		LocalVersionedFs& operator=(const LocalVersionedFs&) = delete;

		unique_ptr<LocalVersionedFsImp> m_imp;
	};


	class LocalRepositoryImp;
	class LocalRepository : public IRepository{
	public:
		LocalRepository(IVfs& vfs, const file_path& prefix);
		// from IVfs
		// remove, createDir, copy, and truncate should be failed since history is not editable.
		virtual fs_error remove(sub_file_path path);
		virtual fs_error createDir(sub_file_path path);
		virtual fs_error copy(sub_file_path from, sub_file_path to);
		virtual fs_error truncate(sub_file_path path, long_size_t s);

		virtual void sync();
		virtual const string& resource() const;
		virtual RootFs* root() const;
		virtual bool mounted() const;
		virtual file_path mountPoint() const;

		/// if path doesn't end with version string (#<sha1 digest>), it should return sub items in given HEAD.
		/// if with version, it only return the children in given version directory.
		virtual VfsNodeRange children(sub_file_path path) const;

		virtual VfsState state(sub_file_path path) const;
        virtual any getopt(int id, const any & optdata = any() ) const ;
        virtual any setopt(int id, const any & optdata,  const any & indata= any());
		virtual void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag);
		virtual void setRoot(RootFs* r);

		// from IRepository
		virtual const FileHistory& history(const file_path& p) const;
		virtual const Submission* getSubmission(const version_type& ver) const;
		virtual version_type getFileVersion(const version_type& commit_id, const file_path& p);
		virtual void push();
		virtual void pull();
		virtual void fetch(const version_type& ver, int level);
		virtual IVfs& underlying() const;
		virtual const file_path& prefix() const;
		virtual const Submission& commit(IWorkspace& wk, const string& description, const Submission& base);
	private:
		unique_ptr<LocalRepositoryImp> m_imp;

	};

	class Workspace : public IWorkspace {
	public:
		virtual fs_error markRemove(const file_path& p);
		virtual fs_error unmarkRemove(const file_path& p);
		virtual bool isMarkedRemove(const file_path& p) const;
		virtual FilePathRange removedList() const;
	};


	/// this interface design is optimized for network access.
	/// it provide coarse-grained methods, such as batch access.
	/// NetVersionedFs should expose these APIs indirectly with/without caching (controled by app)
	/// so that app can control for more smooth UI workflow.
	class INetVersionedFsDriver
	{
		public:
	};
	class NetVersionedFs : public IVersionedFs
	{
	public:
		NetVersionedFs(IVfs& local, INetVersionedFsDriver& driver, const file_path& prefix);
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

		virtual std::shared_ptr<IRepository> getRepository(const file_path& p, file_path* rest, file_path* repo_path);
		virtual IVfs& underlying() const;
	private:
		virtual void setRoot(RootFs* r);

	};

	/// this class can be used by both client & proxy.
	class NetRepository : public IRepository
	{
		public:
		// net repository, the history() should not expect complete list?
		virtual const FileHistory& history(const file_path& p) const;
		virtual const Submission* getSubmission(const version_type& ver) const;
		virtual version_type getFileVersion(const version_type& commit_id, const file_path& p);
		virtual void push();
		virtual void pull();
		virtual void fetch(const version_type& ver, int level);
		virtual IVfs& underlying() const;
		virtual const file_path& prefix() const;
		virtual const Submission& commit(IWorkspace& wk, const string& description, const Submission& base);

		virtual INetVersionedFsDriver& getDriver() const;
	};

}}

#endif //end XIRANG_VERSIONED_VFS_H

