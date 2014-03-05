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

void add_dir(xirang::zip::reader_writer& zip, xirang::vfs::IVfs& vfs, const xirang::file_path& dir, const xirang::file_path& prefix){
	for(auto& i : vfs.children(dir)){
		auto filename = dir / i.path;
		auto st = vfs.state(filename);
		if (st.state == xirang::fs::st_dir){
			add_dir(zip, vfs, filename, prefix);
		}
		else if (st.state == xirang::fs::st_regular){
			auto fin = vfs.create<xirang::io::read_map>(filename, xirang::io::of_open);
			zip.append(fin.get<xirang::io::read_map>(), prefix / filename);
		}
		else {
			std::cout << "Unrecongnized file: " << filename.str() << std::endl;
		}

	}
}
void add_file(xirang::zip::reader_writer& zip, const xirang::file_path& file){
	xirang::io::file_reader  f(file);
	xirang::iref<xirang::io::read_map> fmap(f);
	zip.append(fmap.get<xirang::io::read_map>(), file.filename());
}

void add(const char* dest, char** src){
	auto path = xirang::file_path(xirang::string(dest), xirang::pp_utf8check);
	auto file = xirang::fs::recursive_create(path, xirang::io::of_create_or_open);
	xirang::iref<xirang::io::read_map, xirang::io::write_map> ifile(file);
	xirang::zip::reader_writer zip(ifile);

	for (;*src; ++src){
		const char* s = *src;
		auto input = xirang::file_path(xirang::string(s), xirang::pp_utf8check);
		auto st = xirang::fs::state(input).state;
		if (st == xirang::fs::st_regular){
			add_file(zip, input);
		}
		else if (st == xirang::fs::st_dir){
			xirang::vfs::LocalFs vfs(input);
			add_dir(zip, vfs, xirang::file_path(), input.filename());
		}
		else {
			std::cerr << "Failed to read file " << input.str() << std::endl;
		}

	}
}

void list(const char* src)
{
	auto path = xirang::file_path(xirang::string(src), xirang::pp_utf8check);
	xirang::io::file_reader file(path);
	xirang::iref<xirang::io::read_map> file_map(file);
	xirang::zip::reader reader(file_map.get<xirang::io::read_map>());
	for (auto &i : reader.items()){
		std::cout << i.name.str() << " \t" << i.compressed_size << "\t" << i.uncompressed_size
			<< "\n";
	}
}

void extract(const char* src, const char* dest_dir){
	auto path = xirang::file_path(xirang::string(src), xirang::pp_utf8check);
	auto dest_path = xirang::file_path(xirang::string(dest_dir == 0 ? "." : dest_dir), xirang::pp_utf8check);
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
			if (i.method == 0){
				xirang::io::copy_data(rd.get<xirang::io::read_map>(), dest.get<xirang::io::write_map>());
				auto crc = xirang::zip::crc32(dest.get<xirang::io::read_map>());
				if (crc == i.crc32)
					std::cout << "\tcrc32 OK.";
				else
					std::cout << "\tcrc32 error.";

				std::cout << "\tOK.\n";
			}
			else if(i.method == 8){

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

}

int main(int argc, char** argv){
	if (argc < 3) usage();
	if (argv[1] == std::string("add")){
		//if (argc < 4) usage();
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
