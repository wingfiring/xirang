#include <xirang/path.h>
#include <algorithm>
#include <vector>
#include <xirang/string_algo/string.h>


namespace xirang{
    namespace{
        const auto onedot = literal(".");
        const auto twodot = literal("..");
    }

	static void check_utf8_(const string& str) {
		utf8::decode_string(str);
	}
	const char sub_file_path::dim = '/';

	sub_file_path::sub_file_path(){}

	sub_file_path::sub_file_path(const_range_string str)
		: m_str(str)
	{ }
	sub_file_path::sub_file_path(string::const_iterator first,
			string::const_iterator last)
		: m_str(first, last)
	{}

	sub_file_path& sub_file_path::operator=(const sub_file_path& rhs){
		m_str = rhs.m_str;
		return *this;
	}
	void sub_file_path::swap(sub_file_path& rhs){
		std::swap(m_str, rhs.m_str);
	}

	sub_file_path sub_file_path::parent() const{
		auto pos = rfind(m_str, dim);
		if ((pos ==  m_str.begin() && is_absolute())
			|| (is_network() && pos == m_str.begin() + 1)){
			return *begin();
		}
		if (pos == m_str.end()) pos = m_str.begin();
		return sub_file_path(m_str.begin(), pos);
	}
	sub_file_path sub_file_path::ext() const{
		auto pos = rfind(m_str, '.');
		if (pos != m_str.end()) ++pos;
		return sub_file_path(pos, m_str.end());
	}
	sub_file_path sub_file_path::stem() const{
		auto pos1 = rfind(m_str, dim);
		if (pos1 != m_str.end()) ++pos1;
		auto pos2 = rfind(pos1, m_str.end(), '.');
		return sub_file_path(pos1, pos2);
	}
	sub_file_path sub_file_path::filename() const{
		auto pos = rfind(m_str, dim);
		if (pos != m_str.end()) ++pos;
		return sub_file_path(pos, m_str.end());
	}

	bool sub_file_path::is_absolute() const{
		return !m_str.empty() && m_str[0] == dim;
	}
	bool sub_file_path::is_network() const{
		return m_str.size() > 2
			&& m_str[0] == dim 
			&& m_str[1] == dim
			&& m_str[2] != dim;
	}
	bool sub_file_path::is_root() const{
		return m_str.size() == 1 && m_str[0] == dim;
	}
	bool sub_file_path::has_disk() const{
		return m_str.size() > 2
			&& m_str[2] == ':';
	}
	bool sub_file_path::is_pure_disk() const{
		return m_str.size() == 3
			&& m_str[2] == ':';
	}
	bool sub_file_path::is_normalized() const{
		return std::none_of(begin(), end(), [](const sub_file_path& p){ return p.str().empty() || p.str() == onedot || p.str() == twodot;});
	}
	bool sub_file_path::empty() const{
		return m_str.empty();
	}
	bool sub_file_path::under(sub_file_path path) const{
		if( m_str.size() <= path.size())
			return false;
		auto pos = std::mismatch(m_str.begin(), m_str.end(), path.str().begin());
		if (pos.second != path.str().end())
			return false;
		return *pos.first == dim;
		
	}
	bool sub_file_path::contains(sub_file_path path) const{
		return path.under(*this);
	}

	const_range_string sub_file_path::str() const{
		return m_str;
	}
	string sub_file_path::native_str() const{
#ifdef WIN_OS_
		auto ret = replace(m_str, dim, '\\');
		if (is_absolute() && !is_network()){
			return const_range_string(++ret.begin(), ret.end());
		}
		return std::move(ret);
#else
		return str();
#endif
	}
	wstring sub_file_path::native_wstr() const{
		return utf8::decode_string(native_str());
	}
	wstring sub_file_path::wstr() const{
		return utf8::decode_string(m_str);
	}
	sub_file_path::iterator sub_file_path::begin() const{
		return sub_file_path::iterator(m_str, m_str.begin());
	}
	sub_file_path::iterator sub_file_path::end() const{
		return sub_file_path::iterator(m_str, m_str.end());
	}


	sub_file_path::iterator::iterator()
		: m_pos(), m_path()
	{}
	sub_file_path::iterator::iterator(const_range_string spath, string::const_iterator pos)
		: m_pos(pos), m_path(spath)
	{}

	void sub_file_path::iterator::swap(sub_file_path::iterator& rhs){
		std::swap(m_pos, rhs.m_pos);
		std::swap(m_path, rhs.m_path);
	}

	sub_file_path::iterator& sub_file_path::iterator::operator++(){
		AIO_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		AIO_PRE_CONDITION(m_pos != end_() &&"increase end iterator");

		if (m_pos == begin_()){
			if(path_().is_network()){
				m_pos += 2;
			}
			else if(path_().is_absolute()){
				++m_pos;
				return *this;
			}
		}
		m_pos = find(m_pos, end_(), sub_file_path::dim);
		if (m_pos != end_()) ++m_pos;
		return *this;
	}
	sub_file_path::iterator& sub_file_path::iterator::operator--(){
		AIO_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		AIO_PRE_CONDITION(m_pos != begin_() &&"decrease begin iterator");

		if (m_pos == begin_() + 1 && path_().is_absolute()){
			--m_pos;
			return *this;
		}

		if (m_pos != begin_() && m_pos != end_()){
			AIO_PRE_CONDITION(*(m_pos -1) == sub_file_path::dim);
			--m_pos;
		}
		auto pos = rfind(begin_(), m_pos, sub_file_path::dim);
		m_pos = (pos == m_pos) ? begin_() : pos + 1;

		if (path_().is_network() && m_pos == begin_() + 1)
			m_pos = begin_();
		
		return *this;
	}
	sub_file_path::iterator sub_file_path::iterator::operator++(int){
		auto tmp = *this;
		++*this;
		return tmp;
	}
	sub_file_path::iterator sub_file_path::iterator::operator--(int){
		auto tmp = *this;
		--*this;
		return tmp;
	}

	bool sub_file_path::iterator::operator==(const sub_file_path::iterator& rhs){
		AIO_PRE_CONDITION(m_path == rhs.m_path);

		return m_pos == rhs.m_pos;
	}
	bool sub_file_path::iterator::operator!=(const sub_file_path::iterator& rhs){
		return !(*this == rhs);
	}

	sub_file_path::iterator::reference sub_file_path::iterator::operator*() const{
		AIO_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		AIO_PRE_CONDITION(m_pos != end_() &&"dereference end iterator");
		auto pos = m_pos;
		if (m_pos == begin_()){
			if(path_().is_network())
				pos = find(m_pos + 2, end_(), sub_file_path::dim);
			else if (path_().is_absolute())
				++pos;
			else
				pos = find(m_pos, end_(), sub_file_path::dim);
		}
		else
			pos = find(m_pos, end_(), sub_file_path::dim);

		m_cache = sub_file_path(m_pos, pos);
		return m_cache;
	}
	sub_file_path::iterator::pointer sub_file_path::iterator::operator->() const{
		return &**this;
	}
	sub_file_path sub_file_path::iterator::path_() const{
		return sub_file_path(begin_(), end_());
	}
	string::const_iterator sub_file_path::iterator::begin_() const{
		return m_path.begin();
	}
	string::const_iterator sub_file_path::iterator::end_() const{
		return m_path.end();
	}

	/////////////////////////////////// 
	// file_path

	file_path::file_path() {}
	file_path::file_path(const sub_file_path& spath) 
		: m_str(spath.str())
	{}

	file_path::file_path(const string& str, path_process pp /* = pp_default*/)
		: m_str(str)
	{
		if (pp & pp_utf8check)
			check_utf8_(str);
		if (pp & pp_normalize)
			normalize(pp);
	}
	file_path::file_path(const wstring& str, path_process pp /* = pp_default */)
		: m_str(utf8::encode_string(str))
	{
		if (pp & pp_normalize)
			normalize(pp);
	}
	file_path::file_path(const file_path& rhs) : m_str(rhs.m_str){}
	file_path::file_path(file_path&& rhs) : m_str(std::move(rhs.m_str)){}

	file_path& file_path::operator=(const file_path& rhs){
		file_path(rhs).swap(*this);
		return *this;
	}
	file_path& file_path::operator=(file_path&& rhs){
		file_path(std::move(rhs)).swap(*this);
		return *this;
	}


	sub_file_path file_path::parent() const{
		return as_sub_path().parent(); 
	}
	sub_file_path file_path::ext() const{
		return as_sub_path().ext();
	}
	sub_file_path file_path::stem() const{
		return as_sub_path().stem();
	}
	sub_file_path file_path::filename() const{
		return as_sub_path().filename();
	}
	bool file_path::is_absolute() const{
		return as_sub_path().is_absolute();
	}
	bool file_path::is_network() const{
		return as_sub_path().is_network();
	}
	bool file_path::is_root() const{
		return as_sub_path().is_root();
	}
	bool file_path::has_disk() const{
		return as_sub_path().has_disk();
	}
	bool file_path::is_pure_disk() const{
		return as_sub_path().is_pure_disk();
	}

	bool file_path::is_normalized() const{
		return as_sub_path().is_normalized();
	}
	bool file_path::empty() const{
		return m_str.empty();
	}
	bool file_path::under(sub_file_path path) const{
		return as_sub_path().under(path);
	}
	bool file_path::contains(sub_file_path path) const{
		return as_sub_path().contains(path);
	}

    file_path& file_path::normalize(path_process pp)
	{
		std::vector<sub_file_path> stack;
		for (auto p : *this){
			if (p.str() == onedot || p.str().empty())
				continue;
			if (p.str() == twodot)
			{
				if (!stack.empty() && (!is_absolute() || stack.size() > 1)) stack.pop_back();
				continue;
			}
			stack.push_back(p);
		}

		string_builder result;
		for (auto& p : stack)
		{
			result += p.str();
			if (!p.is_root())
				result.push_back(sub_file_path::dim);
		}
		if (result.size() > 1)
			result.resize(result.size() - 1);
		if ((pp & pp_winfile) && result.size() > 1 && result[0] != sub_file_path::dim && result[1] == ':')
			result.insert(result.begin(), 1, sub_file_path::dim);
		m_str = result;
		return *this;
	}


	file_path& file_path::operator/=(const sub_file_path& rhs){
		if (rhs.is_root())
			return *this;

		if (is_root())
		{
			if (rhs.is_absolute())
				m_str = rhs.str();
			else
				m_str = literal("/") << rhs.str();
		}
		else if (rhs.is_network()){
			m_str = m_str << rhs.str();
			normalize();
		}
		else if (rhs.is_absolute())
			m_str = m_str << rhs.str();
		else
			m_str = m_str << literal("/") << rhs.str();
		return *this;
	}

	file_path& file_path::replace_parent(const file_path& rhs){
		*this = rhs / filename();
		return *this;
	}
	file_path& file_path::replace_ext(const file_path& rhs){
		auto pos = rfind(m_str.begin(), m_str.end(), '.');
		if (!rhs.m_str.empty() && rhs.m_str[0] == '.')
			m_str = const_range_string(m_str.begin(), pos) << rhs.m_str;
		else 
			m_str = const_range_string(m_str.begin(), pos) << literal(".") << rhs.m_str;

		return *this;
	}
	file_path& file_path::replace_stem(const file_path& rhs){
		auto extension = ext().str();
		if (extension.empty())
			*this = parent() / rhs;
		else
			*this = parent() / file_path(string(rhs.str() << literal(".") <<  extension), pp_none);

		return *this;
	}
	file_path& file_path::replace_filename(const file_path& rhs){
		*this = parent() / rhs;
		return *this;
	}
	file_path& file_path::to_absolute(){
		if (!is_absolute())
			m_str = literal("/") << m_str;
		return *this;
	}
	file_path& file_path::remove_absolute(){
		if (is_network())
			m_str = const_range_string(m_str.begin() + 2, m_str.end());
		else if(is_absolute()){
			m_str = const_range_string(m_str.begin() + 1, m_str.end());
		}
		return *this;
	}

	void file_path::swap(file_path& rhs){
		m_str.swap(rhs.m_str);
	}

	const string& file_path::str() const{
		return m_str;
	}

	string file_path::native_str() const{
		return as_sub_path().native_str();
	}
	wstring file_path::native_wstr() const{
		return as_sub_path().native_wstr();
	}
	wstring file_path::wstr() const{
		return as_sub_path().wstr();
	}

	file_path::operator sub_file_path() const{
		return as_sub_path();
	}
	sub_file_path file_path::as_sub_path() const{
		return sub_file_path(m_str.begin(), m_str.end());
	}
	file_path::iterator file_path::begin() const{
		return as_sub_path().begin();
	}
	file_path::iterator file_path::end() const{
		return as_sub_path().end();
	}

	file_path operator/(const sub_file_path& lhs, const sub_file_path& rhs){
		file_path ret = lhs;
		ret /= rhs;
		return std::move(ret);
	}

	////////////////////////////////
	//sub_simple_path

	const char sub_simple_path::dim = '.';
	sub_simple_path::sub_simple_path(){}

	sub_simple_path::sub_simple_path(const_range_string str)
		: m_str(str)
	{ }
	sub_simple_path::sub_simple_path(string::const_iterator first,
			string::const_iterator last)
		: m_str(first, last)
	{}

	sub_simple_path& sub_simple_path::operator=(const sub_simple_path& rhs){
		m_str = rhs.m_str;
		return *this;
	}
	void sub_simple_path::swap(sub_simple_path& rhs){
		std::swap(m_str, rhs.m_str);
	}

	sub_simple_path sub_simple_path::parent() const{
		auto pos = rfind(m_str, dim);
		if (pos ==  m_str.begin() && is_absolute()){
			return *begin();
		}
		if (pos == m_str.end()) pos = m_str.begin();
		return sub_simple_path(m_str.begin(), pos);
	}
	sub_simple_path sub_simple_path::filename() const{
		auto pos = rfind(m_str, dim);
		if (pos != m_str.end()) ++pos;
		return sub_simple_path(pos, m_str.end());
	}

	bool sub_simple_path::is_absolute() const{
		return !m_str.empty() && m_str[0] == dim;
	}
	bool sub_simple_path::is_root() const{
		return m_str.size() == 1 && m_str[0] == dim;
	}
	bool sub_simple_path::is_normalized() const{
		return std::none_of(begin(), end(), [](const sub_simple_path& p){ return p.str().empty();});
	}
	bool sub_simple_path::empty() const{
		return m_str.empty();
	}

	bool sub_simple_path::under(sub_simple_path path) const{
		if( m_str.size() <= path.size())
			return false;
		auto pos = std::mismatch(m_str.begin(), m_str.end(), path.str().begin());
		if (pos.second != path.str().end())
			return false;
		return *pos.first == dim;
		
	}
	bool sub_simple_path::contains(sub_simple_path path) const{
		return path.under(*this);
	}

	const_range_string sub_simple_path::str() const{
		return m_str;
	}
	sub_simple_path::iterator sub_simple_path::begin() const{
		return sub_simple_path::iterator(m_str, m_str.begin());
	}
	sub_simple_path::iterator sub_simple_path::end() const{
		return sub_simple_path::iterator(m_str, m_str.end());
	}

	sub_simple_path::iterator::iterator()
		: m_pos(), m_path()
	{}
	sub_simple_path::iterator::iterator(const_range_string spath, string::const_iterator pos)
		: m_pos(pos), m_path(spath)
	{}

	void sub_simple_path::iterator::swap(sub_simple_path::iterator& rhs){
		std::swap(m_pos, rhs.m_pos);
		std::swap(m_path, rhs.m_path);
	}

	sub_simple_path::iterator& sub_simple_path::iterator::operator++(){
		AIO_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		AIO_PRE_CONDITION(m_pos != end_() &&"increase end iterator");

		if (m_pos == begin_() &&path_().is_absolute()){
			++m_pos;
			return *this;
		}
		m_pos = find(m_pos, end_(), sub_simple_path::dim);
		if (m_pos != end_()) ++m_pos;
		return *this;
	}
	sub_simple_path::iterator& sub_simple_path::iterator::operator--(){
		AIO_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		AIO_PRE_CONDITION(m_pos != begin_() &&"decrease begin iterator");

		if (m_pos == begin_() + 1 && path_().is_absolute()){
			--m_pos;
			return *this;
		}

		if (m_pos != begin_() && m_pos != end_()){
			AIO_PRE_CONDITION(*(m_pos -1) == sub_simple_path::dim);
			--m_pos;
		}
		auto pos = rfind(begin_(), m_pos, sub_simple_path::dim);
		m_pos = (pos == m_pos) ? begin_() : pos + 1;

		return *this;
	}
	sub_simple_path::iterator sub_simple_path::iterator::operator++(int){
		auto tmp = *this;
		++*this;
		return tmp;
	}
	sub_simple_path::iterator sub_simple_path::iterator::operator--(int){
		auto tmp = *this;
		--*this;
		return tmp;
	}

	bool sub_simple_path::iterator::operator==(const sub_simple_path::iterator& rhs){
		AIO_PRE_CONDITION(m_path == rhs.m_path);

		return m_pos == rhs.m_pos;
	}
	bool sub_simple_path::iterator::operator!=(const sub_simple_path::iterator& rhs){
		return !(*this == rhs);
	}

	sub_simple_path::iterator::reference sub_simple_path::iterator::operator*() const{
		AIO_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		AIO_PRE_CONDITION(m_pos != end_() &&"dereference end iterator");
		auto pos = m_pos;
		if (m_pos == begin_()){
			if (path_().is_absolute())
				++pos;
			else
				pos = find(m_pos, end_(), sub_simple_path::dim);
		}
		else
			pos = find(m_pos, end_(), sub_simple_path::dim);

		m_cache = sub_simple_path(m_pos, pos);
		return m_cache;
	}
	sub_simple_path::iterator::pointer sub_simple_path::iterator::operator->() const{
		return &**this;
	}
	sub_simple_path sub_simple_path::iterator::path_() const{
		return sub_simple_path(begin_(), end_());
	}
	string::const_iterator sub_simple_path::iterator::begin_() const{
		return m_path.begin();
	}
	string::const_iterator sub_simple_path::iterator::end_() const{
		return m_path.end();
	}

	simple_path::simple_path() {}

	simple_path::simple_path(const string& str, path_process pp /* = pp_default*/)
		: m_str(str)
	{
		if (pp & pp_utf8check)
			check_utf8_(str);
		if (pp & pp_normalize)
			normalize(pp);
	}
	simple_path::simple_path(const simple_path& rhs) : m_str(rhs.m_str){}
	simple_path::simple_path(sub_simple_path rhs) : m_str(rhs.str()){}
	simple_path::simple_path(simple_path&& rhs) : m_str(std::move(rhs.m_str)){}

	simple_path& simple_path::operator=(const simple_path& rhs){
		simple_path(rhs).swap(*this);
		return *this;
	}
	simple_path& simple_path::operator=(simple_path&& rhs){
		simple_path(std::move(rhs)).swap(*this);
		return *this;
	}
	simple_path& simple_path::operator=(sub_simple_path rhs){
		m_str = rhs.str();
		return *this;
	}
	simple_path::operator sub_simple_path() const{
		return as_sub_path();
	}
	sub_simple_path simple_path::as_sub_path() const{
		return sub_simple_path(m_str.begin(), m_str.end());
	}

	sub_simple_path simple_path::parent() const{
		return as_sub_path().parent();
	}
	sub_simple_path simple_path::filename() const{
		return as_sub_path().filename();
	}
	bool simple_path::is_absolute() const{
		return as_sub_path().is_absolute();
	}
	bool simple_path::is_root() const{
		return as_sub_path().is_root();
	}
	bool simple_path::is_normalized() const{
		return as_sub_path().is_normalized();
	}

	bool simple_path::empty() const{
		return as_sub_path().empty();
	}
	bool simple_path::under(sub_simple_path path) const{
		return as_sub_path().under(path);
	}
	bool simple_path::contains(sub_simple_path path) const{
		return as_sub_path().contains(path);
	}
    simple_path& simple_path::normalize(path_process pp)
	{
		std::vector<sub_simple_path> stack;
		for (auto p : *this){
			if (p.str().empty())
				continue;
			stack.push_back(p);
		}

		string_builder result;
		for (auto& p : stack)
		{
			result += p.str();
			if (!p.is_root())
				result.push_back(sub_simple_path::dim);
		}
		if (result.size() > 1)
			result.resize(result.size() - 1);
		m_str = result;
		return *this;
	}

	simple_path& simple_path::operator/=(const sub_simple_path& rhs){
		if (rhs.is_root())
			return *this;

		if (is_root())
		{
			if (rhs.is_absolute())
				m_str = rhs.str();
			else
				m_str = literal(".") << rhs.str();
		}
		else if (rhs.is_absolute())
			m_str = m_str << rhs.str();
		else
			m_str = m_str << literal(".") << rhs.str();
		return *this;
	}

	simple_path& simple_path::replace_parent(const simple_path& rhs){
		*this = rhs / filename();
		return *this;
	}
	simple_path& simple_path::replace_filename(const simple_path& rhs){
		*this = parent() / rhs;
		return *this;
	}
	simple_path& simple_path::to_absolute(){
		if (!is_absolute())
			m_str = literal(".") << m_str;
		return *this;
	}
	simple_path& simple_path::remove_absolute(){
		if(is_absolute()){
			m_str = const_range_string(m_str.begin() + 1, m_str.end());
		}
		return *this;
	}

	void simple_path::swap(simple_path& rhs){
		m_str.swap(rhs.m_str);
	}

	const string& simple_path::str() const{
		return m_str;
	}

	simple_path::iterator simple_path::begin() const{
		return as_sub_path().begin();
	}
	simple_path::iterator simple_path::end() const{
		return as_sub_path().end();
	}

	simple_path operator/(const sub_simple_path& lhs, const sub_simple_path& rhs){
		simple_path ret = lhs;
		ret /= rhs;
		return std::move(ret);
	}

}

