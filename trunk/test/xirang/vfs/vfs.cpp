#include "../precompile.h"
#include <xirang/vfs/inmemory.h>
#include "./vfs.h"

using namespace xirang::fs;
using namespace xirang;
using aio::io::archive_mode;
using aio::io::open_flag;


void VfsTester::test_mount(xirang::fs::IVfs& vfs)
{
    BOOST_CHECK(vfs.root() == 0);
	BOOST_CHECK(!vfs.mounted());
    BOOST_CHECK(vfs.mountPoint() == aio::empty_str);

    RootFs root("made everything simple");
    fs_error err = root.mount("/", vfs);
	BOOST_CHECK(err == aiofs::er_ok);
	BOOST_CHECK(vfs.root() == &root);
	BOOST_CHECK(vfs.mounted());
	BOOST_CHECK(vfs.mountPoint() == "/");

    BOOST_CHECK(root.unmount("/") == aiofs::er_ok);
    BOOST_CHECK(!vfs.mounted());
    BOOST_CHECK(vfs.mountPoint() == aio::empty_str);
}

void VfsTester::test_on_empty(xirang::fs::IVfs& vfs)
{
    xirang::fs::VfsNodeRange children = vfs.children(aio::empty_str);
	BOOST_CHECK(children.begin() == children.end());

    children = vfs.children("not found");
	BOOST_CHECK(children.begin() == children.end());

	VfsState st = vfs.state(aio::empty_str);    //test root
	BOOST_CHECK(st.state == aiofs::st_dir);
	BOOST_CHECK(st.node.path == aio::empty_str && "state root of vfs");
	BOOST_CHECK(st.node.owner_fs == &vfs);

    st = vfs.state("not found");
	BOOST_CHECK(st.state == aiofs::st_not_found);
	BOOST_CHECK(st.node.path == "not found");
	BOOST_CHECK(st.node.owner_fs == &vfs);

    BOOST_CHECK(vfs.createDir(aio::empty_str) != aiofs::er_ok); //create root
    BOOST_CHECK(vfs.createDir("not/found") != aiofs::er_ok);
    aio::io::archive_ptr file1 = vfs.create("not found", aio::io::mt_read | aio::io::mt_write | aio::io::mt_random
			, aio::io::of_open);
	BOOST_CHECK(!file1.get());

    BOOST_CHECK(vfs.remove(aio::empty_str) != aiofs::er_ok);
    BOOST_CHECK(vfs.remove("not found") != aiofs::er_ok);

    BOOST_CHECK(vfs.copy("not found1", "not found2") != aiofs::er_ok);

    BOOST_CHECK(vfs.truncate(aio::empty_str, 10) != aiofs::er_ok);
    BOOST_CHECK(vfs.truncate("not found", 10) != aiofs::er_ok);
}

void VfsTester::test_modification(xirang::fs::IVfs& vfs)
{
    BOOST_CHECK(vfs.createDir("dir1") == aiofs::er_ok);
	BOOST_CHECK(vfs.createDir("dir2") == aiofs::er_ok);
	BOOST_CHECK(vfs.createDir("dir1/dir11") == aiofs::er_ok);

	VfsNodeRange children = vfs.children(aio::empty_str);
	BOOST_CHECK(children.begin() != children.end());
    VfsNodeRange::iterator itr = children.begin();
	BOOST_CHECK((*itr).path == "dir1" || (*itr).path == "dir2");
    ++itr;
	BOOST_CHECK((*itr).path == "dir1" || (*itr).path == "dir2");
    ++itr;
    BOOST_CHECK(itr == children.end());

    children = vfs.children("dir1");
    itr = children.begin();
	BOOST_CHECK((*itr).path == "dir11");
    ++itr;
    BOOST_CHECK(itr == children.end());


    aio::io::archive_ptr file1 = vfs.create("dir1/file1", aio::io::mt_read | aio::io::mt_write | aio::io::mt_random
			, aio::io::of_create);
	BOOST_REQUIRE(file1.get());
    BOOST_CHECK(vfs.state("dir1/file1").node.path == "dir1/file1");
    BOOST_CHECK(vfs.state("dir1/file1").size == 0);
    BOOST_CHECK(vfs.state("dir1/file1").state == aiofs::st_regular);

	aio::io::random* prange = file1->query_random();
	BOOST_REQUIRE(prange != 0);
	BOOST_CHECK(prange->offset() == 0);
	BOOST_CHECK(prange->size() == 0);

	aio::io::writer* pwriter = file1->query_writer();
	BOOST_REQUIRE(pwriter != 0);
	string text = "make everything simple, as simple as posibble, but no more simpler";
	pwriter->write(string_to_c_range(text));
	BOOST_CHECK(prange->size() == text.size());
	BOOST_CHECK(prange->offset() == text.size());

	vfs.sync();
    file1.reset();

	BOOST_CHECK(vfs.copy("dir1/file1", "dest") == aiofs::er_ok);

    BOOST_CHECK(vfs.truncate("dest", text.size()/2) == aiofs::er_ok);

    aio::io::archive_ptr file_dest = vfs.create("dest", aio::io::mt_read | aio::io::mt_random
			, aio::io::of_open);
    BOOST_REQUIRE(file_dest);
    BOOST_CHECK(file_dest->query_random() && file_dest->query_random()->size() == text.size()/2);
    BOOST_CHECK(vfs.state("dest").size== text.size()/2);
    file_dest.reset();

    BOOST_CHECK(vfs.remove("dest") == aiofs::er_ok);
    BOOST_CHECK(vfs.state("dest").state == aiofs::st_not_found);
}


void VfsTester::test_readonly(xirang::fs::IVfs& vfs)
{
    BOOST_CHECK(vfs.createDir("dir3") != aiofs::er_ok);
	BOOST_CHECK(vfs.createDir("dir3/dir31") != aiofs::er_ok);

    aio::io::archive_ptr file1 = vfs.create("dir1/file1", aio::io::mt_read | aio::io::mt_write | aio::io::mt_random
			, aio::io::of_create_or_open);
	BOOST_REQUIRE(!file1.get());
	vfs.sync();
    

    BOOST_CHECK(vfs.state("dir1/file1").state == aiofs::st_regular);
	BOOST_CHECK(vfs.copy("dir1/file1", "dest") != aiofs::er_ok);
    BOOST_CHECK(vfs.truncate("dir1/file1", 0) != aiofs::er_ok);
    BOOST_CHECK(vfs.remove("dir1/file1") != aiofs::er_ok);
    BOOST_CHECK(vfs.state("dir1/file1").state == aiofs::st_regular);
}

BOOST_AUTO_TEST_SUITE(vfs_suite)

BOOST_AUTO_TEST_CASE(rootfs_case)
{
    InMemory vfs;
    InMemory vfs2;

    string resource = "made everything simple";
	RootFs root(resource);

	BOOST_CHECK(root.resource() == resource);

	fs_error err = root.mount("/", vfs);
	BOOST_CHECK(err == aiofs::er_ok);
	BOOST_CHECK(vfs.root() == &root);
	BOOST_CHECK(vfs.mounted());
	BOOST_CHECK(vfs.mountPoint() == "/");

	BOOST_CHECK(!root.containMountPoint("/"));

    BOOST_REQUIRE(vfs.createDir("dir2") == aiofs::er_ok);
	BOOST_CHECK(root.mount("/dir2", vfs2) == aiofs::er_ok);
	BOOST_CHECK(root.containMountPoint("/"));
	BOOST_CHECK(!root.containMountPoint("/dir2"));

	BOOST_CHECK(root.mountPoint(vfs2) == "/dir2/");
	BOOST_CHECK(root.mountPoint("/dir2/") == &vfs2);
	BOOST_CHECK(root.mountPoint("/not_exist") == 0);

	VfsRange mntfs = root.mountedFS();
	int count = 0;
	for (VfsRange::iterator itr = mntfs.begin(); itr != mntfs.end(); ++itr, ++count);
	BOOST_CHECK(count == 2);

	vfs.sync();

    
    vfs.createDir("dir1");
    vfs.create("dir1/file1", aio::io::mt_write, aio::io::of_create);
	BOOST_CHECK(vfs2.copy("/dir1/file1", "dest") == aiofs::er_ok);

	VfsState st = root.locate("/dir2/dest");
	BOOST_CHECK(st.state == aiofs::st_regular);
	BOOST_CHECK(st.size == 0);
	BOOST_CHECK(st.node.owner_fs == &vfs2);

	BOOST_CHECK(root.unmount("/") == aiofs::er_busy_mounted);
	BOOST_CHECK(root.unmount("/dir2") == aiofs::er_ok);
	BOOST_CHECK(root.unmount("/") == aiofs::er_ok);
}

BOOST_AUTO_TEST_SUITE_END()

