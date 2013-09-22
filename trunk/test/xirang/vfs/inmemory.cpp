#include "../precompile.h"
#include <xirang/vfs/inmemory.h>
#include "./vfs.h"

BOOST_AUTO_TEST_SUITE(vfs_suite)
using namespace xirang::fs;
using namespace xirang;
using aio::io::archive_mode;
using aio::io::open_flag;

BOOST_AUTO_TEST_CASE(inmemfs_case)
{
    using namespace xirang::fs;
    using namespace xirang;
    using aio::io::archive_mode;
    using aio::io::open_flag;

    InMemory vfs;
    aio::any ret = vfs.getopt(vo_readonly);
    BOOST_CHECK(!ret.empty() && !aio::any_cast<bool>(ret));

    VfsTester tester;
    tester.test_mount(vfs);
    tester.test_on_empty(vfs);
    tester.test_modification(vfs);

    ret = vfs.setopt(vo_readonly, true);
    BOOST_REQUIRE(!ret.empty() && aio::any_cast<bool>(ret));
    tester.test_readonly(vfs);
}
BOOST_AUTO_TEST_SUITE_END()
