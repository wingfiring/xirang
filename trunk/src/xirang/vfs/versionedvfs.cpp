#include <xirang/versionedvfs.h>
#include <xirang/vfs/vfs_common.h>
#include <xirang/io/exchs11n.h>
#include <xirang/io/versiontype.h>
#include <xirang/io/path.h>
#include <xirang/io/adaptor.h>
#include <xirang/versionhelper.h>
#include <xirang/io/s11nbasetype.h>


#include <tuple>
#include <unordered_map>
#include <set>
#include <map>
#include <ctime>

namespace xirang{ namespace vfs{

	struct blob_info{
		uint32_t flag;
		version_type version;
		uint64_t size;
		long_size_t offset;	// -1 means separated blob file.
	};

	template<typename Ar> Ar& load(Ar& ar, blob_info& tr){
		ar & tr.flag & tr.version & tr.size & tr.offset;
		return ar;
	}

	template<typename Ar> Ar& save(Ar& ar, const blob_info& tr){
		ar & tr.flag & tr.version & tr.size & tr.offset;
		return ar;
	}

	struct blob_info_map{
		typedef std::unordered_map<version_type, blob_info, hash_version_type> type;
		type items;
	};

	template<typename Ar> Ar& load(Ar& ar, blob_info_map& infos){
		blob_info item;
		version_type version;
		while (ar.readable()){
			ar & version & item;
			infos.items.insert(std::make_pair(version, item));
		}
		return ar;
	}

	template<typename Ar> Ar& save(Ar& ar, const blob_info_map& infos){
		for (auto & i: infos.items){
			ar & i.first & i.second;
		}
		return ar;
	}

	template<typename Ar> Ar& load(Ar& ar, Submission& sub){
		ar & sub.flag & sub.prev & sub.tree & sub.time
			& sub.author & sub.submitter & sub.description;
		return ar;
	}

	template<typename Ar> Ar& save(Ar& ar, const Submission& sub){
		ar & sub.flag & sub.prev & sub.tree & sub.time
			& sub.author & sub.submitter & sub.description;
		return ar;
	}

	struct tree_blob {
		uint32_t flag;
		typedef std::unordered_map<string, version_type, hash_string> items_type;
		items_type items;
	};
	template<typename Ar> Ar& load(Ar& ar, tree_blob& tb){
		ar & tb.flag;
		size_t s = load_size_t(ar) ;
		tb.items.reserve(s);
		for (size_t i = 0; i < s; ++i){
			auto version = load<version_type>(ar);
			auto name = load<string>(ar);
			tb.items.insert(std::make_pair(name, version));
		}
		return ar;
	}

	template<typename Ar> Ar& save(Ar& ar, const tree_blob& tb){
		ar & tb.flag & tb.items.size();
		for (auto &i : tb.items)
		  ar & i.second & i.first;
		return ar;
	}

	template<typename Ar> Ar& save(Ar& ar, const FileHistoryItem& fhi){
		return ar & fhi.submission & fhi.version;
	}
	template<typename Ar> Ar& load(Ar& ar, FileHistoryItem& fhi){
		return ar & fhi.submission & fhi.version;
	}

	typedef std::vector<FileHistoryItem> path_versions_type;
	struct path_map_type{
		std::map<file_path, path_versions_type, path_less>  items;
	};

	template<typename Ar> Ar& load(Ar& ar, path_map_type& tb){
		while (ar.readable()){
			file_path p;
			ar & p;
			auto & dest = tb.items[p];
			auto s = load_size_t(ar);
			for (size_t i = 0; i < s; ++i){
				dest.push_back(load<FileHistoryItem>(ar));
			}
		}
		return ar;
	}

	template<typename Ar> Ar& save(Ar& ar, const path_map_type& tb){
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
				: m_repoManager(repoManager)
				  , m_underlying(fs)
				  , m_prefix(prefix)
				  , m_host(host)
				  , m_root()
		{ }

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
	{ }
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

				s_blob_idx & m_blob_infos;
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
				auto tree = get_blob_<tree_blob>(file_version);
				if (tree.flag != bt_tree)	return VfsNodeRange();
				return VfsNodeRange(iterator(LocalRepoFileIterator(tree.items.begin(), m_host))
							, iterator(LocalRepoFileIterator(tree.items.end(), m_host)));
			}
			VfsState state(sub_file_path path) const{
				version_type file_version = getVersionOfPath_(path);
				VfsState fst =
				{
					{ path, m_host}, fs::st_not_found, 0
				};

				if (is_empty(file_version)) return fst;
				auto pos = m_blob_infos.items.find(file_version);
				if (pos == m_blob_infos.items.end())	return fst;
				if (pos->second.flag == bt_file){
					fst.state = fs::st_regular;
					fst.size = pos->second.size;
				} else if (pos->second.flag == bt_tree){
					fst.state = fs::st_dir;
				}
				return fst;
			}
			any getopt(int id, const any & optdata) const { return any();}
			any setopt(int id, const any & optdata, const any & indata){ return any();}
			void** do_create(unsigned long long mask,
						void** base, unique_ptr<void>& owner, sub_file_path path, int flag){
				void ** ret = 0;
				if (mask & detail::get_mask<io::reader, io::read_map>::value ){
					version_type file_version = getVersionOfPath_(path);
					if (is_empty(file_version)) return 0;

					auto pos = m_blob_infos.items.find(file_version);
					if (pos == m_blob_infos.items.end()
								|| pos->second.flag != bt_file) return 0;

					if (pos->second.offset != long_size_t(-1)){
						auto adaptor = io::decorate<io::sub_archive
							, io::sub_reader_p
							, io::sub_read_map_p
							>(m_data_file, pos->second.offset, pos->second.offset + pos->second.size);
						iauto<io::reader, io::read_map> res (std::move(adaptor));
						ret = copy_interface<io::reader, io::read_map>::apply(mask, base, res, res.target_ptr.get());
						unique_ptr<void>(std::move(res.target_ptr)).swap(owner);
						return ret;
					}else { // for big file, put under folder '#data'
						static const file_path prefix("#data");
						return m_underlying.do_create(mask, base, owner, prefix / file_path(file_version.id.to_string(), pp_none), flag);
					}
				}
				return ret;
			}
			void setRoot(RootFs* r){
				AIO_PRE_CONDITION(!mounted() || r == 0);
				m_root = r;
			}

			// from IRepository
			FileHistory history(const file_path& p) const{
				auto pos = m_path_map.items.find(p);
				if (pos == m_path_map.items.end()) return FileHistory();

				typedef FileHistory::iterator iterator;
				return FileHistory(iterator(pos->second.begin()), iterator(pos->second.end()));

			}
			Submission getSubmission(const version_type& ver) const{
				return get_blob_<Submission>(is_empty(ver)? m_head : ver);
			}
			version_type getFileVersion(const version_type& commit_id, const file_path& p) const{
				auto cm = get_blob_<Submission>(commit_id);
				if (cm.flag != bt_submission) return version_type();

				return getFileVersionFromTree_(cm.tree, p);
			}

			IVfs& underlying() const{ return m_underlying;}
			const file_path& prefix() const{ return m_prefix;}

			struct Context{
				iauto<io::random, io::writer> idx_file;
				path_map_type path_versions;
			};
			Submission commit(IWorkspace& wk, const string& description, const version_type& base){
				auto s_pos = m_blob_infos.items.find(base);
				if (s_pos == m_blob_infos.items.end()) return Submission();

				Submission sub_base = load_submission_(s_pos->second.offset, s_pos->second.size);

				auto t_pos = m_blob_infos.items.find(sub_base.tree);
				if (t_pos == m_blob_infos.items.end())
				  AIO_THROW(repository_coruppted_exception)("submission tree blob not found");

				Context ctx;

				ctx.idx_file = m_underlying.create<io::writer, io::random>(m_prefix/K_blob_idx, io::of_create_or_open);
				if (!ctx.idx_file) AIO_THROW(bad_repository_exception)("failed to open #blob.idx file");
				auto & seek = ctx.idx_file.get<io::random>();
				seek.seek(seek.size());


				Submission sub;
				sub.flag = bt_submission;
				sub.description = description;
				sub.prev = base;
				sub.time = std::time(0);

				sub.tree = do_commit_(wk, sub_base.tree, file_path(), ctx);

				append_submission_blob_(sub, ctx);

				auto tree_map_file = m_underlying.create<io::writer, io::random>(m_prefix/K_path_idx, io::of_create_or_open);
				if (!tree_map_file) AIO_THROW(bad_repository_exception)("failed to open #path.idx file");
				auto & seek2 = tree_map_file.get<io::random>();
				seek2.seek(seek2.size());

				auto sink = io::exchange::as_sink(tree_map_file.get<io::writer>());
				for (auto& i : ctx.path_versions.items){
					auto & v = m_path_map.items[i.first];
					sink & i.first.str() & (v.size() + i.second.size());
					for (auto& f : i.second){
						FileHistoryItem fhi = {sub.version, f.version};
						sink & fhi;
						v.push_back(fhi);
					}
				}

				return sub;
			}

		private:
			version_type do_commit_(IWorkspace& wk, const version_type& vid, const file_path& path_in_repo, Context& ctx)
			{
				auto pos = m_blob_infos.items.find(vid);
				bool parent_exist = pos != m_blob_infos.items.end();

				if (!is_changed_(wk, path_in_repo))
				  return vid;

				if (wk.state(path_in_repo).state == fs::st_regular)
				  return add_file_to_repo_(wk, path_in_repo, ctx);

				// else add a folder, so need to merge the old one and new one
				tree_blob new_tree;
				if (parent_exist && pos->second.flag == bt_tree){
					auto old_tree = get_blob_<tree_blob>(pos->second.version);
					if (old_tree.flag != bt_tree)
					  AIO_THROW(repository_coruppted_exception)("failed to get tree blob");
					new_tree = old_tree;

					for (auto& r : wk.affectedRemove(path_in_repo)){
						auto full_path = path_in_repo/r.path;
						if (wk.isMarkedRemove(full_path)){
							new_tree.items.erase(r.path.str());
						}
						else {	//only mean it's children is marked as removed. need to regenerate version
							auto pos = new_tree.items.find(r.path.str()); // XXX: should the path be exist?
							if (pos != new_tree.items.end()){
								pos->second = do_commit_(wk, pos->second, path_in_repo/r.path, ctx);
							}
						}
					}
				}

				for (auto& i : wk.children(path_in_repo)){
					file_path wkpath = path_in_repo / i.path;
					if (wk.state(wkpath).state == fs::st_regular){
						new_tree.items.insert(std::make_pair(i.path.str(), add_file_to_repo_(wk, wkpath, ctx)));
					}
					else { //recursive for dir
						auto p = new_tree.items.find(i.path.str());
						auto version = do_commit_(wk, p == new_tree.items.end() ? version_type() : p->second, wkpath, ctx);
						new_tree.items.insert(std::make_pair(i.path.str(), version));
					}
				}

				return save_tree_blob_(path_in_repo, new_tree, ctx);
			}

			version_type save_tree_blob_(const file_path& path, const tree_blob& tree, Context& ctx){
				auto & seek = m_data_file.get<io::random>();
				auto offset = seek.size();
				seek.seek(offset);

				auto sink = io::exchange::as_sink(m_data_file.get<io::writer>());
				sink & tree;

				version_type ret = version_of_object(tree);
				save_blob_idx_(bt_tree, ret, seek.size() - offset, offset, ctx);

				FileHistoryItem fhi={version_type(),ret};
				ctx.path_versions.items[path].push_back(fhi);

				return ret;
			}

			version_type add_file_to_repo_(IWorkspace& wk, const file_path& path, Context& ctx) {
				auto src = wk.create<io::read_map>(path, io::of_open);
				if (!src) AIO_THROW(fs::open_failed_exception)("failed to open file in workdir");

				auto& seek = m_data_file.get<io::random>();
				long_size_t offset = seek.size();
				seek.seek(offset);
				copy_data(src.get<io::read_map>(), m_data_file.get<io::writer>());

				version_type ret = version_of_archive(src.get<io::read_map>());
				save_blob_idx_(bt_file, ret, seek.size() - offset, offset, ctx);

				FileHistoryItem fhi={version_type(),ret};
				ctx.path_versions.items[path].push_back(fhi);

				return ret;
			}
			// return true if any parent of the path or path itself is added or removed;
			bool is_changed_(IWorkspace& wk, const file_path& path_in_repo){
				file_path current;
				for (auto& i : path_in_repo){
					current /= i;
					if (!wk.isAffected(current) && wk.state(current).state == fs::st_not_found)
					  return false;
				}
				return true;
			}
			Submission load_submission_(long_size_t offset, uint64_t size) const{
				auto& seek = m_data_file.get<io::random>();
				seek.seek(offset);
				auto source = io::exchange::as_source(m_data_file.get<io::reader>());
				return load<Submission>(source);
			}
			void append_submission_blob_(const Submission& sub, Context& ctx){
				auto& seek = m_data_file.get<io::random>();
				auto offset = seek.size();
				seek.seek(offset);
				auto sink = io::exchange::as_sink(m_data_file.get<io::writer>());
				sink & sub;

				save_blob_idx_(bt_submission, sub.version, seek.size() - offset, offset, ctx);

				auto ar_head = m_underlying.create<io::writer, io::random>(m_prefix/K_head, io::of_create_or_open);
				if (!ar_head) AIO_THROW(bad_repository_exception)("failed to create #head file");

				auto sink2 = io::exchange::as_sink(ar_head.get<io::writer>());
				sink2 & sub.version;
				m_head = sub.version;
			}

			template<typename T> T get_blob_(const version_type& ver) const{
					T blob;
					blob.flag = bt_none;
					auto pos = m_blob_infos.items.find(ver);
					if (pos == m_blob_infos.items.end())
					  return blob;

					auto& seek = m_data_file.get<io::random>();
					seek.seek(pos->second.offset);
					auto source = io::exchange::as_source(m_data_file.get<io::reader>());
					return load<T>(source);
				}
			void save_blob_idx_(uint32_t flag, const version_type& ver, uint64_t size, long_size_t offset, Context& ctx){
				blob_info binfo = {flag, ver, size, offset};
				auto sink = io::exchange::as_sink(ctx.idx_file.get<io::writer>());
				sink & binfo;
				m_blob_infos.items.insert(std::make_pair(binfo.version, binfo));
			}

			version_type getVersionOfPath_(const file_path& p) const{
				auto filename = p.filename();
				if (!filename.empty() && filename.str()[0] == '#'){	// has versioning part
					return version_type(filename.str());
				}
				return getFileVersion(m_head, p);
			}
			version_type getFileVersionFromTree_(const version_type& tree_id, const file_path& p) const{
				auto tree= get_blob_<tree_blob>(tree_id);
				if (tree.flag != bt_tree) return version_type();

				for (auto i(p.begin()), last(p.end());  i != last;){
					auto idir = tree.items.find(i->str());
					if (idir == tree.items.end())
					  return version_type();

					auto & version = idir->second;
					if (++i == last) return version;

					auto ipos = m_blob_infos.items.find(version);
					if (ipos == m_blob_infos.items.end()) return version_type();

					if (ipos->second.flag == bt_tree){
						tree = get_blob_<tree_blob>(version);
						if (tree.flag != bt_tree) return version_type();
					}
				}
				AIO_POST_CONDITION(false && "Logic wrong, should not reach here.");
				return version_type();
			}

			IVfs& m_underlying;
			file_path m_prefix;
			IVfs* m_host;
			RootFs* m_root;
			blob_info_map m_blob_infos;
			path_map_type m_path_map;
			version_type m_head;
			iauto<io::reader, io::writer, io::random, io::read_map, io::write_map> m_data_file;
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

	FileHistory LocalRepository::history(const file_path& p) const{ return m_imp->history(p);}
	Submission LocalRepository::getSubmission(const version_type& ver) const{ return m_imp->getSubmission(ver);}
	version_type LocalRepository::getFileVersion(const version_type& commit_id, const file_path& p){
		return m_imp->getFileVersion(commit_id, p);
	}
	void LocalRepository::push(){ }	// do nothing for local repo
	void LocalRepository::pull(){ } // do nothing for local repo
	void LocalRepository::fetch(const version_type& /* ver */, int /* level */){}
	IVfs& LocalRepository::underlying() const{ return m_imp->underlying();}
	const file_path& LocalRepository::prefix() const{ return m_imp->prefix();}
	Submission LocalRepository::commit(IWorkspace& wk, const string& description, const version_type& base){
		return m_imp->commit(wk, description, base);
	}
}}


