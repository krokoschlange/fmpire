#include "utils.h"

#include <cstring>

namespace fmpire
{

static const char* base64_code = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
								 "abcdefghijklmnopqrstuvwxyz"
								 "0123456789+/";

static int base64_decode[256] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  62, 0,  0,  0,  63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
	61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,
	0,  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,
};

std::string encode_base64(const uint8_t* const data, const size_t size)
{
	std::string encoded((size * 4) / 3 + 4, '*');
	uint32_t char_idx = 0;
	size_t byte = 0;

	for (; byte < size; byte += 3)
	{
		uint8_t block[3] = {0, 0, 0};
		std::memcpy(&block, data + byte, std::min(size - byte, 3ul));

		encoded[char_idx++] = base64_code[block[0] >> 2];
		encoded[char_idx++] =
			base64_code[((block[0] & 0b11) << 4) + (block[1] >> 4)];
		encoded[char_idx++] =
			base64_code[((block[1] & 0b1111) << 2) + (block[2] >> 6)];
		encoded[char_idx++] = base64_code[block[2] & 0b111111];
	}

	uint8_t padding_bytes = byte - size;

	encoded.resize(char_idx - padding_bytes);

	return encoded;
}

size_t decode_base64(const std::string& encoded,
					 size_t offset,
					 uint8_t* decoded,
					 const size_t decoded_size)
{
	size_t byte = 0;
	uint8_t current_bytes[2] = {0, 0};
	uint8_t bits = 0;
	size_t char_idx = 0;

	for (; byte < decoded_size && char_idx < encoded.size(); char_idx++)
	{
		uint8_t char_val = base64_decode[encoded[char_idx]] << 2;
		current_bytes[0] |= char_val >> bits;
		current_bytes[1] |= char_val << (8 - bits);
		bits += 6;

		if (bits >= 8)
		{
			decoded[byte] = current_bytes[0];
			bits -= 8;
			byte++;
			current_bytes[0] = current_bytes[1];
			current_bytes[1] = 0;
		}
	}
	return char_idx;
}

void decode_base64(std::string_view& encoded,
				   uint8_t* decoded,
				   const size_t decoded_size)
{
	size_t byte = 0;
	uint8_t current_bytes[2] = {0, 0};
	uint8_t bits = 0;
	size_t char_idx = 0;

	for (; byte < decoded_size && encoded.size() > 0; char_idx++)
	{
		uint8_t char_val = base64_decode[encoded[0]] << 2;
		encoded.remove_prefix(1);

		current_bytes[0] |= char_val >> bits;
		current_bytes[1] |= char_val << (8 - bits);
		bits += 6;

		if (bits >= 8)
		{
			decoded[byte] = current_bytes[0];

			bits -= 8;
			byte++;
			current_bytes[0] = current_bytes[1];
			current_bytes[1] = 0;
		}
	}
}


} // namespace fmpire
