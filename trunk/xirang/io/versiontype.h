#ifndef XIRANG_IO_VERSION_TYPE_H
#define XIRANG_IO_VERSION_TYPE_H
#include <xirang/io/s11n.h>
#include <xirang/io/sha1.h>
#include <xirang/versiontype.h>
namespace xirang{
	template<typename Ar, typename =
		typename std::enable_if<io::s11n::is_deserializer<Ar>::value>::type>
	Ar& load(Ar& ar, version_type& ver)
	{
		return ar & ver.protocol_version
			& ver.algorithm
			& ver.id
			& ver.conflict_id;
	}

	template<typename Ar, typename =
		typename std::enable_if< io::s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const version_type& ver)
	{
		ar & ver.protocol_version
			& ver.algorithm
			& ver.id
			& ver.conflict_id;
		return ar;
	}


}
#endif //end XIRANG_IO_VERSION_TYPE_H

