#include <xirang/io/file.h>
#include <xirang/sha1.h>

#include <iostream>
#include <iomanip>

int main(int argc, char** argv){
	for (int i = 1; i < argc; ++i){
		auto path = xirang::file_path(xirang::string((const char*)argv[i]), xirang::pp_utf8check);
		xirang::io::file_reader file(path);
		auto view = file.view_rd(xirang::ext_heap::handle(0, file.size()));
		auto addr = view->address();
		xirang::sha1 sha;
		sha.process_block(addr.begin(), addr.begin() + 10);
		sha.process_block(addr.begin() + 10, addr.end());
		auto & dig = sha.get_digest();

		std::cout << dig;
		std::cout << "  " << argv[i] << std::endl;

		auto dig2 = from_string(to_string(dig));
		std::cout << dig;
		std::cout << "  " << argv[i] << std::endl;


	}

	return 0;
}
