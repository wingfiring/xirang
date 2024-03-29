#include <xirang/zip.h>
#include <xirang/path.h>
#include <xirang/io/file.h>
#include <xirang/vfs/local.h>
#include <xirang/string_algo/string.h>

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

void write_content_type(std::ostream& fout, const std::string& ext_){
	std::string ext = ext_;
	std::transform(ext.begin(), ext.end(), ext.begin(), xirang::tolower<char> );
	auto pos = mime.find(ext);
	if (pos != mime.end()){
		fout << "Content-type: "
			<< pos->second;
		fout << "\r\n";
	}
}

void response_file(std::ostream& fout, iref<io::read_map>& file){
	auto & rmap = file.get<xirang::io::read_map>();
	auto view = rmap.view_rd(xirang::ext_heap::handle(0, rmap.size()));
	auto address = view.get<xirang::io::read_view>().address();

	fout << "Cache-Control: public, max-age=7776000\r\n\r\n";
	fout.write((const char*)address.begin(), address.size());
}
void response_dir(std::ostream& fout, const file_path& path, int state, long_size_t size){
	std::string type = (state == fs::st_dir ? "DIR" : "FILE");
	std::string type_str = (state == fs::st_dir ? "DIR " : "FILE");

	fout << "<div class=\"" << type << "\"><span class=\"file_type\">" << type_str << "</span>"
		"<span class=\"file_size\">" << size << "</span>"
		"<span class=\"file_name\"> <a href=\"" << path.filename().str();
	if (state == fs::st_dir) fout << "/";
	fout << "\">" << path.filename().str() << "</a></span></div>";
}
void response_error(std::ostream& os, int code, const std::string& path){
	os << "Status: " << code << "\r\n"
		"Content-type: text/html\r\n"
		"\r\n"
		"Request file: "
		<< path
		<< " not found\r\n";
}
bool end_with_slash(const std::string& path){
	return !path.empty() && path[path.size() - 1] == '/';
}
void response_redirect(std::ostream& fout, const file_path& dir_name, bool is_dir){
	fout << "Status: 301\r\n"
		"Location:" << dir_name.str();
	if (is_dir) fout << "/";
	fout << "\r\n"
		"Content-type: text/html\r\n"
		"\r\n";
}

const sub_file_path zip_ext = sub_file_path(literal("zip"));

const file_path index_pages[] = {file_path("index.html"), file_path("index.htm")};

void do_main(FCGX_Request& request, vfs::LocalFs& docfs, cache_manager& cache){
	fcgi_streambuf in_buf(request.in);
	fcgi_streambuf out_buf(request.out);
	std::istream fin(&in_buf);
	std::ostream fout(&out_buf);

	auto s = FCGX_GetParam("PATH_INFO", request.envp);
	if (!s){
		response_error(fout, 404, std::string());
		return;
	}

	std::string path = s ? s : "";
	file_path doc_path = file_path(path.c_str());
	file_path base, rest;
	fs::file_state state;
	std::tie(base, rest, state) = locate(docfs, doc_path);
	if (state == fs::st_dir) {
		if (!rest.empty()){
			response_error(fout, 404, path);
			return;
		}
		if (!path.empty() && !end_with_slash(path)){
			response_redirect(fout, base.filename(), true);
			return;
		}

		file_path index_page;
		for (auto& f : index_pages){
			auto st = docfs.state(base / f).state;
			if (st == fs::st_regular){
				index_page = f;
				break;
			}
		}
		if (!index_page.empty()){
			response_redirect(fout, index_page, false);
			return;
		}
		// FIXME: children() should return a foward  range (multi-pass iteration), but the imp of boost file iterator is a single pass iterator
		// so some behavior of the range is incorrect. for example, if the size() method is called, then the internal iterator will be end, and 
		// user can't iterate the range again.
		auto children = docfs.children(base);
		std::vector<vfs::VfsNode> files;
		for(auto n : children){
			files.push_back(n);
		}

		std::sort(files.begin(), files.end(), [](const vfs::VfsNode & lhs, const vfs::VfsNode& rhs){
				return lhs.path.str() < rhs.path.str();
				});

		fout << "Cache-Control: public, max-age=86400\r\n"
			"Content-type: text/html\r\n\r\n";
		std::stringstream sstr;
		for (auto & n : files){
			auto st = docfs.state(base / n.path);
			bool is_zip = st.node.path.ext() == zip_ext;
			response_dir(sstr, st.node.path, (is_zip? fs::st_dir : st.state), st.size);
		}
		write_html(fout, sstr.str());
		return;
	}

	// regular file
	if (!(base.ext() == zip_ext)){
		if (rest.empty()){
			auto file = docfs.create<io::read_map>(base, io::of_open);
			if (!file)
				response_error(fout, 404, path);
			write_content_type(fout, string(base.ext().str()).c_str());
			response_file(fout, file);
			return;
		}
		response_error(fout, 404, path);
		return;
	}

	// for zip file;
	auto info = cache.get_info(base);
	if (!info){
		response_error(fout, 404, path);
		return;
	}

	auto header = info->zip.get_file(rest);
	if (!header || (header->external_attrs & 0x10) != 0){
		auto items = info->zip.items(rest);
		if (!header && items.empty()){
			response_error(fout, 404, path);
			return;
		}

		if (!end_with_slash(path)){
			response_redirect(fout, rest.filename(), true);
			return;
		}

		file_path index_page;
		for (auto& f : index_pages){
			auto h = info->zip.get_file(rest/f);
			if (h && (h->external_attrs & 0x10) == 0){
				index_page = f;
				break;
			}
		}
		if (!index_page.empty()){
			response_redirect(fout, index_page, false);
			return;
		}

		fout << "Cache-Control: public, max-age=7776000\r\n"
			"Content-type: text/html\r\n\r\n";

		std::stringstream sstr;
		for (auto &i : items){
			response_dir(sstr, i.name, ((i.external_attrs & 0x10) ? fs::st_dir : fs::st_regular), i.uncompressed_size);
		}
		write_html(fout, sstr.str());
		return;
	}
	if (header->method == xirang::zip::cm_deflate){
		fout << "Content-Encoding: deflate\r\n";
	}

	auto ar = open_raw(*header);
	write_content_type(fout, string(header->name.ext().str()).c_str());
	response_file(fout, ar);

}
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

	std::string err_msg;
	while (FCGX_Accept_r (&request) == 0){
		try{
			do_main(request, docfs, cache);
			continue;
		}catch(xirang::exception& e){
			err_msg = e.what();
		}
		catch(...){
			std::ofstream ofs("/tmp/zipdoc.log", std::ios_base::app);
			err_msg = "unknown error.";
		}
		std::ofstream ofs("/tmp/zipdoc.log", std::ios_base::app);
		ofs << err_msg << std::endl;

		fcgi_streambuf out_buf(request.out);
		std::ostream fout(&out_buf);
		response_error(fout, 404, "");
	}
	return 0;
}
