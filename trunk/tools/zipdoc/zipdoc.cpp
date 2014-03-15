#include <xirang/zip.h>
#include <xirang/path.h>
#include <xirang/io/file.h>
#include <xirang/vfs/local.h>

#include <unordered_map>
#include <sstream>
#include <fstream>
#include <map>
#include <tuple>  
#include <fcgio.h>  

#include <boost/algorithm/string/replace.hpp>

using namespace xirang;

std::tuple<file_path, file_path, fs::file_state> locate(vfs::IVfs& vfs, const file_path& target)
{
	file_path found;
	fs::file_state state = fs::st_dir;
	for (auto i: target){
		file_path current = found / i;
		auto st = vfs.state(current);
		if (st.state == fs::st_not_found) break;
		else {
			found = current;
			if (st.state != fs::st_dir) {
				state = st.state;
				break;
			}
		}
	}

	file_path rest;
	if (target.str().size() > found.str().size()) {
		rest = sub_file_path(target.str().begin() + found.str().size() + 1, target.str().end());
	}
	return std::make_tuple(std::move(found), std::move(rest), state);
}

struct cache_info{
	std::unique_ptr<iauto<io::read_map>> ifile;
	zip::reader zip;
	std::size_t counter[2];

	explicit cache_info(iauto<io::read_map> && fzip)
		: ifile(new iauto<io::read_map>(std::move(fzip))), zip(ifile->template get<io::read_map>())
	{
		counter[0]=counter[1]=0;
	}

};
struct cache_manager{

	std::unordered_map<file_path, cache_info, hash_file_path> items;
	std::size_t limits;
	std::size_t index;
	vfs::IVfs& fs;
	cache_manager(vfs::IVfs& fs_) : limits(100), index(0), fs(fs_){
	}
	cache_info* get_info(const file_path& base){
		if (limits < items.size()){
			std::vector<std::pair<size_t, file_path> > vec;
			vec.reserve(items.size());
			for (auto & i : items){
				vec.push_back({i.second.counter[index], i.first});
			}
			auto pos = vec.begin() + vec.size()/3;
			nth_element(vec.begin(), pos, vec.end());
			for (auto i = vec.begin(); i != pos; ++i){
				items.erase(i->second);
			}
			index = (index + 1) & 1;
		}
		auto pos = items.find(base);
		if (pos != items.end())
		{
			++pos->second.counter[index];
			return &pos->second;
		}

		auto ar = fs.create<io::read_map>(base, io::of_open);
		if (!ar)
			return 0;
		auto res = items.insert(std::make_pair(base, cache_info(std::move(ar))));
		return &res.first->second;
	}

};

std::unordered_map<std::string, std::string> mime;
void load_mime(const char* path){
	std::ifstream fin(path);
	std::string line;
	while(getline(fin, line)){
		std::stringstream sstr(line);
		std::string mm;
		sstr >> mm;
		std::string ext;
		while (sstr >> ext){
			mime[ext] = mm;
		}
	}
}

std::string html_template;
const std::string dir_pattern = "<DIR_LIST_TABLE>";

void load_template(const char* path){
	std::ifstream fin(path);
	getline(fin, html_template, '\0');
}
void write_html(std::ostream& fout, const std::string& str){
	fout << boost::algorithm::replace_first_copy(html_template, dir_pattern, str);
}

void write_content_type(std::ostream& fout, const std::string& ext){
	auto pos = mime.find(ext);
	fout << "Content-type: ";
	if (pos != mime.end())
		fout << pos->second;
	else
		fout << "application/octet-stream";
	fout << "\r\n";
}

void response_file(std::ostream& fout, iref<io::read_map>& file){
	auto & rmap = file.get<xirang::io::read_map>();
	auto view = rmap.view_rd(xirang::ext_heap::handle(0, rmap.size()));
	auto address = view.get<xirang::io::read_view>().address();

	fout << "Cache-Control: public, max-age=7776000\r\n\r\n";
	fout.write((const char*)address.begin(), address.size());
}
void response_dir(std::ostream& fout, const file_path& path, int state, long_size_t size){
	fout << "<div><span class=\"file_type\">" << (state == fs::st_dir ? "DIR " : "FILE") << "</span>"
		"<span class=\"file_size\">" << size << "</span>"
		"<span class=\"file_name\"> <a href=\"" << (path.parent().filename() / path.filename()).str() << "\">" << path.filename().str() << "</a></span>";
}
void response_error(std::ostream& os, int code, const char* path = 0){
	os << "Status: " << code << "\r\n"
		"Content-type: text/html\r\n"
		"\r\n"  
		"Request file: " 
		<< (path ? path : "[no file path]")
		<< " not found\r\n";
}

const sub_file_path zip_ext = sub_file_path(literal("zip"));

int main(int argc, char** argv){
	if (argc != 4){
		std::cerr << "Usage: zipdoc <doc dir> <mime file> <html>\n";
		return 1;
	}

	load_mime(argv[2]);
	load_template(argv[3]);
	file_path doc_dir(argv[1]);

	if (fs::state(doc_dir).state != fs::st_dir){
		std::cerr << "path not found or not a dir\n";
		return 1;
	}
	vfs::LocalFs docfs(doc_dir);
	cache_manager cache(docfs);

	FCGX_Request request;

	FCGX_Init();
	FCGX_InitRequest (&request, 0, 0);

	while (FCGX_Accept_r (&request) == 0){
		fcgi_streambuf in_buf(request.in);
		fcgi_streambuf out_buf(request.out);
		std::istream fin(&in_buf);
		std::ostream fout(&out_buf);

		auto path = FCGX_GetParam("PATH_INFO", request.envp);
		if (!path){
			response_error(fout, 404);
			continue;
		}
		file_path doc_path = file_path(path);
		file_path base, rest;
		fs::file_state state;
		std::tie(base, rest, state) = locate(docfs, doc_path);
		if (state == fs::st_dir) {
			if (!rest.empty()){
				response_error(fout, 404, path);
				continue;
			}
			auto children = docfs.children(base);
			fout << "Cache-Control: public, max-age=86400\r\n"
				"Content-type: text/html\r\n\r\n";
			std::stringstream sstr;
			for (auto & n : children){
				auto st = docfs.state(base / n.path);
				bool is_zip = st.node.path.ext() == zip_ext;
				response_dir(sstr, doc_path / st.node.path, (is_zip? fs::st_dir : st.state), st.size);
			}
			write_html(fout, sstr.str());
			continue;
		}

		// regular file
		if (!(base.ext() == zip_ext)){
			if (rest.empty()){
				auto file = docfs.create<io::read_map>(base, io::of_open);
				if (!file)
					response_error(fout, 404, path);
				write_content_type(fout, string(base.ext().str()).c_str());
				response_file(fout, file);
				continue;
			}
			response_error(fout, 404, path);
			continue;
		}

		// for zip file;
		auto info = cache.get_info(base);
		if (!info){
			response_error(fout, 404, path);
			continue;
		}

		auto header = info->zip.get_file(rest);
		if (!header || header->uncompressed_size == 0){
			auto items = info->zip.items(rest);
			fout << "Cache-Control: public, max-age=7776000\r\n"
				"Content-type: text/html\r\n\r\n";

			std::stringstream sstr;
			for (auto &i : items){
				response_dir(sstr, doc_path/i.name, ((i.external_attrs & 0x10) ? fs::st_dir : fs::st_regular), i.uncompressed_size);
			}
			write_html(fout, sstr.str());
			continue;
		}
		if (header->method == xirang::zip::cm_deflate){
			fout << "Content-Encoding: deflate\r\n";
		}

		auto ar = open_raw(*header);
		write_content_type(fout, string(header->name.ext().str()).c_str());
		response_file(fout, ar);
	}
	return 0;
}
