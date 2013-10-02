#include "../precompile.h"
#include <xirang/vfs/inmemory.h>
#include "./vfs.h"

using namespace xirang::vfs;
using namespace xirang;
using xirang::io::open_flag;

void VfsTester::test_mount(xirang::vfs::IVfs& vfs)
{
	const sub_file_path root_path(literal("/"));
    BOOST_CHECK(vfs.root() == 0);
	BOOST_CHECK(!vfs.mounted());
    BOOST_CHECK(vfs.mountPoint() == file_path());

    RootFs root("made everything simple");
    fs_error err = root.mount(root_path, vfs);
	BOOST_CHECK(err == fs::er_ok);
	BOOST_CHECK(vfs.root() == &root);
	BOOST_CHECK(vfs.mounted());
	BOOST_CHECK(vfs.mountPoint() == root_path);

    BOOST_CHECK(root.unmount(root_path) == fs::er_ok);
    BOOST_CHECK(!vfs.mounted());
    BOOST_CHECK(vfs.mountPoint() == file_path());
}

void VfsTester::test_on_empty(xirang::vfs::IVfs& vfs)
{
    xirang::vfs::VfsNodeRange children = vfs.children(sub_file_path());
	BOOST_CHECK(children.begin() == children.end());

    children = vfs.children(sub_file_path(literal("not found")));
	BOOST_CHECK(children.begin() == children.end());

	VfsState st = vfs.state(sub_file_path());    //test root
	BOOST_CHECK(st.state == fs::st_dir);
	BOOST_CHECK(st.node.path == sub_file_path() && "state root of vfs");
	BOOST_CHECK(st.node.owner_fs == &vfs);

    st = vfs.state(sub_file_path(literal("not found")));
	BOOST_CHECK(st.state == fs::st_not_found);
	BOOST_CHECK(st.node.path == sub_file_path(literal("not found")));
	BOOST_CHECK(st.node.owner_fs == &vfs);

    BOOST_CHECK(vfs.createDir(sub_file_path()) != fs::er_ok); //create root
    BOOST_CHECK(vfs.createDir(sub_file_path(literal("not/found"))) != fs::er_ok);
    BOOST_CHECK_THROW((vfs.create<io::reader, io::writer, io::random>(sub_file_path(literal("not found")), xirang::io::of_open)), fs::not_found_exception);

    BOOST_CHECK(vfs.remove(sub_file_path()) != fs::er_ok);
    BOOST_CHECK(vfs.remove(sub_file_path(literal("not found"))) != fs::er_ok);

    BOOST_CHECK(vfs.copy(sub_file_path(literal("not found1")), sub_file_path(literal("not found2"))) != fs::er_ok);

    BOOST_CHECK(vfs.truncate(sub_file_path(), 10) != fs::er_ok);
    BOOST_CHECK(vfs.truncate(sub_file_path(literal("not found")), 10) != fs::er_ok);
}

void VfsTester::test_modification(xirang::vfs::IVfs& vfs)
{
	const file_path dir1("dir1");
	const file_path dir2("dir2");

    BOOST_CHECK(vfs.createDir(dir1) == fs::er_ok);
	BOOST_CHECK(vfs.createDir(dir2) == fs::er_ok);
	BOOST_CHECK(vfs.createDir(dir1/file_path("dir11")) == fs::er_ok);

	VfsNodeRange children = vfs.children(file_path());
	BOOST_CHECK(children.begin() != children.end());
    VfsNodeRange::iterator itr = children.begin();
	BOOST_CHECK((*itr).path == dir1 || (*itr).path == dir2);
    ++itr;
	BOOST_CHECK((*itr).path == dir1 || (*itr).path == dir2);
    ++itr;
    BOOST_CHECK(itr == children.end());

    children = vfs.children(dir1);
    itr = children.begin();
	BOOST_CHECK((*itr).path == file_path("dir11"));
    ++itr;
    BOOST_CHECK(itr == children.end());


    auto file1 = vfs.create<io::reader, io::writer, io::random>(file_path("dir1/file1"), xirang::io::of_create);
    BOOST_CHECK(vfs.state(file_path("dir1/file1")).node.path == file_path("dir1/file1"));
    BOOST_CHECK(vfs.state(file_path("dir1/file1")).size == 0);
    BOOST_CHECK(vfs.state(file_path("dir1/file1")).state == fs::st_regular);

	xirang::io::random& prange = file1.get<io::random>();
	BOOST_CHECK(prange.offset() == 0);
	BOOST_CHECK(prange.size() == 0);

	xirang::io::writer& pwriter = file1.get<io::writer>();
	string text = "make everything simple, as simple as posibble, but no more simpler";
	pwriter.write(string_to_c_range(text));
	BOOST_CHECK(prange.size() == text.size());
	BOOST_CHECK(prange.offset() == text.size());

	vfs.sync();
    file1.reset();

	BOOST_CHECK(vfs.copy(file_path("dir1/file1"), file_path("dest")) == fs::er_ok);

    BOOST_CHECK(vfs.truncate(file_path("dest"), text.size()/2) == fs::er_ok);

    auto file_dest = vfs.create<io::reader, io::random>(file_path("dest"), xirang::io::of_open);
    BOOST_REQUIRE(file_dest.valid());
    BOOST_CHECK(file_dest.get<io::random>().size() == text.size()/2);
    BOOST_CHECK(vfs.state(file_path("dest")).size== text.size()/2);
    file_dest.reset();

    BOOST_CHECK(vfs.remove(file_path("dest")) == fs::er_ok);
    BOOST_CHECK(vfs.state(file_path("dest")).state == fs::st_not_found);
}


void VfsTester::test_readonly(xirang::vfs::IVfs& vfs)
{
    BOOST_CHECK(vfs.createDir(file_path("dir3")) != fs::er_ok);
	BOOST_CHECK(vfs.createDir(file_path("dir3/dir31")) != fs::er_ok);

    BOOST_CHECK_THROW((vfs.create<io::reader, io::writer, io::random>(file_path("dir1/file1"), xirang::io::of_create_or_open)), fs::permission_denied_exception);
	vfs.sync();

    BOOST_CHECK(vfs.state(file_path("dir1/file1")).state == fs::st_regular);
	BOOST_CHECK(vfs.copy(file_path("dir1/file1"), file_path("dest")) != fs::er_ok);
    BOOST_CHECK(vfs.truncate(file_path("dir1/file1"), 0) != fs::er_ok);
    BOOST_CHECK(vfs.remove(file_path("dir1/file1")) != fs::er_ok);
    BOOST_CHECK(vfs.state(file_path("dir1/file1")).state == fs::st_regular);
}

BOOST_AUTO_TEST_SUITE(vfs_suite)

BOOST_AUTO_TEST_CASE(rootfs_case)
{
    InMemory vfs;
    InMemory vfs2;

    string resource = "made everything simple";
	RootFs root(resource);

	BOOST_CHECK(root.resource() == resource);

	fs_error err = root.mount(file_path("/"), vfs);
	BOOST_CHECK(err == fs::er_ok);
	BOOST_CHECK(vfs.root() == &root);
	BOOST_CHECK(vfs.mounted());
	BOOST_CHECK(vfs.mountPoint() == file_path("/"));

	BOOST_CHECK(!root.containMountPoint(file_path("/")));

    BOOST_REQUIRE(vfs.createDir(file_path("dir2")) == fs::er_ok);
	BOOST_CHECK(root.mount(file_path("/dir2"), vfs2) == fs::er_ok);
	BOOST_CHECK(root.containMountPoint(file_path("/")));
	BOOST_CHECK(!root.containMountPoint(file_path("/dir2")));

	BOOST_CHECK(root.mountPoint(vfs2) == file_path("/dir2/"));
	BOOST_CHECK(root.mountPoint(file_path("/dir2/")) == &vfs2);
	BOOST_CHECK(root.mountPoint(file_path("/not_exist")) == 0);

	VfsRange mntfs = root.mountedFS();
	int count = 0;
	for (VfsRange::iterator itr = mntfs.begin(); itr != mntfs.end(); ++itr, ++count);
	BOOST_CHECK(count == 2);

	vfs.sync();

    
    vfs.createDir(file_path("dir1"));
    vfs.create<io::writer>(file_path("dir1/file1"), xirang::io::of_create);
	BOOST_CHECK(vfs2.copy(file_path("/dir1/file1"), file_path("dest")) == fs::er_ok);

	VfsState st = root.locate(file_path("/dir2/dest"));
	BOOST_CHECK(st.state == fs::st_regular);
	BOOST_CHECK(st.size == 0);
	BOOST_CHECK(st.node.owner_fs == &vfs2);

	BOOST_CHECK(root.unmount(file_path("/")) == fs::er_busy_mounted);
	BOOST_CHECK(root.unmount(file_path("/dir2")) == fs::er_ok);
	BOOST_CHECK(root.unmount(file_path("/")) == fs::er_ok);
}

BOOST_AUTO_TEST_SUITE_END()

