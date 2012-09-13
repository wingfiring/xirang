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

//#include <iostream>

namespace xirang{ namespace fs{ 
    using aio::long_size_t;

	namespace private_ {
		template<> struct node_releaser<file_header*>
		{
			static void release(file_header* p)
			{
				delete p;
			}
		};

	}

	using namespace aio::archive;

	typedef multiplex_owner<multiplex_deletor<multiplex_reader<multiplex_random<multiplex_base<reader, aio::archive::random, aio::ideletor> > > > > zip_read_archive;
	typedef multiplex_owner<multiplex_deletor<multiplex_writer<multiplex_random<multiplex_base<writer, aio::archive::random, aio::ideletor> > > > > zip_write_archive;
	typedef multiplex_owner<multiplex_deletor<multiplex_reader<multiplex_writer<multiplex_random<multiplex_base<reader, writer, aio::archive::random, aio::ideletor> > > > > > zip_read_write_archive;


	class ZipFsImp
	{
		typedef private_::file_node<file_header*> file_node;
		public:

		explicit ZipFsImp(iarchive& file, const string& res, IVfs* host, IVfs& cache, bool sync_on_destroy)
			: m_file(file), m_cache(&cache), m_resource(res), m_host(host), m_root(0)
            , m_readonly(file.query_writer() == 0)
            , m_sync_on_destroy(sync_on_destroy)
		{
			sync_set(m_next_index, 0);
			init_();
		}

		~ZipFsImp()
		{
            if (m_sync_on_destroy && !m_readonly){
                try {sync(); }
                catch(...){	}
            }
		}


		// common operations of dir and file
		// \pre !is_absolute(path)
		fs_error remove(const string& path)
		{
            AIO_PRE_CONDITION(!is_absolute(path));
            if (m_readonly)
                return aiofs::er_permission_denied;

			fs_error ret = remove_check(*host(), path);
			if (ret == aiofs::er_ok)
			{
				file_node* pos = locate(m_root_node, path);
				AIO_PRE_CONDITION(pos);
                if (pos == &m_root_node)
                    return aiofs::er_invalid;
				return removeNode(pos);
			}

			return ret;
		}

		// dir operations
		// \pre !is_absolute(path)
		fs_error createDir(const  string& path)
		{
			AIO_PRE_CONDITION(!is_absolute(path));

            if (m_readonly)
                return aiofs::er_permission_denied;

			if (state(path).state != aiofs::st_not_found)
				return aiofs::er_exist;

			file_node* pos = private_::create_node(m_root_node, path, aiofs::st_dir, false);
			if (pos)
			{
				pos->data = new file_header;
				pos->data->type = aiofs::st_dir;
				pos->data->zip_archive = &m_file;
				pos->data->cache_fs = m_cache;
				pos->data->index = next_index_();
				pos->data->name = path;

				pos->data->gp_flag = 0x800;
				pos->data->compression_method = 0;
                pos->data->mod_time = -1;
                pos->data->mod_date = -1;
			}
			return pos ?  aiofs::er_ok : aiofs::er_not_found;
		}

		// file operations
		archive_ptr create(const string& path, int mode, int flag)
		{
			AIO_PRE_CONDITION(!is_absolute(path));

			file_node* pos = locate(m_root_node, path);

            if ( (m_readonly &&  (mode & aio::archive::mt_write))
                || (pos && flag == of_create)
					|| (!pos && flag == of_open)
                    || (pos && pos->type != aiofs::st_regular)
                    )
				return archive_ptr().move();

			if (!pos)
			{
				pos = private_::create_node(m_root_node, path, aiofs::st_regular, false);
				if (pos && pos->type == aiofs::st_regular)
				{
					if (!pos->data)
					{
						pos->data = new file_header;
						pos->data->type = aiofs::st_regular;
						pos->data->zip_archive = &m_file;
						pos->data->cache_fs = m_cache;
						pos->data->index = next_index_();
						pos->data->name = path;

						pos->data->gp_flag = 0x806;
						pos->data->compression_method = 8;
					}
				}
				else 
					return archive_ptr().move();
			}
			AIO_PRE_CONDITION(pos);

			if (mode & aio::archive::mt_read)
			{
				if ((mode & aio::archive::mt_write) == 0)
				{
					AIO_PRE_CONDITION(flag == of_open);
					return archive_ptr(new zip_read_archive(pos->data->cache(no_edit).move())).move();
				}
				else
                {
                    pos->data->mod_time = -1;
					return archive_ptr(new zip_read_write_archive(pos->data->cache(will_edit).move())).move();
                }
			}
			else if (mode & aio::archive::mt_write)	//write only
			{
                pos->data->mod_time = -1;
				return archive_ptr(new zip_write_archive(pos->data->cache(will_edit).move())).move();
			}
			return archive_ptr().move();

		}

		// \pre !is_absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		fs_error copy(const string& from, const string& to)
		{
            AIO_PRE_CONDITION(!is_absolute(to));
            if (m_readonly)
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
            if (m_readonly)
                return aiofs::er_permission_denied;

			file_node* pos = locate(m_root_node, path);
			if (!pos)
				return aiofs::er_not_found;

			if (pos->type != aiofs::st_regular)
				return aiofs::er_not_regular;

			AIO_PRE_CONDITION(pos->data);

			//TODO:optimization for s == 0
			zip_write_archive(pos->data->cache(will_edit).move()).truncate(s);
			return aiofs::er_ok;
		}

		void sync() {
            if (!m_readonly)
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
		//
		// if r == null, means unmount
		void setRoot(RootFs* r) {
			AIO_PRE_CONDITION(!mounted() || r == 0);
			m_root = r;
		}

		IVfs* host() const { return m_host;}
        any getopt(int id, const any & optdata /*= any() */) const 
        {
            if (id == vo_readonly)
                return m_readonly;
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
		typedef std::pair<int, file_header*> file_entry_type;
		typedef std::vector<file_entry_type> entries_type;

		int next_index_()
		{
			return sync_fetch_add(m_next_index, 1);
		}

		// m_file must support reader and random seek
		void init_()
		{
            if (m_file.query_random()->size() == 0)
                return;

			aio::buffer<byte> buf;
			int number_entries = load_cd(m_file, buf);

			aio::archive::buffer_in mrd(buf);

			for (uint16_t i = 0; i < number_entries; ++i)
			{
				aio::unique_ptr<file_header> ph(new file_header);
				load_header(mrd, *ph);

				//init rest members of header
				ph->index = next_index_();
				ph->zip_archive = &m_file;
				ph->cache_fs = m_cache;

				file_node* pos = create_node(m_root_node, ph->name, ph->type, true);
				AIO_PRE_CONDITION(pos);

				if (pos->data)
					AIO_THROW(duplicated_file_name)(ph->name.c_str());

				pos->data = ph.release();
			}
		};



		void dump_headers_(file_node& node, entries_type& entries)
		{
            if (node.data && node.data->type == aio::fs::st_regular)
				entries.push_back(file_entry_type(node.data->index, node.data));
			for (std::map<string, private_::file_node<file_header*>*>::iterator itr = node.children.begin(); itr != node.children.end(); ++itr)
			{
				dump_headers_(*itr->second, entries);
			}
		}

		void commit_()
		{
            AIO_PRE_CONDITION(!m_readonly);

			archive_ptr tmpzip = temp_file(*m_cache, "TMP", "").move();
			writer* wr = tmpzip->query_writer();
            AIO_PRE_CONDITION(wr);

			aio::archive::random* rng = tmpzip->query_random();
			AIO_PRE_CONDITION(rng);

			entries_type entries;
			dump_headers_(m_root_node, entries);
			std::sort(entries.begin(), entries.end());

			//1. dump entries 
			for (entries_type::iterator itr = entries.begin(); itr != entries.end(); ++itr)
			{
				copy_entry(*itr->second, *wr, *rng);
			}


			//2. dump central dir
			long_size_t offset_central_dir = rng->offset();
			for ( entries_type::iterator itr = entries.begin(); itr != entries.end(); ++itr)
			{
				write_cd_entry(*itr->second, *wr);
			}
			long_size_t size_central_dir = rng->offset() - offset_central_dir;


			write_cd_end(*wr, entries.size(), size_central_dir, offset_central_dir);

			//3. copy the tmp back.
			long_size_t copied_size = copy_archive(*tmpzip, m_file);
			if (copied_size != rng->size())
				AIO_THROW(archive_io_fatal_error);
		}

		iarchive& m_file;
		IVfs* m_cache;

		const string m_resource;
		IVfs* m_host;
		RootFs* m_root;
		file_node m_root_node;
		aio::atomic::atomic_t<int> m_next_index;

        const bool m_readonly;
        bool m_sync_on_destroy;
	};

	ZipFs::ZipFs(iarchive& dir, IVfs& cache, const string& res, bool sync_on_destroy)
		: m_imp (new ZipFsImp(dir, res, this, cache, sync_on_destroy))
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

	// file operations
	archive_ptr ZipFs::create(const string& path, int mode, int flag)
	{
		return m_imp->create(path, mode, flag).move();
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
}
}


