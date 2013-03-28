#include <aio/xirang/vfs/zip.h>
#include <aio/common/atomic.h>
#include <aio/common/archive/mem_archive.h>
#include <vector>
#include <memory>
#include <sstream>

#include "file_tree.h"
#include <aio/xirang/vfs/vfs_common.h>
#include "zip_file_header.h"
#include <aio/common/archive/adaptor.h>
#include <aio/common/deflate.h>

//#include <iostream>

namespace xirang{ namespace fs{ 
    using aio::long_size_t;
	using namespace aio::io;
	using namespace aio;
#if 0
	class ZipFsImp
	{
		public:

		// file operations
		void** do_create(unsigned long long mask,
			void** base, aio::unique_ptr<void>& owner, const string& path, int flag){
			AIO_PRE_CONDITION(!is_absolute(path));

			file_node* pos = locate(m_root_node, path);
			const bool for_write = mask & io::get_mask<aio::io::writer, aio::io::write_view>::value;

            if ((readonly_() && for_write)
                || (pos && flag == of_create)
					|| (!pos && flag == of_open)
                    || (pos && pos->type != aiofs::st_regular)
                    )
				return 0;

			if (!pos){
				pos = private_::create_node(m_root_node, path, aiofs::st_regular, false);
				if (pos && pos->type == aiofs::st_regular)
				{
					if (!pos->data)

					{
						pos->data = new file_header;
						pos->data->type = aiofs::st_regular;
						pos->data->cache_fs = m_cache;
						pos->data->name = path;

						pos->data->gp_flag = 0x806;
						pos->data->compression_method = 8;
					}
				}
				else 
					return 0;
			}
			AIO_PRE_CONDITION(pos);

			AIO_PRE_CONDITION(for_write || flag == of_open);
			pos->data->mod_time = -1;
			return pos->data->cache(m_rmap, m_wmap);
		}


		// \pre !is_absolute(path)
		VfsNodeRange children(const string& path) const
		{
			file_node* pos = locate(const_cast<file_node&>(m_root_node), path);
			typedef private_::FileNodeIterator<file_header*> FileIterator;
			if (pos && pos->type == aiofs::st_dir)
			{
				std::map<string, private_::file_node<file_header*>*>::iterator beg = pos->children.begin();
				std::map<string, private_::file_node<file_header*>*>::iterator end = pos->children.end();
				return VfsNodeRange(
                    VfsNodeRange::iterator(FileIterator(beg, m_host)),
					VfsNodeRange::iterator(FileIterator(end, m_host))
						);
			}
			return VfsNodeRange();
		}

		// \pre !is_absolute(path)
		VfsState state(const string& path) const
		{
			ZipFsImp* self = const_cast<ZipFsImp*>(this);
			file_node* pnode = locate(self->m_root_node, path);
			VfsState st = {
				{path, m_host},
				aiofs::st_not_found, 0
			};
			if (pnode)	//found
			{
				st.state = pnode->type;
				if (st.state == aiofs::st_regular)
				{
                    VfsState stc = m_cache->state(path);
                    if (stc.state == aiofs::st_regular)
                    {
                        st.size = stc.size;
                    }
                    else
						st.size = pnode->data->uncompressed_size;
				}
			}

			return st;
		}

		io::read_map* m_rmap;
		io::write_map* m_wmap;
		IVfs* m_cache;

		const string m_resource;
		IVfs* m_host;
		RootFs* m_root;
		file_node m_root_node;

        bool m_sync_on_destroy;
	};
#endif

	class ZipFsImp
	{
		public:
		typedef private_::file_node<file_header> file_node;

		explicit ZipFsImp(read_map* rd, write_map* wr, ioctrl* ioc, const string& res, IVfs* host, IVfs& cache, bool sync_on_destroy, CachePolicy cp)
			: m_rmap(rd), m_wmap(wr), m_ioctrl(ioc), m_cache(&cache), m_resource(res), m_host(host), m_root(0)
			, m_cache_policy(cp) , m_sync_on_destroy(sync_on_destroy)
		{
			AIO_PRE_CONDITION(rd);
			init_();
		}

		~ZipFsImp()
		{
            if (m_sync_on_destroy && !readonly_()){
                try {sync(); }
                catch(...){	}
            }
		}


		// common operations of dir and file
		// \pre !is_absolute(path)
		fs_error remove(const string& path){
            AIO_PRE_CONDITION(!is_absolute(path));
            if (readonly_())
                return aiofs::er_permission_denied;

            fs_error ret = remove_check(*host(), path);
			if (ret == aiofs::er_ok)
			{
				auto pos = locate(m_root_node, path);
				AIO_PRE_CONDITION(pos.node);
                return pos.not_found.empty()
					? removeNode(pos.node)
					: aiofs::er_not_found;
			}

			return ret;
		}

		// dir operations
		// \pre !is_absolute(path)
		fs_error createDir(const  string& path){
			AIO_PRE_CONDITION(!is_absolute(path));
            if (readonly_())
                return aiofs::er_permission_denied;

			auto pos = locate(m_root_node, path);
			if (pos.not_found.empty())
				return aiofs::er_exist;
			if (pos.node->type != aiofs::st_dir)
				return aiofs::er_not_dir;

			if (aio::contains(pos.not_found, '/'))
				return aiofs::er_not_found;

			create_node(pos, aiofs::st_dir);
			return aiofs::er_ok;
		}

		// file operations
		void** do_create(unsigned long long mask,
			void** base, aio::unique_ptr<void>& owner, const string& path, int flag);

		// \pre !is_absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		fs_error copy(const string& from, const string& to)
		{
            AIO_PRE_CONDITION(!is_absolute(to));
            if (readonly_())
                return aiofs::er_permission_denied;

			VfsNode from_node = { from, m_host};
			if (is_absolute(from))
			{
				AIO_PRE_CONDITION(mounted());
				from_node = m_root->locate(from).node;
			}

			VfsNode to_node = { to, m_host};
			return xirang::fs::copyFile(from_node, to_node);
		}

		fs_error truncate(const string& path, aio::long_size_t s)
		{
            AIO_PRE_CONDITION(!is_absolute(path));
            if (readonly_())
                return aiofs::er_permission_denied;

			auto pos = locate(m_root_node, path);

			if (!pos.not_found.empty())
				return aiofs::er_not_found;
			if (pos.node->type != aiofs::st_regular)
				return aiofs::er_system_error;

			file_header& head = pos.node->data;
			if (head.cached()){
				head.dirty = true;
				return m_cache->truncate(head.cached_path, s);
			}

			return cache_(head, s);
		}

		void sync() {
            if (!readonly_())
			    commit_();
		}

		// query
		const string& resource() const{ return m_resource; }

		// volume
		// if !mounted, return null
		RootFs* root() const { return m_root;}

		// \post mounted() && root() || !mounted() && !root()
		bool mounted() const { return m_root != 0; }

		// \return mounted() ? is_absolute() : empty() 
		string mountPoint() const
		{
			return m_root ? m_root->mountPoint(*m_host) : "";
		}

		// \pre !is_absolute(path)
		VfsNodeRange children(const string& path) const;
		
		// \pre !is_absolute(path)
		VfsState state(const string& path) const;

		// if r == null, means unmount
		void setRoot(RootFs* r) {
			AIO_PRE_CONDITION(!mounted() || r == 0);
			m_root = r;
		}

		IVfs* host() const { return m_host;}
        any getopt(int id, const any & optdata /*= any() */) const 
        {
            if (id == vo_readonly)
                return readonly_();
            else if (id == vo_sync_on_destroy)
                return m_sync_on_destroy;
            return any();
        }

        any setopt(int id, const any & optdata,  const any & indata/*= any()*/)
        {
            if (id == vo_sync_on_destroy)
            {
                m_sync_on_destroy = aio::any_cast<bool>(optdata);
                return true;
            }
            return any();
        }

		private:
		fs_error cache_(file_header& head, aio::long_size_t s){
			string filename;
			aiofs::dir_filename(head.name, &filename);
			
			auto dest = temp_file<ioctrl, write_map>(*m_cache, ("?_" + filename), empty_str, of_create, &head.cached_path);
			ext_heap::handle h(head.relative_offset_data(*m_rmap), head.relative_offset_data(*m_rmap) + head.compressed_size);
			auto inf = decorate<sub_archive , sub_read_map_p>(m_rmap, h);
			read_map& src = inf;
			auto result = aio::zip::inflate(src, dest.get<write_map>());
			dest.get<ioctrl>().truncate(s);
			
			return (result.err == aio::zip::ze_ok) ? aiofs::er_ok: aiofs::er_data_error;
		}
		// m_file must support reader and random seek
		void init_()
		{
			if (m_rmap->size() == 0)
				return;

			aio::iauto<read_view> iview;
			uint32_t number_entries = 0;
			std::tie(iview, number_entries) = load_cd(*m_rmap);
			read_view& view = iview.get<read_view>();
			auto view_range = view.address();
			if (view_range.empty())	//empty cd. it's possible if the input zip file is corrupted. 
				return;

			buffer_in mrd(view.address());

			for (uint16_t i = 0; i < number_entries; ++i)
			{
				auto header = load_header(mrd);
				if (header.name.empty()) continue;	//bad name

				AIO_PRE_CONDITION(header.persist);
				create_node(m_root_node, header.name, header.type, true);
			}
		}

		typedef std::vector<file_header*> entries_type;
		void dump_headers_(file_node& node, entries_type& entries)
		{
            if (node.data.type == aio::fs::st_regular)
				entries.push_back(&node.data);
			for (auto& v : node.children)
				dump_headers_(*v.second, entries);
		}
		void commit_()
		{
            AIO_PRE_CONDITION(!readonly_());

			auto dest = temp_file<read_map, write_map, aio::io::random>(*m_cache, "TMP", empty_str, of_create | of_remove_on_close);

			entries_type entries;
			dump_headers_(m_root_node, entries);
			std::sort(entries.begin(), entries.end());

			//1. dump entries 
			auto& dest_wmap = dest.get<write_map>();
			for (auto v : entries){
				copy_entry(*v, dest_wmap);
			}

			//2. dump central dir
			long_size_t offset_central_dir = dest.get<aio::io::random>().offset();
			for (auto v: entries){
				write_cd_entry(*v, dest_wmap);
			}
			long_size_t size_central_dir = dest.get<aio::io::random>().offset() - offset_central_dir;


			write_cd_end(dest_wmap, entries.size(), size_central_dir, offset_central_dir);

			//3. copy the tmp back.
			long_size_t copied_size = copy_data(dest.get<read_map>(), *m_wmap);
			if (m_ioctrl->truncate(copied_size) != aio::fs::er_ok)
				AIO_THROW(archive_io_fatal_error);
		}

        bool readonly_() const{ return m_wmap == 0; }

		file_node m_root_node;
		read_map* m_rmap;
		write_map* m_wmap;
		ioctrl* m_ioctrl;
		IVfs* m_cache;
		const string m_resource;
		IVfs* m_host;
		RootFs* m_root;
		CachePolicy m_cache_policy;
        bool m_sync_on_destroy;

	};

	ZipFs::ZipFs(aio::iref<read_map> file, IVfs& cache, const string& resource, CachePolicy cp /* = cp_flat */ )
		: m_imp (new ZipFsImp(&file.get<read_map>(), 0, 0, resource, this, cache, false, cp))
	{
	}
	ZipFs::ZipFs(aio::iref<read_map, write_map, ioctrl> file, IVfs& cache, const string& resource, bool sync_on_destroy, CachePolicy cp)
		: m_imp (new ZipFsImp(&file.get<read_map>(), &file.get<write_map>(), &file.get<ioctrl>(),resource, this, cache, sync_on_destroy, cp))
	{
	}
	ZipFs::~ZipFs()
	{
		delete m_imp;
	}

	// common operations of dir and file
	// \pre !is_absolute(path)
	fs_error ZipFs::remove(const string& path) {
		return m_imp->remove(path);
	}

	// dir operations
	// \pre !is_absolute(path)
	fs_error ZipFs::createDir(const  string& path){
		return m_imp->createDir(path);
	}
	void** ZipFs::do_create(unsigned long long mask,
			void** base, aio::unique_ptr<void>& owner, const string& path, int flag){
		return m_imp->do_create(mask, base, owner, path, flag);
	}

	// \pre !is_absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	// otherwise, from should be a
	fs_error ZipFs::copy(const string& from, const string& to)
	{
		return m_imp->copy(from, to);
	}

	fs_error ZipFs::truncate(const string& path, aio::long_size_t s){
		return m_imp->truncate(path, s);
	}

	void ZipFs::sync(){
		m_imp->sync();
	}

	// query
	const string& ZipFs::resource() const {
		return m_imp->resource();
	}

	// volume
	// if !mounted, return null
	RootFs* ZipFs::root() const{
		return m_imp->root();
	}

	// \post mounted() && root() || !mounted() && !root()
	bool ZipFs::mounted() const{
		return m_imp->mounted();
	}

	// \return mounted() ? is_absolute() : empty() 
	string ZipFs::mountPoint() const{
		return m_imp->mountPoint();
	}

	// \pre !is_absolute(path)
	VfsNodeRange ZipFs::children(const string& path) const
	{
		return m_imp->children(path);
	}

	// \pre !is_absolute(path)
	VfsState ZipFs::state(const string& path) const{
		return m_imp->state(path);
	}
	// if r == null, means unmount
	void ZipFs::setRoot(RootFs* r) {
		m_imp->setRoot(r);
	}
    any ZipFs::getopt(int id, const any & optdata /*= any() */) const 
    {
        return m_imp->getopt(id, optdata);
    }

    any ZipFs::setopt(int id, const any & optdata,  const any & indata/*= any()*/)
    {
        return m_imp->setopt(id, optdata, indata);
    }
}}


