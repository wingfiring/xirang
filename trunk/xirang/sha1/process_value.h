#ifndef XIRANG_SHA1_PROCESS_VALUE_H
#define XIRANG_SHA1_PROCESS_VALUE_H

#include <xirang/sha1.h>
#include <xirang/io/exchs11n.h>

#include <xirang/buffer.h>
#include <xirang/string.h>
#include <xirang/versiontype.h>

namespace xirang{
	template<typename T, typename = typename std::enable_if<std::is_scalar<T>::value, void>::type>
	inline void process_value(sha1& sha, T t){
		byte const * p = (byte const*)&t;
		sha.write(make_range(p, p + sizeof(t)));
	}
#if 0
	inline void process_value(sha1& sha, const_range_string s){
		process_value(sha, s.size());
		sha.process_bytes((const byte*)&*s.begin(), s.size());
	}

	inline void process_value(sha1& sha, range<const byte*> rng){
		for (auto i : rng)
			sha.process_byte(i);
	}
	template<typename Itr>
	inline void process_value(sha1& sha, range<Itr> rng){
		for (auto i : rng)
			process_value(sha, *i);
	}

	template<typename T>
	void process_value(sha1& sha, const buffer<T>& buf){
		process_value(sha, buf.size());
		for (auto &i : buf)
			process_value(sha, i);
	}

	inline void process_value(sha1& sha, const sha1_digest& dig){
		for (auto i: dig.v)
			process_value(sha, i);
	}
	inline void process_value(sha1& sha, const version_type& ver){
		process_value(sha, ver.protocol_version);
		process_value(sha, ver.algorithm);
		process_value(sha, ver.id);
		process_value(sha, ver.conflict_id);
	}
#endif

}
#endif //end XIRANG_SHA1_PROCESS_VALUE_H
