#include <xirang/versionedvfs.h>

namespace xirang{ namespace vfs{

	// in each repo, there is a blob store, it contains all files & trees & submission's blob.
	//
	// in the repo, there is a shadowed file tree contains all history dirs.
	// all files stored in object store, nameless
	// In each folder, place a index file to maintain the item history
	//
	// the format+
	//   submission	name	file_type version	flag
	//
	// for remove:
	// 	just need to new a parent tree blob which doesn't contain the given file,
	//
	// workflow:
	// 1. get base submit
	// 2. get root tree
	// 3. walkthrough each file in workspace
	// 4. for each file, or remove, add to shadow fs, and merge to it's parent tree, save each tree blob to object store
	// 5. repeat 4 untile root
	// 6. save commit
	// 7. set local HEAD to the commit;
	//version_type IVersionedVfs::commitWorkspace()



	// push:
	// check remote HEAD localy, if remote HEAD == local HEAD , no action
	// check remote HEAD from server, if server is not remote HEAD, failed, end require pull & merge.
	// then package all commit between remote HEAD & local HEAD. //send object first, send tree & commit blob last.
	// send the package.
	// after success, set remote HEAD with local HEAD.
	void sample(IVfs& fs){
		auto st1 = fs.state(sub_file_path(literal("/usr/sunjac/~repo/a/b/#419a6056b4854d5546927a54b0dda742cb8df782")));
		AIO_PRE_CONDITION(st1.state = fs::st_regular);
		auto ar = fs.create<io::read_map>(sub_file_path(literal("/usr/sunjac/~repo/#419a6056b4854d5546927a54b0dda742cb8df782/b/c")), io::of_create_or_open);
		AIO_PRE_CONDITION(ar);

		file_path repo_path, path_in_repo;
		IRepository* repo = fs.getRepository(file_path("/usr/sunjac/~repo/a/b"), &repo_path, &path_in_repo);
		AIO_PRE_CONDITION(repo_path == file_path("/usr/sunjac/~repo"));
		AIO_PRE_CONDITION(path_in_repo == file_path("a/b"));
		AIO_PRE_CONDITION(repo);

		auto workspace = repo->getWorkspace();
		AIO_PRE_CONDITION(workspace);

		checkOut(*repo, file_path("/usr/sunjac/~repo/#419a6056b4854d5546927a54b0dda742cb8df782/b/c"));
		AIO_PRE_CONDITION(workspace->state(sub_file_path(literal("b/c"))).state == fs::st_regular);

		{
			auto new_file = workspace->create<io::write_map>(file_path("b/c"), io::of_create_or_open);
			AIO_PRE_CONDITION(new_file);
			// save to new_file;
			// calculate version_type against new_file
		}
		auto submission = workspace->commit("test");
		AIO_PRE_CONDITION(!submission.description.empty());
	}

}}


