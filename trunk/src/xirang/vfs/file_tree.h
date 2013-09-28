#ifndef SRC_XIRANG_VFS_FILE_TREE_H
#define SRC_XIRANG_VFS_FILE_TREE_H

#include <xirang/vfs.h>
#include <xirang/string_algo/string.h>

#include <algorithm>
#include <unordered_map>

namespace xirang{ namespace vfs{ 
	AIO_EXCEPTION_TYPE(empty_local_file_name);
	AIO_EXCEPTION_TYPE(not_all_parent_are_dir);
}}

namespace xirang{ namespace vfs{ namespace private_{

	struct hash_file_path{
		size_t operator()(const file_path& path) const{
			return path.str().hash();
		}
	};

	
	template<typename T> struct file_node
	{
		typedef file_node<T> node_type;

		string name;
		file_state type;	//dir or normal
		std::unordered_map<file_path, std::unique_ptr<node_type>, hash_file_path> children;
		node_type* parent;

		T data;

		file_node(node_type* parent_ = 0) :
			type(fs::st_dir),
			parent(parent_),
			data()
		{}
	};

	template<typename T> struct locate_result{
		file_node<T>* node;
		sub_file_path not_found;
	};

	template <typename T>
	locate_result<T> locate(file_node<T>& root, sub_file_path path) 
	{
		file_node<T>* pos = &root;
		
		auto itr = path.begin();
		for (auto end(path.end()); itr != end; ++itr ){
			auto child = pos->children.find(*itr);
			if (child != pos->children.end())
				pos = child->second.get();
			else
				break;
		}

		typedef locate_result<T> return_type;
		return return_type {
			pos, sub_file_path(itr->str().begin(), path.end())
		};
	}


	template<typename T>
	fs_error removeNode(file_node<T>* node)
	{
		AIO_PRE_CONDITION(node && node->parent && node->parent->children.count(node->name));
        if (!node->children.empty())
            return fs::er_not_empty;

		node->parent->children.erase(node->name);
		return fs::er_ok;
	}


	template<typename T>
	file_node<T>* create_node(const locate_result<T>& pos, file_state type, bool whole_path){
		AIO_PRE_CONDITION(pos.node);
		AIO_PRE_CONDITION(!pos.not_found.empty());
		AIO_PRE_CONDITION(!contains(pos.not_found, '/'));
		AIO_PRE_CONDITION(pos.node->children.count(pos.not_found) == 0);

		bool first_create = true;

		auto node = pos.node;
		for (auto& item : pos.not_found){
			AIO_PRE_CONDITION(!node->children.count(item));

			if (!whole_path && !first_create){
				node->parent->children.erase(node->name);
				return 0;
			}

			std::unique_ptr<file_node<T> > fnode(new file_node<T>(node));
			fnode->name = item;
			fnode->type = fs::st_dir;
			auto& new_node = pos.node->children[fnode->name];
			new_node = std::move(fnode);
			pos = new_node.get();
			first_create = false;
		}
		node->type = type;
	}
	template<typename T> file_node<T>* create_node(file_node<T>& root, sub_file_path path, file_state type, bool whole_path)
	{
		AIO_PRE_CONDITION(!path.empty());

		auto result = locate(root, path);
		if (result.not_found.empty())
			return 0;

		return create_node(result, type, whole_path);
	}

	template<typename T>
	class FileNodeIterator
	{
	public:
		typedef file_node<T> node_type;
		typedef typename std::unordered_map<string, std::unique_ptr<node_type>, hash_string>::iterator iterator;
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

