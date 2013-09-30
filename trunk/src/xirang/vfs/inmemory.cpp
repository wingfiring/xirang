#include <xirang/vfs/inmemory.h>
#include <xirang/io/memory.h>

#include <xirang/vfs/vfs_common.h>
#include <xirang/string_algo/string.h>
#include <xirang/buffer.h>

#include "file_tree.h"

namespace xirang{ namespace vfs{ 

	class InMemoryImp
	{
		typedef private_::file_node<buffer<byte> > file_node;
		public:

		InMemoryImp(const string& res, IVfs* host)
			: m_resource(res), m_host(host), m_root(0), m_readonly(false)
		{
		}

		~InMemoryImp()
		{ }

		fs_error remove(sub_file_path path)
		{
            AIO_PRE_CONDITION(!path.is_absolute());
            if (m_readonly)
                return fs::er_permission_denied;

            fs_error ret = remove_check(*host(), path);
			if (ret == fs::er_ok)
			{
				auto pos = locate(m_root_node, path);
				AIO_PRE_CONDITION(pos.node);
                return pos.not_found.empty()
					? removeNode(pos.node)
					: fs::er_not_found;
			}

			return ret;
		}
		// dir operations
		// \pre !absolute(path)
		fs_error createDir(sub_file_path path)
		{
			AIO_PRE_CONDITION(!path.is_absolute());
            if (m_readonly)
                return fs::er_permission_denied;

			auto pos = locate(m_root_node, path);
			if (pos.not_found.empty())
				return fs::er_exist;
			if (pos.node->type != fs::st_dir)
				return fs::er_not_dir;

			if (!pos.not_found.parent().empty())
				return fs::er_not_found;

			auto ret = create_node(pos, fs::st_dir, false);
			unuse(ret);
			AIO_POST_CONDITION(ret);
			return fs::er_ok;
		}


		// file operations
		io::buffer_io create(sub_file_path path, int flag)
		{
			AIO_PRE_CONDITION(!path.is_absolute());

            if ( m_readonly) AIO_THROW(fs::permission_denied_exception);

            auto pos = locate(m_root_node, path);
            if((flag & io::of_open_create_mask) == io::of_create && pos.not_found.empty()) AIO_THROW(fs::exist_exception);
			if((flag & io::of_open_create_mask) == io::of_open && !pos.not_found.empty()) AIO_THROW(fs::not_found_exception);
			if (pos.node->type != fs::st_regular || pos.not_found.parent().empty())
				AIO_THROW(fs::not_found_exception);

			if (!pos.not_found.empty())
			{
				auto res = create_node(pos, fs::st_regular, false);
				return io::buffer_io(res->data);
			}
			else
				return io::buffer_io(pos.node->data);
		}

		io::buffer_in readOpen(sub_file_path path){
			AIO_PRE_CONDITION(!path.is_absolute());

            auto pos = locate(m_root_node, path);
			if(!pos.not_found.empty()) AIO_THROW(fs::not_found_exception);
			if (pos.node->type != fs::st_regular)
				AIO_THROW(fs::file_type_exception);

			return io::buffer_in(pos.node->data);
		}

		// \pre !absolute(to)
		// if from and to in same fs, it may have a more effective implementation
		// otherwise, from should be a
		fs_error copy(sub_file_path from, sub_file_path to)
		{
            AIO_PRE_CONDITION(!to.is_absolute());
            if (m_readonly)
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

		fs_error truncate(sub_file_path path, long_size_t s)
		{
            AIO_PRE_CONDITION(!path.is_absolute());
            if (m_readonly)
                return fs::er_permission_denied;

			auto pos = locate(m_root_node, path);
			if (!pos.not_found.empty())
				return fs::er_not_found;
			if (pos.node->type != fs::st_regular)
				return fs::er_system_error;

			pos.node->data.resize(s);
			return fs::er_ok;

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
		file_path mountPoint() const 
		{
			return m_root ? m_root->mountPoint(*m_host) : file_path();
		}

		// \pre !absolute(path)
		VfsNodeRange children(sub_file_path path) const 
		{
			auto pos = locate(const_cast<file_node&>(m_root_node), path);
			typedef private_::FileNodeIterator<buffer<byte> > FileIterator;
			if (pos.not_found.empty() && pos.node->type == fs::st_dir)
			{
                auto beg = pos.node->children.begin();
                auto end = pos.node->children.end();
				return VfsNodeRange(
                    VfsNodeRange::iterator(FileIterator(beg, m_host)),
					VfsNodeRange::iterator(FileIterator(end, m_host))
						);
			}
			return VfsNodeRange();
		}


		// \pre !absolute(path)
		VfsState state(sub_file_path path) const
		{
			AIO_PRE_CONDITION(!path.is_absolute());
			VfsState fst = 
			{
				{ path, m_host}, fs::st_not_found, 0
			};
			auto pos = locate(const_cast<file_node&>(m_root_node), path);
			if (pos.not_found.empty())
			{
				fst.state = pos.node->type;
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
                return any(m_readonly);
            return any();
        }

        any setopt(int id, const any & optdata,  const any & /*indata*/){
            if (id == vo_readonly){
                m_readonly = any_cast<bool>(optdata);
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

	InMemory::~InMemory() { check_delete(m_imp);}

	// common operations of dir and file
	// \pre !absolute(path)
	fs_error InMemory::remove(sub_file_path path)
	{
		return m_imp->remove(path);
	}

	// dir operations
	// \pre !absolute(path)
	fs_error InMemory::createDir(sub_file_path path) { return m_imp->createDir(path);}

	// file operations
	io::buffer_io InMemory::writeOpen(sub_file_path path, int flag){
		return m_imp->create(path, flag);
	}
	io::buffer_in InMemory::readOpen(sub_file_path path){
		return m_imp->readOpen(path);
	}

	// \pre !absolute(to)
	// if from and to in same fs, it may have a more effective implementation
	// otherwise, from should be a
	fs_error InMemory::copy(sub_file_path from, sub_file_path to)
	{
		return m_imp->copy(from, to);
	}

	fs_error InMemory::truncate(sub_file_path path, long_size_t s)
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
	file_path InMemory::mountPoint() const { return m_imp->mountPoint();}

	// \pre !absolute(path)
	VfsNodeRange InMemory::children(sub_file_path path) const { return m_imp->children(path); }

	// \pre !absolute(path)
	VfsState InMemory::state(sub_file_path path) const { return m_imp->state(path); }
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
	void** InMemory::do_create(unsigned long long mask,
			void** base, unique_ptr<void>& owner, sub_file_path path, int flag){

		void** ret = 0;
		if (mask & detail::get_mask<io::writer, io::write_map>::value ){ //write open
			unique_ptr<io::buffer_io> ar(new io::buffer_io(writeOpen(path, flag)));
			iref<io::reader, io::writer, io::random, io::ioctrl, io::read_map, io::write_map> ifile(*ar);
			ret = copy_interface<io::reader, io::writer, io::random, io::ioctrl, io::read_map, io::write_map>::apply(mask, base, ifile, (void*)ar.get()); 
			unique_ptr<void>(std::move(ar)).swap(owner);
		}
		else{ //read open
			unique_ptr<io::buffer_in> ar(new io::buffer_in(readOpen(path)));
			iref<io::reader, io::random, io::read_map> ifile(*ar);
			ret = copy_interface<io::reader, io::random, io::read_map>::apply(mask, base, ifile, (void*)ar.get()); 
			unique_ptr<void>(std::move(ar)).swap(owner);
		}
		return ret;
	}
}}


