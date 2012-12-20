#include <aio/xirang/vfs/inmemory.h>
#include <aio/common/archive/mem_archive.h>

#include <aio/xirang/vfs/vfs_common.h>
#include <aio/common/archive/adaptor.h>
#include <aio/common/string_algo/string.h>

#include "file_tree.h"

namespace xirang{ namespace fs{ 

	using namespace aio::archive;

	class InMemoryImp
	{
		typedef private_::file_node<buffer<byte>> file_node;
		public:

		InMemoryImp(const string& res, IVfs* host)
			: m_resource(res), m_host(host), m_root(0), m_readonly(false)
		{
		}

		~InMemoryImp()
		{ }

		fs_error remove(const string& path)
		{
            AIO_PRE_CONDITION(!is_absolute(path));
            if (m_readonly)
                return aiofs::er_permission_denied;

            fs_error ret = remove_check(*host(), path);
			if (ret == aiofs::er_ok)
			{
				auto pos = locate(m_root_node, path);
				AIO_PRE_CONDITION(pos.node);
                return pos.not_found.empty()
					? removeNode(pos)
					: aiofs::er_not_found;
			}

			return ret;
		}
		// dir operations
		// \pre !absolute(path)
		fs_error createDir(const  string& path)
		{
			AIO_PRE_CONDITION(!is_absolute(path));
            if (m_readonly)
                return aiofs::er_permission_denied;

			auto pos = locate(m_root_node, path);
			if (pos.not_found.empty())
				return aiofs::er_exist;
			if (pos.node->type != aiofs::st_dir)
				return aio::er_not_dir;

			if (aio::contains(pos.not_found, '/'))
				return aiofs::er_not_found;

			create_node(pos, aiofs::st_dir);
			return aiofs::er_ok;
		}


		// file operations
		aio::io::buffer_io create(const string& path, int flag)
		{
			AIO_PRE_CONDITION(!is_absolute(path));

            if ( m_readonly) AIO_THROW(PermisionDenied);

            auto pos = locate(m_root_node, path);
            if(flag == of_create && pos.not_found.empty()) AIO_THROW(FileExist);
			if(flag == of_open && (!pos.not_found.empty() || pos.node == &m_root_node)) AIO_THROW(FileNotFound);
			if (pos.node->type != aiofs::st_regular || aio::contains(pos.not_found, '/'))
				AIO_THROW(FileNotFound);

			if (!pos.not_found.empty())
			{
				auto res = create_node(pos, aiofs::st_regular);
				return buffer_io(res->data)
			}
			else
				return buffer_io(pos.node->data);
		}

		aio::io::buffer_in readOpen(const string& path){
			AIO_PRE_CONDITION(!is_absolute(path));

            auto pos = locate(m_root_node, path);
			if(!pos.not_found.empty() || pos.node == &m_root_node) AIO_THROW(FileNotFound);
			if (pos.node->type != aiofs::st_regular || aio::contains(pos.not_found, '/'))
				AIO_THROW(FileNotFound);

			return buffer_in(pos.node->data);
		}

		// \pre !absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		// otherwise, from should be a
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
				return aiofs::er_system_error;

			pos->data->resize(s);
			return aiofs::er_ok;

		}

		void sync() { }

		// query
		const string& resource() const { return m_resource;}

		// volume
		// if !mounted, return null
		RootFs* root() const{ return m_root;}

		// \post mounted() && root() || !mounted() && !root()
		bool mounted() const { return m_root != 0; }

		// \return mounted() ? absolute() : empty() 
		string mountPoint() const 
		{
			return m_root ? m_root->mountPoint(*m_host) : "";
		}

		// \pre !absolute(path)
		VfsNodeRange children(const string& path) const 
		{
			file_node* pos = locate(const_cast<file_node&>(m_root_node), path);
			typedef private_::FileNodeIterator<iarchive*> FileIterator;
			if (pos && pos->type == aiofs::st_dir)
			{
                auto beg = pos->children.begin();
                auto end = pos->children.end();
				return VfsNodeRange(
                    VfsNodeRange::iterator(FileIterator(beg, m_host)),
					VfsNodeRange::iterator(FileIterator(end, m_host))
						);
			}
			return VfsNodeRange();
		}


		// \pre !absolute(path)
		VfsState state(const string& path) const
		{
			AIO_PRE_CONDITION(!is_absolute(path));
			VfsState fst = 
			{
				{ path, m_host}, aiofs::st_not_found, 0
			};
			auto pos = locate(const_cast<file_node&>(m_root_node), path);
			if (pos.not_found.empty())
			{
				fst.state = pos->type;
				fst.size = pos.node->data.size();
			}
			return fst;
		}

		void setRoot(RootFs* r) 
		{
			AIO_PRE_CONDITION(!mounted() || r == 0);
			m_root = r;
		}
		IVfs* host() const { return m_host;}

        any getopt(int id, const any & optdata) const {
            if (id == vo_readonly)
                return m_readonly;
            return any();
        }

        any setopt(int id, const any & optdata,  const any & /*indata*/){
            if (id == vo_readonly){
                m_readonly = aio::any_cast<bool>(optdata);
                return any(m_readonly);
            }
            return any();
        }

		private:

		file_node m_root_node;
		string m_resource;
		IVfs* m_host;
		RootFs* m_root;

        bool m_readonly;
	};


	InMemory::InMemory(const string& resource)
		: m_imp(new InMemoryImp(resource, this))
	{}

	InMemory::~InMemory() { delete m_imp;}

	// common operations of dir and file
	// \pre !absolute(path)
	fs_error InMemory::remove(const string& path)
	{
		return m_imp->remove(path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error InMemory::createDir(const  string& path) { return m_imp->createDir(path);}

	// file operations
	aio::io::buffer_io InMemory::create(const string& path, int flag){
		return m_imp->create(path, flag);
	}
	aio::io::buffer_in readOpen(const string& path){
		return m_imp->readOpen(path);
	}

	// \pre !absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	// otherwise, from should be a
	fs_error InMemory::copy(const string& from, const string& to)
	{
		return m_imp->copy(from, to);
	}

	fs_error InMemory::truncate(const string& path, aio::long_size_t s)
	{
		return m_imp->truncate(path, s);
	}

	void InMemory::sync() { return m_imp->sync();}

	// query
	const string& InMemory::resource() const { return m_imp->resource();}

	// volume
	// if !mounted, return null
	RootFs* InMemory::root() const { return m_imp->root();}

	// \post mounted() && root() || !mounted() && !root()
	bool InMemory::mounted() const { return m_imp->mounted(); }

	// \return mounted() ? absolute() : empty() 
	string InMemory::mountPoint() const { return m_imp->mountPoint();}

	// \pre !absolute(path)
	VfsNodeRange InMemory::children(const string& path) const { return m_imp->children(path); }

	// \pre !absolute(path)
	VfsState InMemory::state(const string& path) const { return m_imp->state(path); }
	// if r == null, means unmount
	void InMemory::setRoot(RootFs* r) { return m_imp->setRoot(r); }

	any InMemory::getopt(int id, const any & optdata /*= any() */) const 
	{
		return m_imp->getopt(id, optdata);
	}
	any InMemory::setopt(int id, const any & optdata,  const any & indata/*= any()*/)
	{
		return m_imp->setopt(id, optdata, indata);
	}

}}



