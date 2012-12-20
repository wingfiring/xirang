#ifndef SRC_XIRANG_VFS_FILE_TREE_H
#define SRC_XIRANG_VFS_FILE_TREE_H

#include <aio/xirang/vfs.h>
#include <aio/common/char_separator.h>
#include <aio/common/string_algo/string.h>

#include <boost/tokenizer.hpp>
#include <unordered_map>
#include <algorithm>

namespace xirang{ namespace fs{ 
	AIO_EXCEPTION_TYPE(empty_local_file_name);
	AIO_EXCEPTION_TYPE(not_all_parent_are_dir);
}}

namespace xirang{ namespace fs{ namespace private_{

	struct hash_string{
		size_t operator()(const string& str) const{
			return str.hash();
		}
	};

	template<typename T> struct file_node
	{
		typedef file_node<T> node_type;

		string name;
		file_state type;	//dir or normal
		std::unordered_map<string, node_type*, hash_string> children;
		node_type* parent;

		T data;

		file_node() :
			type(aiofs::st_dir),
			parent(0),
			data()
		{}

		~file_node()
		{
			for (auto& i : children)
				delete i.second;
		}
	};

	template<typename T> struct locate_result{
		file_node<T>* node;
		aio::const_range_string not_found;
	};

	template <typename T>
	locate_result<T> locate(file_node<T>& root, const aio::const_range_string& path) 
	{
        aio::char_separator<char> sep('/');
        typedef boost::tokenizer<aio::char_separator<char>, string::const_iterator, aio::const_range_string> tokenizer;
        tokenizer tokens(path, sep);

		file_node<T>* pos = &root;
        tokenizer::iterator itr = tokens.begin();
		for (; itr != tokens.end(); ++itr)
		{
			auto child = pos->children.find(*itr);
			if (child != pos->children.end())
				pos = child->second;
			else
				break;
		}

		auto rest_first = itr == tokens.end()?  path.end() : itr->begin();

		typedef locate_result<T> return_type;
		return return_type {
			pos, aio::const_range_string(rest_first, path.end())
		};
		//return ret;
	}


	template <typename T>
	file_node<T>* locate_parent(file_node<T>& root, const string& path, string& base) 
	{
        aio::char_separator<char> sep('/');
        typedef boost::tokenizer<aio::char_separator<char>, string::const_iterator, aio::const_range_string> tokenizer;
        tokenizer tokens(path, sep);


		file_node<T>* pos = &root;
        tokenizer::iterator itr = tokens.begin();
		for (; itr != tokens.end(); ++itr)
		{
			auto child = pos->children.find(*itr);
			if (child != pos->children.end())
				pos = child->second;
			else
				break;
		}

		if (itr == tokens.end())
		{
			base = pos->name;
			return pos->parent;
		}

		aio::string_builder sbbase;
		for (; itr != tokens.end(); ++itr)
		{
			sbbase += *itr;
			sbbase.push_back('/');
		}

		if (!sbbase.empty())
			sbbase.pop_back();
		base = sbbase;

		return pos;
	}

	template<typename T>
	fs_error removeNode(file_node<T>* node)
	{
		AIO_PRE_CONDITION(node);
        if (!node->children.empty())
            return aiofs::er_not_empty;

		node->parent->children.erase(node->name);
		delete node;
		return aiofs::er_ok;
	}

	template<typename T> file_node<T>* create_node(file_node<T>& root, const string& path, file_state type, bool whole_path)
	{
		AIO_PRE_CONDITION(!path.empty());

		aio::char_separator<char> sep('/');
        typedef boost::tokenizer<aio::char_separator<char>, string::const_iterator, aio::const_range_string> tokenizer;
        tokenizer tokens(path, sep);

		file_node<T>* pos = &root;

        tokenizer::iterator name_first = tokens.begin(); 
		tokenizer::iterator name_end = tokens.end(); 

		int create_times = 0;
		for(;name_first != name_end; ++name_first)
		{
			if (pos->type != aiofs::st_dir)
				AIO_THROW(not_all_parent_are_dir);

			auto found = pos->children.find(*name_first);
			if (found != pos->children.end())
			{
				pos = found->second;
			}
			else
			{
				if (!whole_path && create_times != 0)
				{
					pos->parent->children.erase(pos->name);
					delete pos;
					return 0;
				}

				aio::unique_ptr<file_node<T> > fnode(new file_node<T> );
				fnode->name = *name_first;
				fnode->type = aiofs::st_dir;
				fnode->parent = pos;
				pos->children[*name_first] = fnode.get();
				pos = fnode.release();
				++create_times;
			}
		}
		pos->type = type;
		return pos;
	}

	template<typename T>
	file_node<T>* create_node(const locate_result<T>& pos, file_state type){
		AIO_PRE_CONDITION(pos.node);
		AIO_PRE_CONDITION(!pos.not_found.empty());
		AIO_PRE_CONDITION(!aio::contains(pos.not_found, '/'));
		AIO_PRE_CONDITION(pos.node->children.count(pos.not_found) == 0);

		aio::unique_ptr<file_node<T> > fnode(new file_node<T> );
		fnode->name = pos.not_found;
		fnode->type = type;
		fnode->parent = pos.node;
		pos.node->children[fnode->name] = fnode.get();

		return fnode.release();
	}

	template<typename T>
	class FileNodeIterator
	{
	public:
		typedef file_node<T> node_type;
		typedef typename std::unordered_map<string, node_type*, hash_string>::iterator iterator;
		FileNodeIterator()
			: m_itr() 
		{
            m_node.owner_fs = 0;
        }

		explicit FileNodeIterator(const iterator& itr, IVfs* vfs)
			: m_itr(itr)
		{ 
            m_node.owner_fs = vfs;
        }

		const VfsNode& operator*() const
		{
            m_node.path = m_itr->first;
			return m_node;
		}

        const VfsNode* operator->() const
		{
            m_node.path = m_itr->first;
			return &m_node;
		}

		FileNodeIterator& operator++()	{ ++m_itr; return *this;}
		FileNodeIterator operator++(int) { 
			FileNodeIterator ret = *this; 
			++*this; 
			return ret;
		}

		FileNodeIterator& operator--(){ return *this;}
		FileNodeIterator operator--(int){ return *this;}

		bool operator==(const FileNodeIterator& rhs) const
		{
			return m_itr == rhs.m_itr;
		}
	private:
		iterator m_itr;
        mutable VfsNode m_node;
	};



}}}

#endif //end SRC_XIRANG_VFS_FILE_TREE_H

