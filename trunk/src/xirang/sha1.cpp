#include <xirang/sha1.h>
namespace xirang{
	namespace {
		 uint32_t left_rotate(uint32_t x, std::size_t n)
		{
			return (x<<n) ^ (x>> (32-n));
		}
		 const char* K_hex_table= "0123456789abcdef";
	}
	sha1_digest::sha1_digest(){
		for (auto& i : v)
			i = 0;
	}

	bool sha1_digest::operator==(const sha1_digest& rhs) const{
		for (int i = 0; i != 5; ++i){
			if (v[i] != rhs.v[i])	return false;
		}

		return true;
	}
	 bool sha1_digest::operator<(const sha1_digest& rhs) const{
		 return v[0] < rhs.v[0]
			 || (v[0] == rhs.v[0] && v[1] < rhs.v[1])
			 || (v[1] == rhs.v[1] && v[2] < rhs.v[2])
			 || (v[2] == rhs.v[2] && v[3] < rhs.v[3])
			 || (v[3] == rhs.v[3] && v[4] < rhs.v[4]);
	 }


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


	 sha1::sha1()
	{
		reset();
	}
	bool sha1::writable() const{ return true;}
	void sha1::sync(){}

	 void sha1::reset()
	{
		data.h_.v[0] = 0x67452301;
		data.h_.v[1] = 0xEFCDAB89;
		data.h_.v[2] = 0x98BADCFE;
		data.h_.v[3] = 0x10325476;
		data.h_.v[4] = 0xC3D2E1F0;

		data.block_byte_index_ = 0;
		data.bit_count_low = 0;
		data.bit_count_high = 0;
	}
    void process_block(sha1::sha1_data&);
    void process_byte_impl(sha1::sha1_data&, uint8_t byte);

	 void sha1::process_byte(byte b)
	{
		process_byte_impl(data, uint8_t(b));

		// size_t max value = 0xFFFFFFFF
		//if (bit_count_low + 8 >= 0x100000000)  // would overflow
		//if (bit_count_low >= 0x100000000-8)
		if (data.bit_count_low < 0xFFFFFFF8) {
			data.bit_count_low += 8;
		} else {
			data.bit_count_low = 0;

			if (data.bit_count_high <= 0xFFFFFFFE) {
				++data.bit_count_high;
			} else {
				AIO_THROW(sha1_runtime_exception)("sha1 too many bytes");
			}
		}
	}

	 void process_byte_impl(sha1::sha1_data& data, uint8_t b)
	 {
		 data.block_[data.block_byte_index_++] = uint8_t(b);

		 if (data.block_byte_index_ == 64) {
			 data.block_byte_index_ = 0;
			 process_block(data);
		 }
	 }

	 range<const byte*> sha1::write(const range<const byte*>& r)
	 {
		 for (auto i : r)
			 process_byte(i);
		 return range<const byte*>(r.end(), r.end());
	 }

	 void process_block(sha1::sha1_data& data)
	 {
		 uint32_t w[80];
		 for (std::size_t i=0; i<16; ++i) {
			 w[i]  = (data.block_[i*4 + 0] << 24);
			 w[i] |= (data.block_[i*4 + 1] << 16);
			 w[i] |= (data.block_[i*4 + 2] << 8);
			 w[i] |= (data.block_[i*4 + 3]);
		 }
		 for (std::size_t i=16; i<80; ++i) {
			 w[i] = left_rotate((w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]), 1);
		 }

		 uint32_t a = data.h_.v[0];
		 uint32_t b = data.h_.v[1];
		 uint32_t c = data.h_.v[2];
		 uint32_t d = data.h_.v[3];
		 uint32_t e = data.h_.v[4];

		 for (std::size_t i=0; i<80; ++i) {
			 uint32_t f;
			 uint32_t k;

			 if (i<20) {
				 f = (b & c) | (~b & d);
				 k = 0x5A827999;
			 } else if (i<40) {
				 f = b ^ c ^ d;
				 k = 0x6ED9EBA1;
			 } else if (i<60) {
				 f = (b & c) | (b & d) | (c & d);
				 k = 0x8F1BBCDC;
			 } else {
				 f = b ^ c ^ d;
				 k = 0xCA62C1D6;
			 }

			 uint32_t temp = left_rotate(a, 5) + f + e + k + w[i];
			 e = d;
			 d = c;
			 c = left_rotate(b, 30);
			 b = a;
			 a = temp;
		 }

		 data.h_.v[0] += a;
		 data.h_.v[1] += b;
		 data.h_.v[2] += c;
		 data.h_.v[3] += d;
		 data.h_.v[4] += e;
	 }

	 sha1_digest sha1::get_digest() const
	 {
		 sha1_data d = data;
		 // append the bit '1' to the message
		 process_byte_impl(d, 0x80);

		 // append k bits '0', where k is the minimum number >= 0
		 // such that the resulting message length is congruent to 56 (mod 64)
		 // check if there is enough space for padding and bit_count
		 if (d.block_byte_index_ > 56) {
			 // finish this block
			 while (d.block_byte_index_ != 0) {
				 process_byte_impl(d, 0);
			 }

			 // one more block
			 while (d.block_byte_index_ < 56) {
				 process_byte_impl(d, 0);
			 }
		 } else {
			 while (d.block_byte_index_ < 56) {
				 process_byte_impl(d, 0);
			 }
		 }

		 // append length of message (before pre-processing)
		 // as a 64-bit big-endian integer
		 process_byte_impl(d, static_cast<uint8_t>((d.bit_count_high>>24) & 0xFF) );
		 process_byte_impl(d, static_cast<uint8_t>((d.bit_count_high>>16) & 0xFF) );
		 process_byte_impl(d, static_cast<uint8_t>((d.bit_count_high>>8 ) & 0xFF) );
		 process_byte_impl(d, static_cast<uint8_t>((d.bit_count_high)     & 0xFF) );
		 process_byte_impl(d, static_cast<uint8_t>((d.bit_count_low>>24) & 0xFF) );
		 process_byte_impl(d, static_cast<uint8_t>((d.bit_count_low>>16) & 0xFF) );
		 process_byte_impl(d, static_cast<uint8_t>((d.bit_count_low>>8 ) & 0xFF) );
		 process_byte_impl(d, static_cast<uint8_t>((d.bit_count_low)     & 0xFF) );

		 return d.h_;
	 }
}

