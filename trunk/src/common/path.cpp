#include <aio/common/path.h>
#include <algorithm>
#include <vector>


namespace aio{
    namespace{
        const auto onedot = literal(".");
        const auto twodot = literal("..");
    }

	static void check_utf8_(const string& str) {
		utf8::decode_string(str);
	}

	path::path() {}

	path::path(const string& str, path_process pp /* = pp_default*/)
		: m_str(str)
	{
		if (pp & pp_utf8check)
			check_utf8_(str);
		if (pp & pp_normalize)
			normalize();
	}
	path::path(const wstring& str, path_process pp /* = pp_default */)
		: m_str(utf8::encode_string(str))
	{
		if (pp & pp_normalize)
			normalize();
	}
	path::path(const path& rhs) : m_str(rhs.m_str){}
	path::path(path&& rhs) : m_str(std::move(rhs.m_str)){}

	path& path::operator=(const path& rhs){
		path(rhs).swap(*this);
		return *this;
	}
	path& path::operator=(path&& rhs){
		path(std::move(rhs)).swap(*this);
		return *this;
	}


	path path::parent() const{
		auto pos = rfind(m_str, '/');
		if ((pos ==  m_str.begin() && is_absolute())
			|| (is_network() && pos == m_str.begin() + 1)){
			return *begin();
		}
		if (pos == m_str.end()) pos = m_str.begin();
		return path(const_range_string(m_str.begin(), pos), pp_none);
	}
	path path::ext() const{
		auto pos = rfind(m_str.begin(), m_str.end(), '.');
		if (pos != m_str.end()) ++pos;
		return path(const_range_string(pos, m_str.end()), pp_none);
	}
	path path::stem() const{
		auto pos1 = rfind(m_str, '/');
		if (pos1 != m_str.end()) ++pos1;
		auto pos2 = rfind(pos1, m_str.end(), '.');
		return path(const_range_string(pos1, pos2), pp_none);
	}
	path path::filename() const{
		auto pos = rfind(m_str, '/');
		if (pos != m_str.end()) ++pos;
		return path(const_range_string(pos, m_str.end()), pp_none);
	}
	bool path::is_absolute() const{
		return !m_str.empty() && m_str[0] == '/';
	}
	bool path::is_network() const{
		return m_str.size() > 2
			&& m_str[0] == '/' 
			&& m_str[1] == '/'
			&& m_str[2] != '/';
	}
	bool path::is_root() const{
		return m_str.size() == 1 && m_str[0] == '/';
	}

	bool path::is_normalized() const{
		return std::none_of(begin(), end(), [](const path& p){ return p.str().empty() || p.str() == onedot || p.str() == twodot;});
	}
	bool path::empty() const{
		return m_str.empty();
	}

    path& path::normalize()
	{
		std::vector<path> stack;
		for (auto p : *this){
			if (p.str() == onedot || p.str().empty())
				continue;
			if (p.str() == twodot)
			{
				if (!stack.empty() && (!is_absolute() || stack.size() > 1)) stack.pop_back();
				continue;
			}
			if (!p.str().empty())
				stack.push_back(p);
		}

		string_builder result;
		for (auto& p : stack)
		{
			result += p.str();
			if (!p.is_root())
				result.push_back('/');
		}
		if (result.size() > 1)
			result.resize(result.size() - 1);

		m_str = result;
		return *this;
	}


	path& path::operator/=(const path& rhs){
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

	path& path::replace_parent(const path& rhs){
		*this = rhs / filename();
		return *this;
	}
	path& path::replace_ext(const path& rhs){
		auto pos = rfind(m_str.begin(), m_str.end(), '.');
		if (!rhs.m_str.empty() && rhs.m_str[0] == '.')
			m_str = const_range_string(m_str.begin(), pos) << rhs.m_str;
		else 
			m_str = const_range_string(m_str.begin(), pos) << literal(".") << rhs.m_str;

		return *this;
	}
	path& path::replace_stem(const path& rhs){
		auto extension = ext().str();
		if (extension.empty())
			*this = parent() / rhs;
		else
			*this = parent() / path(string(rhs.str() << literal(".") <<  extension), pp_none);

		return *this;
	}
	path& path::replace_filename(const path& rhs){
		*this = parent() / rhs;
		return *this;
	}
	path& path::to_absolute(){
		if (!is_absolute())
			m_str = literal("/") << m_str;
		return *this;
	}
	path& path::remove_absolute(){
		if (is_network())
			m_str = const_range_string(m_str.begin() + 2, m_str.end());
		else if(is_absolute())
			m_str = const_range_string(m_str.begin() + 1, m_str.end());
		return *this;
	}

	void path::swap(path& rhs){
		m_str.swap(rhs.m_str);
	}

	const aio::string& path::str() const{
		return m_str;
	}

	aio::string path::native_str() const{
#ifdef WIN_OS_
		auto ret = replace(m_str, '/', '\\');
		if (is_absolute() && !is_network()){
			return const_range_string(++ret.begin(), ret.end());
		}
		return std::move(ret);
#else
		return str();
#endif
	}
	aio::wstring path::native_wstr() const{
		return utf8::decode_string(native_str());
	}
	aio::wstring path::wstr() const{
		return utf8::decode_string(m_str);
	}

	path::iterator path::begin() const{
		return path::iterator(*this, str().begin());
	}
	path::iterator path::end() const{
		return path::iterator(*this, str().end());
	}

	path operator/(const path& lhs, const path& rhs){
		path ret = lhs;
		ret /= rhs;
		return std::move(ret);
	}

	path::iterator::iterator()
		: m_pos(), m_path()
	{}
	path::iterator::iterator(const path& path, string::const_iterator pos)
		: m_pos(pos), m_path(&path)
	{}

	void path::iterator::swap(path::iterator& rhs){
		std::swap(m_pos, rhs.m_pos);
		std::swap(m_path, rhs.m_path);
	}

	path::iterator& path::iterator::operator++(){
		AIO_PRE_CONDITION(m_path && m_pos && "empty iterator");
		AIO_PRE_CONDITION(m_pos != end_() &&"increase end iterator");

		if (m_pos == begin_()){
			if(m_path->is_network()){
				m_pos += 2;
			}
			else if(m_path->is_absolute()){
				++m_pos;
				return *this;
			}
		}
		m_pos = find(m_pos, end_(), '/');
		if (m_pos != end_()) ++m_pos;
		return *this;
	}
	path::iterator& path::iterator::operator--(){
		AIO_PRE_CONDITION(m_path && m_pos && "empty iterator");
		AIO_PRE_CONDITION(m_pos != begin_() &&"decrease begin iterator");

		if (m_pos == begin_() + 1 && m_path->is_absolute()){
			--m_pos;
			return *this;
		}

		if (m_pos != begin_() && m_pos != end_()){
			AIO_PRE_CONDITION(*(m_pos -1) == '/');
			--m_pos;
		}
		auto pos = rfind(begin_(), m_pos, '/');
		m_pos = (pos == m_pos) ? begin_() : pos + 1;

		if (m_path->is_network() && m_pos == begin_() + 1)
			m_pos = begin_();
		
		return *this;
	}
	path::iterator path::iterator::operator++(int){
		auto tmp = *this;
		++*this;
		return tmp;
	}
	path::iterator path::iterator::operator--(int){
		auto tmp = *this;
		--*this;
		return tmp;
	}

	bool path::iterator::operator==(const path::iterator& rhs){
		return m_path == rhs.m_path
			&& m_pos == rhs.m_pos;
	}
	bool path::iterator::operator!=(const path::iterator& rhs){
		return !(*this == rhs);
	}

	path::iterator::reference path::iterator::operator*() const{
		AIO_PRE_CONDITION(m_path && m_pos && "empty iterator");
		AIO_PRE_CONDITION(m_pos != m_path->str().end() &&"dereference end iterator");
		auto pos = m_pos;
		if (m_pos == begin_()){
			if(m_path->is_network())
				pos = find(m_pos + 2, end_(), '/');
			else if (m_path->is_absolute())
				++pos;
			else
				pos = find(m_pos, m_path->str().end(), '/');
		}
		else
			pos = find(m_pos, m_path->str().end(), '/');

		m_cache = path(const_range_string(m_pos, pos), pp_none);
		return m_cache;
	}
	path::iterator::pointer path::iterator::operator->() const{
		return &**this;
	}
	string::const_iterator path::iterator::begin_() const{
		return m_path->str().begin();
	}
	string::const_iterator path::iterator::end_() const{
		return m_path->str().end();
	}
}

