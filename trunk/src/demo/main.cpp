#include <aio/xirang/xirang.h>

#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <functional>
#include <unordered_map>

#include <aio/xirang/vfs.h>
#include <aio/xirang/xirang.h>
#include <aio/xirang/object.h>
#include <aio/xirang/vfs/local.h>
#include <aio/xirang/vfs/zip.h>
#include <aio/common/io/file.h>
#include <aio/common/io/memory.h>

#include <set>
#include <map>
#include <algorithm>
#include <utility>

#include "typeparser.h"


using namespace std::placeholders;
aio::ext_heap& get_ext_heap()
{
	return *(aio::ext_heap*)0;
}

class session
{
	typedef std::function<void (const std::string&)> command_handler;
	typedef std::unordered_map<std::string, command_handler> dispatcher_type;
public:
	session() 
		: m_dispatcher(0)
		, m_initialized(false)
		, m_vfs("demo")
		, m_xr("demo", aio::memory::get_global_heap(), get_ext_heap())
	{
		m_dispatcher = & m_global_dispatcher;
		m_prompt = ">>";
		m_pwd = "/";

		m_global_dispatcher["vfs"] = std::bind(&session::cmd_vfs, this, _1);
		m_global_dispatcher["xirang"] = std::bind(&session::cmd_xirang, this, _1);
		m_global_dispatcher["help"] = std::bind(&session::cmd_help, this, _1);
		m_global_dispatcher["?"] = std::bind(&session::cmd_help, this, _1);

		m_vfs_dispatcher[".."] = std::bind(&session::cmd_global, this, _1);
		m_vfs_dispatcher["?"] = std::bind(&session::cmd_vfs_help, this, _1);
		m_vfs_dispatcher["help"] = std::bind(&session::cmd_vfs_help, this, _1);

		m_vfs_dispatcher["md"] = std::bind(&session::cmd_vfs_md, this, _1);
		m_vfs_dispatcher["ls"] = std::bind(&session::cmd_vfs_ls, this, _1);
		m_vfs_dispatcher["pwd"] = std::bind(&session::cmd_vfs_pwd, this, _1);
		m_vfs_dispatcher["cd"] = std::bind(&session::cmd_vfs_cd, this, _1);
		m_vfs_dispatcher["cp"] = std::bind(&session::cmd_vfs_cp, this, _1);
		m_vfs_dispatcher["cat"] = std::bind(&session::cmd_vfs_cat, this, _1);
		m_vfs_dispatcher["rm"] = std::bind(&session::cmd_vfs_rm, this, _1);
		m_vfs_dispatcher["mount"] = std::bind(&session::cmd_vfs_mount, this, _1);
		m_vfs_dispatcher["unmount"] = std::bind(&session::cmd_vfs_unmount, this, _1);

		m_xr_dispatcher[".."] = std::bind(&session::cmd_global, this, _1);
		m_xr_dispatcher["?"] = std::bind(&session::cmd_xr_help, this, _1);
		m_xr_dispatcher["help"] = std::bind(&session::cmd_xr_help, this, _1);
		m_xr_dispatcher["init"] = std::bind(&session::cmd_xr_init, this, _1);
		m_xr_dispatcher["load"] = std::bind(&session::cmd_xr_load, this, _1);
		m_xr_dispatcher["dump"] = std::bind(&session::cmd_not_ready, this, _1);
		m_xr_dispatcher["def"] = std::bind(&session::cmd_not_ready, this, _1);
		m_xr_dispatcher["ls"] = std::bind(&session::cmd_xr_ls, this, _1);
		m_xr_dispatcher["lstype"] = std::bind(&session::cmd_xr_lstype, this, _1);
	}

	void process(const std::string& txt)
	{
		process_(txt);
		std::cout << m_prompt;
	}

	void cmd_vfs(const std::string&) { 
		m_prompt = "vfs>>";
		m_dispatcher = & m_vfs_dispatcher;
	}
	void cmd_xirang(const std::string& txt) { 
		m_prompt = "xr>>";
		m_dispatcher = & m_xr_dispatcher;
	}
	void cmd_help(const std::string& txt) { 
		std::cout <<
			"\tvfs	switch to vfs context.\n"
			"\txirang 	switch to xr context.\n"
			"\thelp 	print this help.\n"
			"\texit 	exit demo.\n";
	}

	void cmd_global(const std::string& txt){
		m_prompt = ">>";
		m_dispatcher = &m_global_dispatcher;
	}

	void cmd_not_ready(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		std::cout << *pos << " is not ready\n";
	}

	void cmd_xr_init(const std::string& txt){
		if (m_initialized)
		{
			std::cerr << "xirang is initialized\n";
			return;
		}

		SetupXirang(m_xr);
		m_initialized = true;
	}

	void cmd_xr_ls(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;
		if (pos == tok.end())
		{
			std::cerr << "ls <namespace>\n";
			return;
		}
		std::string path = *pos;

		auto ns = m_xr.root().locateNamespace(path.c_str(), '.');
		if (!ns.valid())
		{
			std::cerr << path << " not found\n";
			return;
		}

		auto sub_ns = ns.namespaces();
		auto types = ns.types();
		auto alias = ns.alias();
		auto objects = ns.objects();

		std::cout << "namespace:\n";
		std::for_each(sub_ns.begin(), sub_ns.end(), [](xirang::Namespace ns){ std::cout << ns.name().c_str() << " ";});
		std::cout << "\ntypes:\n";
		std::for_each(types.begin(), types.end(), [](xirang::Type t){ std::cout << t.name().c_str() << " ";});
		std::cout << "\nalias:\n";
		std::for_each(alias.begin(), alias.end(), [](xirang::TypeAlias a){ std::cout << "[" << a.name().c_str() << "," << a.type().fullName().c_str() << "] ";});
		std::cout << "\nobjects:\n";
        std::for_each(objects.begin(), objects.end(), [](xirang::NameValuePair obj){ std::cout << "[" << obj.name->c_str() 
            << "," << obj.value->type().name().c_str() 
            << "," << obj.value->data()
            << "] ";});
	}

	void cmd_xr_lstype(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;
		if (pos == tok.end())
		{
			std::cerr << "lstype <type>\n";
			return;
		}
		std::string path = *pos;

		auto type = m_xr.root().locateType(path.c_str(), '.');
		if (!type.valid())
		{
			std::cerr << path << " not found\n";
			return;
		}

		std::cout << "[" << type.name().c_str() << "," << type.fullName().c_str() 
			<< ", align=" << type.align() << ", payload=" << type.payload() << "]\n";

		auto bases = type.bases();
		auto members = type.members();
		auto args = type.args();

		if (type.argCount() > 0)
		{
			std::cout << "type args:\n";
			std::for_each(args.begin(), args.end(), [](xirang::TypeArg a){ std::cout << "[" << a.name().c_str() << "," << a.typeName().c_str();
					if (a.type().valid())
						std::cout << "(" << a.type().fullName() << ")";
					std::cout << "] ";});
			std::cout << "\n";
		}

		if (type.baseCount() > 0)
		{
			std::cout << "bases:\n";
			std::for_each(bases.begin(), bases.end(), [](xirang::TypeItem a){ std::cout << "[" << a.name().c_str() << " " 
					<< a.typeName().c_str();
					if (a.type().valid())
						std::cout << "(" << a.type().fullName() << ")";
					std::cout << " off=" << a.offset() << "] ";});
			std::cout << "\n";
		}

		if (type.memberCount() > 0)
		{
			std::cout << "members:\n";
			std::for_each(members.begin(), members.end(), [](xirang::TypeItem a){ std::cout << "[" << a.name().c_str() << " " << a.typeName().c_str();
					if (a.type().valid())
						std::cout << "(" << a.type().fullName() << ")";
					std::cout << " off=" << a.offset() << "] ";});
			std::cout << "\n";
		}
	}
	void cmd_xr_load(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;

		if (pos == tok.end())
		{
			std::cout << "load <type definition file>\n";
			return;
		}

		for (; pos != tok.end(); ++pos)
		{
			load_type(pos->c_str());
		}
	}

	void load_type(const aio::string& f)
	{
		aio::string file = f;
		if (!xirang::fs::is_absolute(file))
			file = m_pwd + file;

		auto state = m_vfs.locate(file);
		if (state.state == xirang::fs::st_not_found)
		{
			std::cerr << "file not found\n";
			return;
		}

		if (state.state != xirang::fs::st_regular)
		{
			std::cerr << "support regular file only.\n";
			return;
		}

		AIO_PRE_CONDITION(state.node.owner_fs != 0);

		auto pfile = state.node.owner_fs->create(state.node.path, (aio::archive::archive_mode)(aio::archive::mt_random | aio::archive::mt_read), aio::archive::of_open);
		if (!pfile)
		{
			std::cerr << "failed to open the file for reading\n";
			return;
		}

		aio::buffer<unsigned char> buf;
		aio::archive::buffer_out bout(buf);
		copy_archive(*pfile, bout);
		buf.push_back(0);
		buf.resize((std::size_t)state.size);

		TypeLoader loader;
		loader.load(buf, m_xr);
	}

	void cmd_xr_help(const std::string& txt){
		std::cout <<
			"\tinit 		init xirang\n"
			"\tload 		load type\n"
			"\tdump 		Not ready\n"
			"\tdef 		Not ready\n"
			"\tls 		list members of namespace\n"
			"\tlstype 		list members of type\n"

			"\thelp 		print this help\n"
			"\t..		switch to parent context\n"
			;
	}
	void cmd_vfs_help(const std::string& txt){
		std::cout <<
			"\tmd 		make dir\n"
			"\tls 		list files\n"
			"\tpwd 		print current dir\n"
			"\tcd 		change current dir\n"
			"\tcat 		print file content\n"

			"\trm 		remove file or directory\n"
			"\tcp 		copy file \n"
			"\tmount	 	mount file system\n"
			"\tunmount 	unmount file system\n"

			"\thelp		print this help\n"
			"\t..		switch to parent context\n"
			;
	}

	void cmd_vfs_rm(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;
		if (pos == tok.end())
		{
			std::cout << "rm <path of file or dir>\n";
			return;
		}

		aio::string path = 
			xirang::fs::is_absolute(pos->c_str())
			? pos->c_str()
			: xirang::fs::normalize(m_pwd + pos->c_str());
		
		auto st = m_vfs.locate(path);

		if (st.state == xirang::fs::st_not_found)
		{
			std::cerr << "file not found\n";
			return;
		}

		auto err = st.node.owner_fs->remove(st.node.path);
		if (err != xirang::fs::er_ok)
		{
			std::cerr << "remove failed\n" << (int)err  << "\n";
		}
	}

	void cmd_vfs_md(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;
		if (pos == tok.end())
		{
			std::cout << "md <path of dir>\n";
			return;
		}

		aio::string path = 
			xirang::fs::is_absolute(pos->c_str())
			? pos->c_str()
			: xirang::fs::normalize(m_pwd + pos->c_str());
		
		auto st = m_vfs.locate(path);

		if (st.state != xirang::fs::st_not_found)
		{
			std::cerr << "target is exist\n";
			return;
		}

		auto err = st.node.owner_fs->createDir(st.node.path);
		if (err != xirang::fs::er_ok)
		{
			std::cerr << "create failed\n" << (int)err  << "\n";
		}
	}

	void cmd_vfs_mount(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;

		if (pos == tok.end())
		{
			auto rng = m_vfs.mountedFS();
			std::cout << "The mounted filesystem:\n";

			for ( auto itr = rng.begin(); itr != rng.end(); ++itr)
			{
				std::cout << '\t' << (*itr).path << " <-- " 
					<< (*itr).fs->resource() << "\n";
			}
			return;
		}


		do {
			std::string sub_cmd = *pos;

			if (++pos == tok.end())
				break;
			aio::string mpoint = pos->c_str();

			if (++pos == tok.end())
				break;
			aio::string mpath = pos->c_str();


			if (!xirang::fs::is_absolute(mpoint))
			{
				std::cerr << "mount point must be absolute path\n";
				return;
			}

			if (m_vfs.mountPoint("/") == 0 && mpoint != "/")
			{
				std::cerr << "root should be mounted at first\n";
				return;
			}

			if (m_vfs.mountPoint(mpoint) != 0)
			{
				std::cerr << "mount point is used\n";
				return;
			}

			if (sub_cmd == "local")
			{
				if (++pos != tok.end())
					break;
				std::shared_ptr<xirang::fs::IVfs> pfs(new xirang::fs::LocalFs(mpath));
				if (m_vfs.mount(mpoint, *pfs) == xirang::fs::er_ok)
				{
					m_fs_map.insert(std::make_pair(pfs.get(), pfs));
				}
				return;
			}

			if(sub_cmd == "zip")
			{
				if (++pos == tok.end())
					break;

				aio::string cache = pos->c_str();

				if (++pos != tok.end())
					break;

				std::shared_ptr<aio::archive::iarchive> par (new aio::archive::file_read_write_archive(mpath, aio::archive::of_open));

				std::shared_ptr<xirang::fs::IVfs> pfs(new xirang::fs::LocalFs(cache));

				std::shared_ptr<xirang::fs::IVfs> zipfs(new xirang::fs::ZipFs(*par, mpath, *pfs));
				if (m_vfs.mount(mpoint, *zipfs) == xirang::fs::er_ok)
				{
                    m_fs_map.insert(std::make_pair(zipfs.get(), zipfs));
                    m_related_archive.insert(std::make_pair(zipfs.get(), par));
                    m_related_fs.insert(std::make_pair(zipfs.get(), pfs));
				}
				return;
			}

		} while(false);

		std::cout <<
			"\tmount -h\n"
			"\t\tprint this help\n"
			"\tmount local <mount point> <dir>\n"
			"\t\tmount a local filesystem\n"
			"\tmount zip <mount point> <file> <cache_fs>\n"
			"\t\tmount a zip file as filesystem\n"
			;

	}

	void cmd_vfs_unmount(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;

		if (pos == tok.end())
		{
			std::cout <<
				"\tunmount -h\n"
				"\t\tprint this help\n"
				"\tumount <mount point>\n"
				"\t\tumount the given file system\n"
				;
			return;
		}

		aio::string mpoint = xirang::fs::append_tail_slash(pos->c_str());
		auto pfs = m_vfs.mountPoint(mpoint);
		if (!pfs)
		{
			std::cerr << mpoint.c_str() << " is not a mount point\n";
			return;
		}

		auto err = m_vfs.unmount(mpoint);
		if (err == xirang::fs::er_ok)
		{
			m_fs_map.erase(pfs);            
            m_related_archive.erase(pfs);
            m_related_fs.erase(pfs);
		}
		else
		{
			std::cerr << "Failed code: " << perror(err) << "\n";
		}
	}

	const char* perror(xirang::fs::FsError err)
	{
		const char* err_msg[] = {
			"ok",
			"invalid",
			"busy_mounted",
			"not_found",
			"exist",
			"used_mount_point",
			"not_a_mount_point",
			"unmount_root",
			"fs_not_found",
			"system_error",
			"open_failed",
			"file_busy",
			"not_regular",
			"not_dir"
		};

		AIO_PRE_CONDITION(err <= xirang::fs::er_not_dir);
		return err_msg[int(err)];
	}

	void cmd_vfs_pwd(const std::string& txt){
		std::cout << m_pwd.c_str() << "\n";
	}
	void cmd_vfs_cd(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;

		if (pos == tok.end())
		{
			std::cout << m_pwd.c_str() << "\n";
			return;
		}

		aio::string dir = pos->c_str();
		if (++pos != tok.end())
		{
			std::cout << "\tcd  [dir]\n"
				"\t\t switch to new directory\n";
		}
		else
		{
			aio::string new_dir = dir;
			if (!xirang::fs::is_absolute(new_dir))
			{
				aio::string p = m_pwd + new_dir;
				new_dir = xirang::fs::append_tail_slash(xirang::fs::normalize(p));
			}
			auto st = m_vfs.locate(new_dir);
			if (st.state == xirang::fs::st_dir
					|| st.state == xirang::fs::st_mount_point)
				m_pwd = new_dir;
			else
				std::cerr << dir << " is not a valid dir\n";
		}

	}

	std::string base_name(const aio::string& file)
	{
		std::string p = file.c_str();
		auto idx = p.rfind("/");
		if (idx == std::string::npos)
			return p;

		return p.substr(++idx);
	}
	void cmd_vfs_cp(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;

		aio::string from, dest;
		if (pos != tok.end())
			from = aio::to_range(*pos++);
		if (pos != tok.end())
			dest = aio::to_range(*pos++);
		if (pos != tok.end() || from.empty() || dest.empty())
		{
			std::cout << "cp <src file>  <dest file>	copy file.\n";
			return;
		}



		aio::string path_src = 
			xirang::fs::is_absolute(from.c_str())
			? from.c_str()
			: xirang::fs::normalize(m_pwd + from.c_str());

		auto st = m_vfs.locate(path_src);

		if (st.state == xirang::fs::st_not_found)
		{
			std::cerr << "file not found\n";
			return;
		}
		else if (st.state != xirang::fs::st_regular)
		{
			std::cerr << "can copy from regular file only\n";
			return;
		}

		auto ar_src = st.node.owner_fs->create(st.node.path, (aio::archive::archive_mode)(aio::archive::mt_random | aio::archive::mt_read), aio::archive::of_open);
		if (!ar_src)
		{
			std::cerr << "failed to open source file\n";
		}


		aio::string path_dest = 
			xirang::fs::is_absolute(dest.c_str())
			? dest.c_str()
			: xirang::fs::normalize(m_pwd + dest.c_str());

		auto dst_st = m_vfs.locate(path_dest);

		if (dst_st.state == xirang::fs::st_not_found 
				|| dst_st.state == xirang::fs::st_regular)
		{
			auto ar_dst = dst_st.node.owner_fs->create(dst_st.node.path, (aio::archive::archive_mode)(aio::archive::mt_random | aio::archive::mt_write), aio::archive::of_create_or_open);
			if (!ar_dst)
			{
				std::cerr << "failed to open dest file\n";
				return;
			}
			copy_archive(*ar_src, *ar_dst);
			return;
		}
		else if (st.state != xirang::fs::st_dir)
		{
			return;
		}
		
		std::cerr << "create target file failed.\n";
		return;



	}

	void cmd_vfs_cat(const std::string& txt){
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;
		if (pos == tok.end())
		{
			std::cout << "cat <file> 	print text file.\n";
			return;
		}

		aio::string path = 
			xirang::fs::is_absolute(pos->c_str())
			? pos->c_str()
			: xirang::fs::normalize(m_pwd + pos->c_str());

		auto st = m_vfs.locate(path);

		if (st.state == xirang::fs::st_not_found)
		{
			std::cerr << "file not found\n";
		}
		else if (st.state != xirang::fs::st_regular)
		{
			std::cerr << "can cat regular file only\n";
		}
		else
		{
			auto ar = st.node.owner_fs->create(st.node.path, (aio::archive::archive_mode)(aio::archive::mt_random | aio::archive::mt_read), aio::archive::of_open);
			if (!ar)
			{
				std::cerr << "failed to open file\n";
			}
			else
			{
				aio::buffer<unsigned char> buf;
				aio::archive::buffer_out bout(buf);
				copy_archive(*ar, bout);

				buf.push_back(0);
				std::cout << &buf[0] << "\n";
			}
		}
	}
	void cmd_vfs_ls(const std::string& txt){

		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		AIO_PRE_CONDITION (pos != tok.end());
		++pos;
		if (pos == tok.end())
			ls_file(m_pwd);

		for (; pos != tok.end(); ++pos)
		{
			aio::string path = 
				xirang::fs::is_absolute(pos->c_str())
				? pos->c_str()
				: xirang::fs::normalize(m_pwd + pos->c_str())
				;
			std::cout << "\n" << path << ":\n";
			ls_file(path);
		}
	}

	void ls_file(const aio::string& path){

		auto st = m_vfs.locate(path);

		std::set<std::string> files;

		std::size_t max_size = 0;

		if (st.state == xirang::fs::st_dir
				|| st.state == xirang::fs::st_mount_point)
		{
			aio::string dir_parent =  st.node.path.empty() ? st.node.path : xirang::fs::append_tail_slash(st.node.path);
			auto rng = st.node.owner_fs->children(st.node.path);
			for (auto itr = rng.begin(); itr !=rng.end(); ++itr)
			{
				std::string name = base_name((*itr).path);
				auto fst = (*itr).owner_fs->state(dir_parent + (*itr).path);
				if (fst.state == xirang::fs::st_dir
						|| fst.state == xirang::fs::st_mount_point)
				{
					name += "/";
				}

				files.insert(name);
				max_size = std::max(max_size, name.size());
				std::cout << name << " ";
			}
		}
		else
		{
			std::cout << base_name(st.node.path) << "\n";
		}

		//std::for_each(files.begin(), files.end(), [](const std::string& s){ std::cout << s << " "; });
		std::cout << "\n";

	}
private:
	void process_(const std::string& txt)
	{
		typedef boost::char_separator<char> separator;

		boost::tokenizer<separator> tok(txt, separator(" \t\n"));
		auto pos = tok.begin();
		if (pos == tok.end())
			return;

		auto hpos =  m_dispatcher->find(*pos);
		if (hpos != m_dispatcher->end())
			hpos->second(txt);
		else
			std::cout << "command not found.\n";
	}

	dispatcher_type* m_dispatcher;

	dispatcher_type m_vfs_dispatcher;
	dispatcher_type m_xr_dispatcher;
	dispatcher_type m_global_dispatcher;

	std::string m_prompt;

	bool m_initialized;
	xirang::fs::RootFs m_vfs;
	xirang::Xirang m_xr;

	std::map<xirang::fs::IVfs*, std::shared_ptr<xirang::fs::IVfs> > m_fs_map; 
    std::map<xirang::fs::IVfs*, std::shared_ptr<xirang::fs::IVfs> > m_related_fs; 
    std::map<xirang::fs::IVfs*, std::shared_ptr<aio::archive::iarchive> > m_related_archive; 
	aio::string m_pwd;

	std::set<aio::archive::iarchive*> m_archives;
};

namespace sa = boost::algorithm;


int main()
{
	srand((unsigned int)time(0));
	session s;
	std::string text;
	s.process(text);

	while(std::getline(std::cin, text))
	{
		sa::trim(text);
		if (text == "exit")
		{
			std::cout << "Goodbye.\n";
			break;
		}
		try
		{
			s.process(text);
		}
		catch(aio::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
		catch(std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
		catch(...)
		{
			std::cerr << "unknown exception." << std::endl;
		}
	}

	return 0;
}
