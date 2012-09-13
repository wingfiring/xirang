#ifndef SRC_XIRANG_VFS_FILE_TREE_H
#define SRC_XIRANG_VFS_FILE_TREE_H

#include <aio/xirang/vfs.h>
#include <boost/tokenizer.hpp>
#include <map>
#include <algorithm>
#include <aio/common/char_separator.h>

namespace xirang{ namespace fs{ 
	AIO_EXCEPTION_TYPE(empty_local_file_name);
	AIO_EXCEPTION_TYPE(not_all_parent_are_dir);
}}

namespace xirang{ namespace fs{ namespace private_{

	template <typename T> struct node_releaser;

	template<typename T> struct file_node
	{
		typedef file_node<T> node_type;

		string name;
		file_state type;	//dir or normal
		std::map<string, node_type*> children;
		node_type* parent;
		T data;

		file_node() :
			type(aiofs::st_dir),
			parent(0),
			data()
		{}

		~file_node()
		{
			for (typename std::map<string, node_type*>::iterator itr = children.begin(); itr != children.end(); ++itr)
			{
				delete itr->second;
			}

			node_releaser<T>::release(data);
		}
	};

	template <typename T>
	file_node<T>* locate_parent(file_node<T>& root, const string& path, string& base) 
	{
        aio::char_separator sep('/');
        typedef boost::tokenizer<aio::char_separator, string::const_iterator, aio::const_range_string> tokenizer;
        tokenizer tokens(path, sep);


		file_node<T>* pos = &root;
        tokenizer::iterator itr = tokens.begin();
		for (; itr != tokens.end(); ++itr)
		{
			typename std::map<string, file_node<T>*>::iterator child = pos->children.find(*itr);
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
	file_node<T>* locate(file_node<T>& root, const string& path) 
	{
		if (path.empty())	//root
			return &root;

		string base;
		file_node<T>*  pos = locate_parent(root, path, base);
		if (!is_filename(base) || !pos)
			return 0;

		typename std::map<string, file_node<T>*>::iterator itr = pos->children.find(base);
		return itr == pos->children.end() ? 0 : itr->second;
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

		aio::char_separator sep('/');
        typedef boost::tokenizer<aio::char_separator, string::const_iterator, aio::const_range_string> tokenizer;
        tokenizer tokens(path, sep);

		file_node<T>* pos = &root;

        tokenizer::iterator name_first = tokens.begin(); 
		tokenizer::iterator name_end = tokens.end(); 

		int create_times = 0;
		for(;name_first != name_end; ++name_first)
		{
			if (pos->type != aiofs::st_dir)
				AIO_THROW(not_all_parent_are_dir);

			typename std::map<string, file_node<T>*>::iterator found = pos->children.find(*name_first);
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
	class FileNodeIterator
	{
	public:
		typedef file_node<T> node_type;
		typedef typename std::map<string, node_type*>::iterator iterator;
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

