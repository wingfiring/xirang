#include "../precompile.h"
#include <aio/common/fsutility.h>
#include <aio/xirang/vfs/local.h>
#include "./vfs.h"

BOOST_AUTO_TEST_SUITE(vfs_suite)
using namespace xirang::fs;
using namespace xirang;
using aio::io::archive_mode;
using aio::io::open_flag;

BOOST_AUTO_TEST_CASE(localfs_case)
{
    string path1 = aio::fs::temp_dir("localfs");
    path1 = append_tail_slash(path1);

    {
        LocalFs local1(path1);

        VfsTester tester;
        tester.test_mount(local1);
        tester.test_on_empty(local1);
        tester.test_modification(local1);

        //check unicode path
        aio::string file_name = "\xd0\xa0\xd0\xb0\xd0\xb7\xd0\xbd\xd0\xbe\xd0\xb5";
        xirang::fs::VfsState ust = local1.state(file_name);
        BOOST_CHECK(ust.state == aiofs::st_not_found);
        local1.create(file_name, archive_mode( aio::io::mt_read |  aio::io::mt_write), aio::io::of_create);
        ust = local1.state(file_name);
        BOOST_CHECK(ust.state == aiofs::st_regular);
        BOOST_CHECK(ust.size == 0);

    }
	// clean
    aio::fs::recursive_remove(path1);
}


BOOST_AUTO_TEST_SUITE_END()
