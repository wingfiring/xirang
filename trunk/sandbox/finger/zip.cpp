#include <xirang/zip.h>
#include <xirang/vfs/local.h>
#include <xirang/io/file.h>


#include <iostream>

void usage(){
	std::cout <<
		"zip add <zip file> <folder> ... \n"
		"zip list <zip file> \n"
		"zip extract <zip file> [dest]\n";
	exit(1);
}

void add(const char* dest, char** src){
}

void list(const char* src)
{
	auto path = xirang::file_path(xirang::string(src));
	xirang::io::file_reader file(path);
	xirang::iref<xirang::io::read_map> file_map(file);
	xirang::zip::reader reader(file_map.get<xirang::io::read_map>());
	for (auto &i : reader.items()){
		std::cout << i.name.str() << " \t" << i.compressed_size << "\t" << i.uncompressed_size
			<< "\n";
	}
}

void extract(const char* src, const char* dest_dir){
	auto path = xirang::file_path(xirang::string(src));
	auto dest_path = xirang::file_path(xirang::string(dest_dir == 0 ? "." : dest_dir));
	if (!xirang::fs::exists(dest_path) && xirang::fs::recursive_create_dir(dest_path) != xirang::fs::er_ok){
		std::cerr << "Failed to create dir " << dest_path.str() << "\n";
		exit(2);
	}
		
		
	xirang::vfs::LocalFs local_fs(dest_path);

	xirang::io::file_reader file(path);
	xirang::iref<xirang::io::read_map> file_map(file);
	xirang::zip::reader reader(file_map.get<xirang::io::read_map>());
	const uint32_t dir_mask = 0x10;
	for (auto &i : reader.items()){
		std::cout << i.name.str() << " \t" << i.compressed_size << "\t" << i.uncompressed_size;

		if (i.external_attrs & dir_mask){
			xirang::vfs::recursive_create_dir(local_fs, i.name);
			std::cout << "\tOK.\n";
		}
		else {
			auto rd = xirang::zip::open_raw(i);
			auto dest = xirang::vfs::recursive_create<xirang::io::write_map, xirang::io::read_map>(local_fs, i.name, xirang::io::of_create_or_open);
			auto ret = xirang::zip::inflate(rd.get<xirang::io::read_map>(), dest.get<xirang::io::write_map>());
			if (ret.err == 0){
				auto crc = xirang::zip::crc32(dest.get<xirang::io::read_map>());
				if (crc == i.crc32)
					std::cout << "\tcrc32 OK.";
				else 
					std::cout << "\tcrc32 error.";

				std::cout << "\tOK.\n";
			}
			else
				std::cout << "\nerr:" << ret.err 
					<< "\tcompressed_size:" << ret.in_size
					<< "\tuncompressed_size:" << ret.out_size
					<< "\n";
		}
	}

}

int main(int argc, char** argv){
	if (argc < 3) usage();
	if (argv[1] == std::string("add")){
		if (argc < 4) usage();
		add (argv[2], argv + 3);
	}
	else if (argv[1] == std::string("list")){
		list (argv[2]);
	}
	else if (argv[1] == std::string("extract")){
		extract (argv[2], argv[3]);
	}

	return 0;
}


// 
// g++ -g -std=c++11 -o zip zip.cpp -I../../ -I/usr/local/include -L../../build_dir/debug/lib/ -L/usr/local/lib -lxirang -lz -lboost_system -lboost_filesystem
//
