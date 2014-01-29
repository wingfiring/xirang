#include <xirang/deflate.h>
#include <xirang/io/file.h>


#include <iostream>

void usage(){
	std::cout << "inflate src dest \n";
	exit(1);
}

using namespace xirang;

int main(int argc, char** argv){
	if (argc < 3) usage();
	try{
		file_path src(argv[1]);
		file_path dest(argv[2]);

		io::file_reader reader(src);
		io::file_writer writer(dest, io::of_create_or_open);

		iref<io::read_map> rd(reader);
		iref<io::write_map> wr(writer);

		auto result = zip::inflate(rd.get<io::read_map>(), wr.get<io::write_map>());

		std::cout << result.err << "\n"
			<< result.in_size << "\n"
			<< result.out_size << "\n";
	}
	catch (...){
		std::cout << "Failed to inflate file\n";
	}
	return 0;
}


//
// g++ -g -std=c++11 -o inflate inflate.cpp -I../../ -I/usr/local/include -L../../build_dir/debug/lib/ -L/usr/local/lib -lxirang -lz -lboost_system -lboost_filesystem
//
