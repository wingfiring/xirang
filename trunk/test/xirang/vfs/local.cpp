#include "../precompile.h"
#include <xirang/fsutility.h>
#include <xirang/vfs/local.h>
#include "./vfs.h"

BOOST_AUTO_TEST_SUITE(vfs_suite)
using namespace xirang::vfs;
using namespace xirang;
using xirang::io::open_flag;

BOOST_AUTO_TEST_CASE(localfs_case)
{
    file_path path1 = xirang::fs::temp_dir(file_path("localfs"));

    {
        LocalFs local1(path1);

        VfsTester tester;
        tester.test_mount(local1);
        tester.test_on_empty(local1);
        tester.test_modification(local1);

        //check unicode path
        file_path file_name("\xd0\xa0\xd0\xb0\xd0\xb7\xd0\xbd\xd0\xbe\xd0\xb5");
        VfsState ust = local1.state(file_name);
        BOOST_CHECK(ust.state == fs::st_not_found);
        local1.create<io::reader, io::writer>(file_name, io::of_create);
        ust = local1.state(file_name);
        BOOST_CHECK(ust.state == fs::st_regular);
        BOOST_CHECK(ust.size == 0);

    }
	// clean
    xirang::fs::recursive_remove(path1);
}


BOOST_AUTO_TEST_SUITE_END()
