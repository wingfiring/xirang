#include <iostream>
#include <xirang/versionedvfs.h>
#include <xirang/fsutility.h>
#include <xirang/vfs/local.h>
#include <xirang/vfs/inmemory.h>
#include <xirang/vfs/subvfs.h>

#include <xirang/io/exchs11n.h>
#include <xirang/io/path.h>

#include <xirang/io/file.h>
#include <xirang/io/memory.h>

#include <unordered_map>

using namespace xirang;
using std::cout;
using std::cerr;
using std::endl;

void cmd_init(int argc, char** argv){
	if (argc != 0){
		cerr << "init \n";
		return;
	}

	file_path dir;
	vfs::LocalFs vfs(dir);
	dir = file_path("mygit");
	vfs.createDir(dir);
	vfs.createDir(dir/file_path("stage"));
	auto err = vfs::initRepository(vfs, dir);

	if (err == fs::er_ok){
		std::cout << "OK.\n";
	}
	else
		std::cout << "Error:" << int(err) << std::endl;
}

void log_submission(){
	vfs::LocalFs vfs(file_path("mygit"));
	vfs::RepoManager rpmgr;

	auto repo = rpmgr.getRepo(vfs, sub_file_path());
	auto sub = repo->getSubmission(version_type());
	if (sub.version == version_type()){
		cout << "Empty repo\n";
	}

	for(;;){
		cout << sub.version.id << "\t" << sub.tree.id << endl;
		if (!is_empty(sub.prev))
			sub = repo->getSubmission(sub.prev);
		else
			break;
	}
}
void log_file(const file_path& p){
	vfs::LocalFs vfs(file_path("mygit"));
	vfs::RepoManager rpmgr;

	auto repo = rpmgr.getRepo(vfs, sub_file_path());

	for(auto& i : repo->history(p)){
		cout << i.submission.id << "\t" << i.version.id << endl;
	}
}

void cmd_log(int argc, char** argv){
	if (argc > 1){
		cerr << "log [path]\n";
		return;
	}

	if (argc == 0){
		log_submission();
	}
	else {
		log_file(file_path(argv[0]));
	}
}

void cmd_add(int argc, char** argv){
	if (argc < 1){
		cout << "usage: add files ...\n";
		return;
	}

	vfs::LocalFs wkfs(file_path("mygit/stage"));
	vfs::Workspace wk(wkfs, string());

	for (int i = 0; i < argc; ++i){
		try{
			file_path path(argv[i]);
			io::file_reader rd(path);
			iref<io::read_map> src(rd);
			auto dest = vfs::recursive_create<io::write_map>(wk, path, io::of_create_or_open);
			copy_data(src.get<io::read_map>(), dest.get<io::write_map>());

			cout << "\t" << path.str() << " added\n";
		}
		catch(exception& e){
			cout << "Failed to add file " << argv[i] << endl;
			cout << e.what() << endl;
		}
	}

}

void cmd_rm(int argc, char** argv){
	if (argc < 1){
		cout << "usage: add files ...\n";
		return;
	}
	vfs::LocalFs rpfs(file_path("mygit"));
	auto f = rpfs.create<io::random, io::writer>(file_path("wkremove"), io::of_create_or_open);
	auto& seek = f.get<io::random>();
	seek.seek(seek.size());

	auto sink = io::exchange::as_sink(f.get<io::writer>());
	for (int i = 0; i < argc; ++i)
		sink & file_path(argv[i]);
}

void show_file(vfs::IVfs& fs, const file_path& dir){
	for (auto& i: fs.children(dir)){
		file_path path = dir/i.path;
		if (fs.state(path).state == fs::st_regular){
			cout << "\t" << path.str() << endl;
		}
		else
			show_file(fs, path);
	}
}


void do_show_stage(){
	vfs::LocalFs wkfs(file_path("mygit/stage"));
	vfs::Workspace wk(wkfs, string() );

	try{
		vfs::LocalFs rpfs(file_path("mygit"));
		auto f = rpfs.create<io::reader>(file_path("wkremove"), io::of_open);
		auto source = io::exchange::as_source(f.get<io::reader>());
		while(f.get<io::reader>().readable()){
			wk.markRemove(load<file_path>(source));
		}
	}
	catch(exception&){}

	cout << "Added:\n";
	show_file(wk, file_path());

	cout << "\nRemoved:\n";
	for (auto& i : wk.allRemoved()){
		cout << "\t" << i.str() << endl;
	}
}

void cmd_show(int argc, char** argv){
	if (argc == 0){
		cout << "show \n"
			<< "\tstage\n"
			;
		return;
	}
	if (string((const char*)argv[0]) == string("stage")){
		do_show_stage();
	}
}
void cmd_checkin(int argc, char** argv){
	version_type base;


	vfs::LocalFs vfs(file_path("mygit"));
	vfs::LocalRepository repo(vfs, file_path());

	vfs::LocalFs wkfs(file_path("mygit/stage"));
	vfs::Workspace wk(wkfs, string());
	try{
		auto f = vfs.create<io::reader>(file_path("wkremove"), io::of_open);
		auto source = io::exchange::as_source(f.get<io::reader>());
		while(f.get<io::reader>().readable()){
			wk.markRemove(load<file_path>(source));
		}
	}
	catch(exception&){}

	if (argc != 0){
		base = version_type(sha1_digest(as_range_string(argv[0])));
	}
	else {
		base = repo.getSubmission(version_type()).version;
	}

	auto c = repo.commit(wk, string("test"), base);
	if (c.flag == vfs::bt_none)
		cout << "Commit failed\n";
	else {
		cout << c.version.id << endl;
	}
}

void cmd_cat(int argc, char** argv){
	if (argc != 1){
		cout << "cat <file id>\n";
		return;
	}
	version_type id(sha1_digest(as_range_string(argv[0])));

	vfs::LocalFs vfs(file_path("mygit"));
	vfs::LocalRepository repo(vfs, file_path());

	auto type = repo.blobType(id);
	switch(type){
		case vfs::bt_file:
			{
				auto f = repo.create<io::read_map>(file_path(string(string("#") << id.id.to_string())), io::of_open);
				io::mem_archive mar;
				io::copy_data<io::read_map, io::writer>(f.get<io::read_map>(), mar);
				cout.write((const char*)mar.data().begin(), mar.data().size());
				cout << endl;
			}
			break;
		case vfs::bt_tree:
			{
				cout << "tree:\n";
				for (auto& i: repo.treeItems(id)){
					cout << i.version.id << "\t" << i.name.str() << endl;
				}
			}
			break;
		case vfs::bt_submission:
			{
				auto s = repo.getSubmission(id);
				cout << "Submission:" << s.version.id << "\t" << s.description << endl;
			}
			break;
		default:
			cout << "Not found\n";
	}

}

typedef std::function<void(int, char**)> command_type;
std::unordered_map<std::string, command_type> command_table = {
	{std::string("init"), cmd_init},
	{std::string("log"), cmd_log},
	{std::string("add"), cmd_add},
	{std::string("rm"), cmd_rm},
	{std::string("show"), cmd_show},
	{std::string("ci"), cmd_checkin},
	{std::string("cat"), cmd_cat}
};

void print_help(){
	cout << "command list:\n";
	for (auto& i : command_table){
			cout << i.first << endl;
	}
}
int do_main(int argc, char** argv){
	if (argc < 2){
		print_help();
		return 1;
	}

	std::string command(argv[1]);

	auto pos = command_table.find(command);
	if (pos == command_table.end()){
		print_help();
		return 1;
	}

	pos->second(argc - 2, argv + 2);
	return 0;
}

int main(int argc, char** argv){
	try{
		return do_main(argc, argv);
	}catch(exception& e){
		cerr << e.what() << endl;
	}
}

