#include <xirang/vfs/zipfs.h>
#include <xirang/zip.h>
#include <xirang/vfs/vfs_common.h>
#include <xirang/io/memory.h>

#include <map>

namespace xirang{ namespace vfs{
	struct EntryInfo{
		zip::file_header header ;
		file_path cache_name;
		fs::file_state type = fs::st_dir;
		bool is_dirty = false;
		bool is_force_store = false;
	};
	EntryInfo make_entryinfo(const file_path& name){
		EntryInfo info;
		info.header.name = name;
		return info;
	}

	class ZipFsImp 
	{
		public:
			explicit ZipFsImp(const iref<io::read_map, io::write_map>& ar, IVfs* cache, const string& resource, ZipFs* host)
				: m_resource(resource)
				  , m_read_map(&ar.get<io::read_map>())
				  , m_cache(cache)
				  , m_host(host)
				  , m_war(ar)
			{
				init_load_();
			}
			explicit ZipFsImp(iauto<io::read_map, io::write_map> ar, IVfs* cache, const string& resource, ZipFs* host)
				: m_resource(resource)
				  , m_read_map(&ar.get<io::read_map>())
				  , m_cache(cache)
				  , m_host(host)
				  , m_war(ar)
				  , m_archive(std::move(ar))
			{
				init_load_();
			}
			explicit ZipFsImp(const iref<io::read_map>& ar, IVfs* cache, const string& resource, ZipFs* host)
				: m_resource(resource)
				  , m_read_map(&ar.get<io::read_map>())
				  , m_cache(cache)
				  , m_host(host)
			{
				init_load_();
			}
			explicit ZipFsImp(iauto<io::read_map> ar, IVfs* cache, const string& resource, ZipFs* host)
				: m_resource(resource)
				  , m_read_map(&ar.get<io::read_map>())
				  , m_cache(cache)
				  , m_host(host)
				  , m_archive(std::move(ar))
			{
				init_load_();
			}

			fs_error remove(sub_file_path path){
				AIO_PRE_CONDITION(!path.is_absolute());
				if (is_readonly_() ||path.empty())
					return fs::er_permission_denied;

				auto pos = m_items.find(path);
				if (pos == m_items.end())
					return fs::er_not_found;
				if (pos->second.type == fs::st_dir
						&& !children_(path).empty()){
					return fs::er_not_empty;
				}
				m_items.erase(pos);
				return fs::er_ok;
			}

			fs_error createDir(sub_file_path path){
				AIO_PRE_CONDITION(!path.is_absolute());
				if (is_readonly_() ||path.empty())
					return fs::er_permission_denied;

				auto parent = path.parent();
				if (!parent.empty()){
					auto pos = m_items.find(parent);
					if (pos == m_items.end())
						return fs::er_not_found;
				}

				m_items.insert(std::make_pair(file_path(path), make_entryinfo(path)));
				return fs::er_ok;
			}
			fs_error copy(sub_file_path from, sub_file_path to){
				AIO_PRE_CONDITION(!to.is_absolute());
				if (is_readonly_() ||to.empty())
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
				if (is_readonly_() ||path.empty())
					return fs::er_permission_denied;
				
				auto pos = m_items.find(path);
				if (pos == m_items.end())
					return fs::er_not_found;
				if (pos->second.type != fs::st_regular)
					return fs::er_not_regular;

				if (pos->second.cache_name.empty()){
					auto ret = extract_file_(pos->second);
					if (ret != fs::er_ok)	return ret;
				}

				pos->second.is_dirty = true;
				return m_cache->truncate(pos->second.cache_name, s);
			}

			fs_error extract_file_(EntryInfo& info){
				auto fin = zip::open_raw(info.header);
				auto fout = temp_file<io::write_map>(*m_cache, sub_file_path(), sub_file_path()
						, io::of_remove_on_close, &info.cache_name);
				auto s = io::copy_data(fin.get<io::read_map>(), fout.get<io::write_map>());
				if (s != fin.get<io::read_map>().size()){
					info.cache_name = file_path();
					return fs::er_system_error;
				}
				return fs::er_ok;
			}

			void sync(){
				if (is_readonly_()) return;

				m_war.get<io::write_map>().truncate(0);
				zip::reader_writer zip_writer(m_war);
				for (auto& i : m_items){
					auto &info = i.second;
					if (info.type == fs::st_dir){
						if (info.is_force_store){
							zip::file_header h;
							h.name = i.first;
							h.method = zip::cm_store;
							h.flags = 0x800;
							h.external_attrs = 0x10;	//dir
							io::empty empty;
							iref<io::reader> in(empty);
							zip_writer.append(in.get<io::reader>(), h);
						}
						continue;
					}
					if (info.is_dirty){
						auto src = m_cache->create<io::read_map>(info.cache_name, io::of_open);
						zip_writer.append(src.get<io::read_map>(), i.first);
					}
					else{
						auto src = zip::open_raw(info.header);
						zip_writer.append(src.get<io::read_map>(), info.header, zip::ft_defalted);
					}
				}
			}
			const string& resource() const { return m_resource;}
			RootFs* root() const{ return m_root;}
			bool mounted() const { return m_root != 0; }
			file_path mountPoint() const {
				return m_root ? m_root->mountPoint(*m_host) : file_path();
			}

			typedef std::map<file_path, EntryInfo, path_less>::const_iterator item_iterator;
			class FileIterator
			{
				public:
					FileIterator() : m_itr() {
						m_node.owner_fs = 0;
					}

					explicit FileIterator(item_iterator itr, IVfs* vfs)
						: m_itr(itr)
					{ 
						m_node.owner_fs = vfs;
					}

					const VfsNode& operator*() const
					{
						m_node.path = m_itr->first;
						return m_node;
					}

					const VfsNode* operator->() const
					{
						m_node.path = m_itr->first;
						return &m_node;
					}

					FileIterator& operator++()	{ ++m_itr; return *this;}
					FileIterator operator++(int) { 
						FileIterator ret = *this; 
						++*this; 
						return ret;
					}

					FileIterator& operator--(){ return *this;}
					FileIterator operator--(int){ return *this;}

					bool operator==(const FileIterator& rhs) const
					{
						return m_itr == rhs.m_itr;
					}
				private:
					item_iterator m_itr;
					mutable VfsNode m_node;
			};

			VfsNodeRange children(sub_file_path path) const{
				auto ret = children_(path);
				return VfsNodeRange(
                    VfsNodeRange::iterator(FileIterator(ret.begin(), m_host)),
					VfsNodeRange::iterator(FileIterator(ret.end(), m_host))
						);
				return VfsNodeRange();
			}
			VfsState state(sub_file_path path) const{
				AIO_PRE_CONDITION(!path.is_absolute());
				VfsState fst = {
					{ path, m_host}, fs::st_not_found, 0
				};
				auto pos = m_items.find(path);
				if (pos != m_items.end()){
					fst.state = pos->second.type;

					if (pos->second.is_dirty && pos->second.type == fs::st_regular)
						fst.size = m_cache->state(pos->second.cache_name).size;
					else 
						fst.size = pos->second.header.uncompressed_size;
				}
				return fst;
			}
			void** do_create(unsigned long long mask,
					void** base, unique_ptr<void>& owner, sub_file_path path, int flag){
				AIO_PRE_CONDITION(!path.is_absolute());
				if (is_readonly_() ||path.empty())
					AIO_THROW(fs::permission_denied_exception);

				auto parent = path.parent();
				if (!parent.empty() && m_items.count(path) == 0)
					AIO_THROW(fs::not_found_exception);

				auto pos = m_items.find(path);

				auto f = (flag & io::of_open_create_mask);
				if (pos == m_items.end()){
					if(f == io::of_open) AIO_THROW(fs::not_found_exception);
					if (pos->second.type != fs::st_regular)
						AIO_THROW(fs::not_regular_exception);

					EntryInfo info;
					info.header.name = path;
					info.type = fs::st_regular;
					auto fout = temp_file<io::write_map>(*m_cache, sub_file_path(), sub_file_path()
							, io::of_remove_on_close, &info.cache_name);
					if (!fout)
						AIO_THROW(fs::system_error_exception);

					m_items.insert(std::make_pair(file_path(path), info));
				}
				else{
					if(f == io::of_create) AIO_THROW(fs::exist_exception);
					if(pos->second.cache_name.empty() && extract_file_(pos->second) != fs::er_ok)
						AIO_THROW(fs::system_error_exception);
				}

				if (mask & detail::get_mask<io::writer, io::write_map>::value )
					pos->second.is_dirty = true;

				return m_cache->do_create(mask, base, owner, pos->second.cache_name, (flag | io::of_create_or_open));
			}

			void setRoot(RootFs* r){
				AIO_PRE_CONDITION(!mounted() || r == 0);
				m_root = r;
			}

			IVfs* host_() const { return m_host;}
			bool is_readonly_() const{
				return !m_rwriter.valid();
			}
			void init_load_(){

				zip::reader zip_reader(*m_read_map);
				auto items = zip_reader.items();
				for (auto&i : items){
					EntryInfo info;
					info.header = i;
					info.type = (i.external_attrs & 0x10) ? fs::st_dir : fs::st_regular;

					create_parent_(i.name);
					auto parent = i.name.parent();
					if (!parent.empty()){
						auto pos = m_items.find(parent);
						AIO_PRE_CONDITION(pos != m_items.end());
						if (pos->second.type != fs::st_dir)
							continue;
					}

					m_items.insert(std::make_pair(i.name, info));
				}
			}
			void create_parent_(const file_path& path){
				auto parent = path.parent();
				if(!parent.empty()){
					auto pos = m_items.find(path);
					if (pos == m_items.end()){
						create_parent_(parent);
						m_items.insert(std::make_pair(path, make_entryinfo(path)));
					}
				}
			}
			range<item_iterator> children_(sub_file_path path) const{
					string pfirst = path.str() << literal("/");
					string plast = path.str() << literal("//");
					auto pos1 = m_items.lower_bound(file_path(pfirst, pp_none));
					auto pos2 = m_items.lower_bound(file_path(plast, pp_none));
					return make_range(pos1, pos2);
			}
		private:
			string m_resource;
			std::map<file_path, EntryInfo, path_less> m_items;
			io::read_map* m_read_map;
			IVfs* m_cache;
			ZipFs* m_host;
			RootFs* m_root;
			zip::reader_writer m_rwriter;
			iref<io::read_map, io::write_map> m_war;
			iauto<io::read_map> m_archive;
	};
	ZipFs::ZipFs(const iref<io::read_map, io::write_map>& ar, IVfs* cache, const string& resource/*  = string() */)
		: m_imp(new ZipFsImp(ar, cache, resource, this))
	{
	}
	ZipFs::ZipFs(iauto<io::read_map, io::write_map> ar, IVfs* cache, const string& resource /* = string() */)
		: m_imp(new ZipFsImp(std::move(ar), cache, resource, this))
	{}
	ZipFs::ZipFs(const iref<io::read_map>& ar, IVfs* cache, const string& resource /* = string() */)
		: m_imp(new ZipFsImp(ar, cache, resource, this))
	{}
	ZipFs::ZipFs(iauto<io::read_map> ar, IVfs* cache, const string& resource /* = string() */)
		: m_imp(new ZipFsImp(std::move(ar), cache, resource, this))
	{}

	ZipFs::~ZipFs(){}

	fs_error ZipFs::remove(sub_file_path path){
		return m_imp->remove(path);
	}

	fs_error ZipFs::createDir(sub_file_path path){
		return m_imp->createDir(path);
	}

	fs_error ZipFs::copy(sub_file_path from, sub_file_path to){
		return m_imp->copy(from, to);
	}

	fs_error ZipFs::truncate(sub_file_path path, long_size_t s){
		return m_imp->truncate(path, s);
	}

	void ZipFs::sync(){ return m_imp->sync(); }
	const string& ZipFs::resource() const{ return m_imp->resource(); }
	RootFs* ZipFs::root() const{ return m_imp->root(); }
	bool ZipFs::mounted() const{ return m_imp->mounted(); }
	file_path ZipFs::mountPoint() const{ return m_imp->mountPoint(); }

	VfsNodeRange ZipFs::children(sub_file_path path) const{
		return m_imp->children(path);
	}

	VfsState ZipFs::state(sub_file_path path) const{
		return m_imp->state(path);
	}

	void** ZipFs::do_create(unsigned long long mask,
			void** base, unique_ptr<void>& owner, sub_file_path path, int flag){
		return m_imp->do_create(mask, base, owner, path, flag);
	}

	void ZipFs::setRoot(RootFs* r){ m_imp->setRoot(r);}
}}

