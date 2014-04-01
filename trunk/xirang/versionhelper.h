#ifndef XIRANG_VERSION_HELPER_H
#define XIRANG_VERSION_HELPER_H
#include <xirang/versiontype.h>
#include <xirang/io/exchs11n.h>
namespace xirang{
	template<typename Ar> sha1_digest sha1_of_archive(Ar& ar, long_size_t max_size  = ~0){
		sha1 sha;
		iref<io::writer> ssha(sha);
		io::copy_data(ar, ssha.template get<io::writer>(), max_size);
		return sha.get_digest();
	}
	template<typename Ar> version_type version_of_archive(Ar& ar, long_size_t max_size  = ~0){
		return version_type(sha1_of_archive(ar, max_size));
	}

	template<typename T> sha1_digest  sha1_of_object(const T& t){
		sha1 sha;
		auto ssha = io::exchange::as_sink(sha);
		ssha & t;
		return sha.get_digest();
	}
	template<typename T> version_type version_of_object(const T& t){
		return version_type(sha1_of_object(t));
	}
}

#endif //end XIRANG_VERSION_HELPER_H

