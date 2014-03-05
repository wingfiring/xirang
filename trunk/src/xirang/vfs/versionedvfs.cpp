#include <xirang/versionedvfs.h>
#include <xirang/vfs/vfs_common.h>
#include <xirang/io/exchs11n.h>
#include <xirang/io/versiontype.h>
#include <xirang/io/path.h>

#include <tuple>
#include <unordered_map>
#include <set>
#include <map>

namespace xirang{ namespace vfs{

	struct blob_info{
		uint32_t flag;
		version_type version;
		string name;
		uint64_t size;
	};

	template<typename Ar>
	Ar& load(Ar& ar, blob_info& tr){
		ar & tr.flag & tr.version & tr.name & tr.size;
		return ar;
	}

	template<typename Ar>
	Ar& save(Ar& ar, const blob_info& tr){
		ar & tr.flag & tr.version & tr.name & tr.size;
		return ar;
	}

	struct blob_idx_item{
		blob_info info;
		long_size_t offset;	// -1 means separated blob file.
	};

	struct blob_idx{
		typedef std::unordered_map<version_type, blob_idx_item, hash_version_type> type;
		type items;
	};

	template<typename Ar>
	Ar& load(Ar& ar, blob_idx& idx){
		blob_idx_item idx_item;
		version_type version;
		while (ar.readable()){
			ar & version & idx_item.info & idx_item.offset;
			idx.items.insert(std::make_pair(version, idx_item));
		}
		return ar;
	}

	template<typename Ar>
	Ar& save(Ar& ar, const blob_idx& idx){
		for (auto & i: idx.items){
			ar & i.first & i.second.info & i.second.offset;
		}
		return ar;
	}

	template<typename Ar>
	Ar& load(Ar& ar, Submission& sub){
		ar & sub.flag & sub.prev & sub.tree & sub.time
			& sub.author & sub.submitter & sub.description;
		return ar;
	}

	template<typename Ar>
	Ar& save(Ar& ar, const Submission& sub){
		ar & sub.flag & sub.prev & sub.tree & sub.time
			& sub.author & sub.submitter & sub.description;
		return ar;
	}

	struct tree_blob {
		uint32_t flag;
		typedef std::unordered_map<string, version_type, hash_string> items_type;
		items_type items;
	};
	template<typename Ar>
	Ar& load(Ar& ar, tree_blob& tb){
		size_t s = 0 ;
		ar & tb.flag & s;
		tb.items.reserve(s);
		for (size_t i = 0; i < s; ++i){
			auto record = load<version_type>(ar);
			tb.items.insert(std::make_pair(record.name, record));
		}
		return ar;
	}

	template<typename Ar>
	Ar& save(Ar& ar, const tree_blob& tb){
		ar & tb.flag & tb.items.size();
		for (auto &i : tb.items)
			ar & i;
		return ar;
	}

	typedef std::set<version_type> path_versions_type;
	struct path_map_type{
		std::map<file_path, path_versions_type, path_less>  items;
	};

	template<typename Ar>
	Ar& load(Ar& ar, path_map_type& tb){
		while (ar.readable()){
			file_path p;
			ar & p;
			auto & dest = tb.items[p];
			auto s = load<size_t>(ar);
			for (size_t i = 0; i < s; ++i){
				dest.insert(load<version_type>(ar));
			}
		}
		return ar;
	}

	template<typename Ar>
	Ar& save(Ar& ar, const path_map_type& tb){
		for (auto& i :  tb.items){
			ar & i.first;
			ar & i.second.size();
			for (auto& m : i.second) ar & m;
		}
		return ar;
	}



	// in each repo, there is a blob store, it contains all files & trees & submission's blob.
	//
	// in the repo, there is a shadowed file tree contains all history dirs.
	// all files stored in object store, nameless
	// In each folder, place a index file to maintain the item history
	//
	// the format+
	//   submission	name	file_type version	flag
	//
	// for remove:
	// 	just need to new a parent tree blob which doesn't contain the given file,
	//
	// workflow:
	// 1. get base submit
	// 2. get root tree
	// 3. walkthrough each file in workspace
	// 4. for each file, or remove, add to shadow fs, and merge to it's parent tree, save each tree blob to object store
	// 5. repeat 4 untile root
	// 6. save commit
	// 7. set local HEAD to the commit;
	//version_type IVersionedVfs::commitWorkspace()



	// push:
	// check remote HEAD localy, if remote HEAD == local HEAD , no action
	// check remote HEAD from server, if server is not remote HEAD, failed, end require pull & merge.
	// then package all commit between remote HEAD & local HEAD. //send object first, send tree & commit blob last.
	// send the package.
	// after success, set remote HEAD with local HEAD.

	class LocalVersionedFsImp
	{
	public:
		LocalVersionedFsImp(IRepoManager& repoManager, IVfs& fs, const file_path& prefix, IVfs* host)
			: m_repoManager(repoManager), m_underlying(fs), m_prefix(prefix), m_host(host)
			, m_root()

		{
		}
		// IVfs API
		fs_error remove(sub_file_path path){
			AIO_PRE_CONDITION(!path.is_absolute());
			if (is_in_repo_path_(path))
				return fs::er_permission_denied;
			return m_underlying.remove(path);
		}
		fs_error createDir(sub_file_path path){
			AIO_PRE_CONDITION(!path.is_absolute());
			if (is_in_repo_path_(path))
				return fs::er_permission_denied;
			return m_underlying.createDir(path);
		}
		fs_error copy(sub_file_path from, sub_file_path to){
			AIO_PRE_CONDITION(!to.is_absolute());
			if (is_in_repo_path_(to))
				return fs::er_permission_denied;

			VfsNode from_node = { from, m_host};
			if (from.is_absolute())
			{
				AIO_PRE_CONDITION(mounted());
				from_node = m_root->locate(from).node;
			}

			VfsNode to_node = { to, m_host};
			return xirang::vfs::copyFile(from_node, to_node);
		}
		fs_error truncate(sub_file_path path, long_size_t s){
			AIO_PRE_CONDITION(!path.is_absolute());
			if (is_in_repo_path_(path))
				return fs::er_permission_denied;

			return m_underlying.truncate(path, s);
		}
		void sync(){ m_underlying.sync(); }

		const string& resource() const { return m_prefix.str();}
		RootFs* root() const { return m_root;}
		bool mounted() const { return m_root != 0;}
		file_path mountPoint() const { return m_root ? m_root->mountPoint(*m_host) : file_path();}

		VfsNodeRange children(sub_file_path path) const{
			AIO_PRE_CONDITION(!path.is_absolute());

			file_path rest;
			auto repo = get_repo_(path, rest);
			if (!repo)
				return m_underlying.children(path);
			return repo->children(rest);
		}
		VfsState state(sub_file_path path) const{
			AIO_PRE_CONDITION(!path.is_absolute());
			file_path rest;
			auto repo = get_repo_(path, rest);
			if (!repo)
				return m_underlying.state(path);
			return repo->state(rest);
		}
		any getopt(int id, const any & optdata) const {
			return m_underlying.getopt(id, optdata);
		}
		any setopt(int id, const any & optdata,  const any & indata){
			return m_underlying.setopt(id, optdata, indata);
		}
		void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
			AIO_PRE_CONDITION(!path.is_absolute());
			file_path rest;
			auto repo = get_repo_(path, rest);
			if (!repo)
				return m_underlying.do_create(mask, ret, owner, path, flag);

			return repo->do_create(mask, ret, owner, path, flag);
		}

		~LocalVersionedFsImp(){ }
		void setRoot(RootFs* r){
			AIO_PRE_CONDITION(!mounted() || r == 0);
			m_root = r;
		}

		IVfs& underlying() const{
			return m_underlying;
		}
	private:
		std::shared_ptr<IRepository> get_repo_(sub_file_path path, file_path& rest)const{
			file_path repoPath;
			file_path mismatch;
			auto hasRepo = m_repoManager.extractRepoPath(*m_host, path, &repoPath, &rest);
			if (!hasRepo)
				return std::shared_ptr<IRepository>();
			return m_repoManager.getRepo(m_underlying, repoPath);
		}
		bool is_in_repo_path_(sub_file_path path) const{
			return m_repoManager.extractRepoPath(m_underlying, path, 0, 0);
		}

		IRepoManager& m_repoManager;
		IVfs& m_underlying;
		file_path m_prefix;
		IVfs* m_host;
		RootFs* m_root;
	};


	LocalVersionedFs::LocalVersionedFs(IRepoManager& repoManager, IVfs& local, const file_path& prefix)
		: m_imp(new LocalVersionedFsImp(repoManager, local, prefix, this))
	{
	}
	// IVfs API
	fs_error LocalVersionedFs::remove(sub_file_path path){ return m_imp->remove(path);}
	fs_error LocalVersionedFs::createDir(sub_file_path path){ return m_imp->createDir(path);}
	fs_error LocalVersionedFs::copy(sub_file_path from, sub_file_path to){ return m_imp->copy(from, to);}
	fs_error LocalVersionedFs::truncate(sub_file_path path, long_size_t s){ return m_imp->truncate(path, s);}
	void LocalVersionedFs::sync(){ m_imp->sync();}
	const string& LocalVersionedFs::resource() const { return m_imp->resource();}
	RootFs* LocalVersionedFs::root() const{ return m_imp->root();}
	bool LocalVersionedFs::mounted() const{ return m_imp->mounted();}
	file_path LocalVersionedFs::mountPoint() const{ return mountPoint();}
	VfsNodeRange LocalVersionedFs::children(sub_file_path path) const{return m_imp->children(path);}
	VfsState LocalVersionedFs::state(sub_file_path path) const{ return m_imp->state(path);}
	any LocalVersionedFs::getopt(int id, const any & optdata) const{ return m_imp->getopt(id, optdata);}
	any LocalVersionedFs::setopt(int id, const any & optdata,  const any & indata){ return m_imp->setopt(id, optdata, indata);}
	void** LocalVersionedFs::do_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
		return m_imp->do_create(mask, ret, owner, path, flag);
	}

	LocalVersionedFs::~LocalVersionedFs(){}
	void LocalVersionedFs::setRoot(RootFs* r){ return m_imp->setRoot(r);}
	IVfs& LocalVersionedFs::underlying() const{
		return m_imp->underlying();
	}

	std::shared_ptr<IRepository> RepoManager::getRepo(IVfs& vfs, sub_file_path repoPath){
		return std::shared_ptr<IRepository>(new LocalRepository(vfs, repoPath));
	}

	static const sub_file_path K_blob_idx = sub_file_path(literal("#blob.idx"));
	static const sub_file_path K_path_idx = sub_file_path(literal("#path.idx"));
	static const sub_file_path K_head = sub_file_path(literal("#head"));
	static const sub_file_path K_remote_head = sub_file_path(literal("#remote"));

	static sub_file_path rest_to_end_(sub_file_path i, sub_file_path path){
		const_range_string rest(i.str().end(), path.str().end());
		if (!rest.empty() && rest[0] == sub_file_path::dim)
			return sub_file_path(rest.begin() + 1, rest.end());
		else
			return sub_file_path(rest);
	}
	bool RepoManager::extractRepoPath(IVfs& vfs, sub_file_path path, file_path* repoPath, file_path* pathInRepo) const{
		if (repoPath) *repoPath = file_path();
		if (pathInRepo) *pathInRepo = file_path();

		if (vfs.state(K_blob_idx).state == fs::st_regular){ // found!
			if (pathInRepo) *pathInRepo = path;
			return true;
		}
		file_path current;

		for (auto i : path){
			auto dirst = vfs.state(current / i).state;
			if(dirst != fs::st_dir && dirst != fs::st_mount_point){ // invalid path
				if (repoPath) *repoPath = current;
				if (pathInRepo) *pathInRepo = rest_to_end_(i, path);
				return false;
			}
			current /= i;

			auto st = vfs.state(current / K_blob_idx);
			if (st.state == fs::st_regular){ // found!
				if (repoPath) *repoPath = current;
				if (pathInRepo) *pathInRepo = rest_to_end_(i, path);
				return true;
			}
		}
		return false;
	}

	struct LocalRepoFileIterator{
		public:
			typedef tree_blob::items_type::const_iterator item_iterator;
			LocalRepoFileIterator() : m_itr() {
				m_node.owner_fs = 0;
			}

			explicit LocalRepoFileIterator(item_iterator itr, IVfs* vfs)
				: m_itr(itr)
			{
				m_node.owner_fs = vfs;
			}

			const VfsNode& operator*() const
			{
				m_node.path = file_path(m_itr->first, pp_none);
				return m_node;
			}

			const VfsNode* operator->() const
			{
				return &**this;
			}


			LocalRepoFileIterator& operator++()  { ++m_itr; return *this;}
			LocalRepoFileIterator operator++(int) {
				LocalRepoFileIterator ret = *this;
				++*this;
				return ret;
			}

			LocalRepoFileIterator& operator--(){ return *this;}
			LocalRepoFileIterator operator--(int){ return *this;}

			bool operator==(const LocalRepoFileIterator& rhs) const
			{
				return m_itr == rhs.m_itr;
			}
		private:
			item_iterator m_itr;
			mutable VfsNode m_node;
	};

	class LocalRepositoryImp {
		public:
			LocalRepositoryImp(IVfs& vfs, const file_path& prefix, IVfs* host)
				: m_underlying(vfs), m_prefix(prefix), m_host(host), m_root()
			{
				auto ar_head = m_underlying.create<io::reader>(prefix/K_head, io::of_open);
				if (!ar_head) AIO_THROW(bad_repository_exception)("failed to open #head file");
				auto s_head = io::exchange::as_source(ar_head.get<io::reader>());
			s_head & m_head;

			auto ar_blob_idx = m_underlying.create<io::reader>(prefix/K_blob_idx, io::of_open);
			if (!ar_blob_idx) AIO_THROW(bad_repository_exception)("failed to open #blob.idx file");
			auto s_blob_idx = io::exchange::as_source(ar_blob_idx.get<io::reader>());

			auto ar_path_map = m_underlying.create<io::reader>(prefix/K_path_idx, io::of_open);
			if (!ar_path_map) AIO_THROW(bad_repository_exception)("failed to open #path.idx file");
			auto s_path_map = io::exchange::as_source(ar_path_map.get<io::reader>());

			s_blob_idx & m_blob_idx;
			s_path_map & m_path_map;
		}
		void sync(){ return m_underlying.sync();}
		const string& resource() const{ return m_prefix.str();}
		RootFs* root() const{ return m_root;}
		bool mounted() const { return m_root != 0;}
		file_path mountPoint() const { return m_root ? m_root->mountPoint(*m_host) : file_path();}
		VfsNodeRange children(sub_file_path path) const{
			typedef VfsNodeRange::iterator iterator;

			version_type file_version;
			auto filename = path.filename();
			if (!filename.empty() && filename.str()[0] == '#'){	// has versioning part
				file_version = version_type(filename.str());
			}
			else { // no version part
				file_version = getFileVersion(m_head, path);
			}

			if (is_empty(file_version)) return VfsNodeRange();
			auto tree_blob = get_tree_(file_version);
			if (!tree_blob)	return VfsNodeRange();
			return VfsNodeRange(iterator(LocalRepoFileIterator(tree_blob->items.begin(), m_host))
					, iterator(LocalRepoFileIterator(tree_blob->items.end(), m_host)));
		}
		VfsState state(sub_file_path path) const{
			version_type file_version;
			auto filename = path.filename();
			if (!filename.empty() && filename.str()[0] == '#'){	// has versioning part
				file_version = version_type(filename.str());
			}
			else { // no version part
				file_version = getFileVersion(m_head, path);
			}
			VfsState fst =
			{
				{ path, m_host}, fs::st_not_found, 0
			};

			if (is_empty(file_version)) return fst;
			auto pos = m_blob_idx.items.find(file_version);
			if (pos == m_blob_idx.items.end())	return fst;
			if (pos->second.info.flag == bt_file){
				fst.state = fs::st_regular;
				fst.size = pos->second.info.size;
			} else if (pos->second.info.flag == bt_tree){
				fst.state = fs::st_dir;
			}
			return fst;
		}
		any getopt(int id, const any & optdata) const { return any();}
		any setopt(int id, const any & optdata, const any & indata){ return any();}
		void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag); //TODO: imp
		void setRoot(RootFs* r){
			AIO_PRE_CONDITION(!mounted() || r == 0);
			m_root = r;
		}

		// from IRepository
		const FileHistory& history(const file_path& p) const; //TODO: imp
		const Submission* getSubmission(const version_type& ver) const{
			return get_submission_(is_empty(ver)? m_head : ver);
		}
		version_type getFileVersion(const version_type& commit_id, const file_path& p) const{
			auto commit = get_submission_(commit_id);
			if (!commit) return version_type();

			return getFileVersionFromTree_(commit->tree, p);
		}

		IVfs& underlying() const{ return m_underlying;}
		const file_path& prefix() const{ return m_prefix;}
		const Submission& commit(IWorkspace& wk, const string& description, const Submission& base); //TODO: imp
	private:
		version_type getFileVersionFromTree_(const version_type& tree_id, const file_path& p) const{
			auto tree_blob = get_tree_(tree_id);
			if (!tree_blob) return version_type();

			for (auto i(p.begin()), last(p.end());  i != last;){
				auto idir = tree_blob->items.find(i->str());
				if (idir == tree_blob->items.end())
					return version_type();

				auto & version = idir->second;
				if (++i == last) return version;

				auto ipos = m_blob_idx.items.find(version);
				if (ipos == m_blob_idx.items.end()) return version_type();

				if (ipos->second.info.flag == bt_tree){
					tree_blob = get_tree_(version);
					if (!tree_blob) return version_type();
				}
			}
			AIO_POST_CONDITION(false && "Logic wrong, should not reach here.");
			return version_type();
		}
		const tree_blob* get_tree_(const version_type& ver) const;
		const Submission* get_submission_(const version_type& ver) const;

		IVfs& m_underlying;
		file_path m_prefix;
		IVfs* m_host;
		RootFs* m_root;
		blob_idx m_blob_idx;
		path_map_type m_path_map;
		version_type m_head;

	};
	LocalRepository::LocalRepository(IVfs& vfs, const file_path& prefix)
		: m_imp(new LocalRepositoryImp(vfs, prefix, this))
	{}
	fs_error LocalRepository::remove(sub_file_path /* path */){
		return fs::er_permission_denied;
	}
	fs_error LocalRepository::createDir(sub_file_path /* path */){
		return fs::er_permission_denied;
	}
	fs_error LocalRepository::copy(sub_file_path /* from */, sub_file_path /* to */ ){
		return fs::er_permission_denied;
	}
	fs_error LocalRepository::truncate(sub_file_path /* path */, long_size_t /* s */){
		return fs::er_permission_denied;
	}
	void LocalRepository::sync(){
		m_imp->sync();
	}
	const string& LocalRepository::resource() const{
		return m_imp->resource();
	}
	RootFs* LocalRepository::root() const{ return m_imp->root(); }
	bool LocalRepository::mounted() const{ return m_imp->mounted();}
	file_path LocalRepository::mountPoint() const{ return m_imp->mountPoint();}
	VfsNodeRange LocalRepository::children(sub_file_path path) const{ return m_imp->children(path);}
	VfsState LocalRepository::state(sub_file_path path) const{ return m_imp->state(path);}
	any LocalRepository::getopt(int id, const any & optdata) const { return m_imp->getopt(id, optdata);}
	any LocalRepository::setopt(int id, const any & optdata,  const any & indata) { return m_imp->setopt(id, optdata, indata);}
	void** LocalRepository::do_create(unsigned long long mask,
			void** ret, unique_ptr<void>& owner, sub_file_path path, int flag){
		return m_imp->do_create(mask, ret, owner, path, flag);
	}
	void LocalRepository::setRoot(RootFs* r){ m_imp->setRoot(r); }

	const FileHistory& LocalRepository::history(const file_path& p) const{ return m_imp->history(p);}
	const Submission* LocalRepository::getSubmission(const version_type& ver) const{ return m_imp->getSubmission(ver);}
	version_type LocalRepository::getFileVersion(const version_type& commit_id, const file_path& p){
		return m_imp->getFileVersion(commit_id, p);
	}
	void LocalRepository::push(){ }	// do nothing for local repo
	void LocalRepository::pull(){ } // do nothing for local repo
	void LocalRepository::fetch(const version_type& /* ver */, int /* level */){}
	IVfs& LocalRepository::underlying() const{ return m_imp->underlying();}
	const file_path& LocalRepository::prefix() const{ return m_imp->prefix();}
	const Submission& LocalRepository::commit(IWorkspace& wk, const string& description, const Submission& base){
		return m_imp->commit(wk, description, base);
	}
}}


