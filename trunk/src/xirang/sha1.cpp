#include <xirang/sha1.h>
namespace xirang{
	static const char* K_hex_table= "0123456789abcdef";

	string sha1_digest::to_string() const{

		static_assert(sizeof(v) == 20, "wrong sha1_digest size");

		char buf[40];

		auto itr = &v[0];
		auto end = itr + 5;
		for (char* p = buf; itr != end; ++itr){
			*p++ = K_hex_table[((*itr) >> 28 & 0xF)];
			*p++ = K_hex_table[((*itr) >> 24 & 0xF)];
			*p++ = K_hex_table[((*itr) >> 20 & 0xF)];
			*p++ = K_hex_table[((*itr) >> 16 & 0xF)];
			*p++ = K_hex_table[((*itr) >> 12 & 0xF)];
			*p++ = K_hex_table[((*itr) >>  8 & 0xF)];
			*p++ = K_hex_table[((*itr) >>  4 & 0xF)];
			*p++ = K_hex_table[((*itr) >>  0 & 0xF)];
		}

		return string(const_range_string(buf, 40));
	}
	sha1_digest::sha1_digest(const_range_string dig){
		if (dig.size() != 40) 
			AIO_THROW(bad_sha1_string_exception)("Wrong size");

		int s = 0;
		uint32_t av = 0;
		int idx = 0;

		for (auto i : dig){
			if (i>='0' && i <= '9')
				av = (av << 4) + (i - '0');
			else if (i >= 'a' && i <= 'f')
				av = (av << 4) + (i - 'a' + 10);
			else
				AIO_THROW(bad_sha1_string_exception)("Wrong digit");

			if (++s == 8){
				v[idx++] = av;
				av = 0; 
				s = 0;
			}
		}
	}
}

