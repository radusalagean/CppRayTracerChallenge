#include <gtest/gtest.h>
#include "compression/deflate_block.h"

namespace CppRayTracerChallenge::Core::Compression
{
	struct Params
	{
		std::vector<unsigned char> data = { 'h', 'e', 'l', 'l' ,'o' };
		bool isFinal = true;

		Params()
		{
		}

		Params(bool isFinal)
		{
			this->isFinal = isFinal;
		}

		Params(std::vector<unsigned char> data)
		{
			this->data = data;
		}

		Params(std::vector<unsigned char> data, bool isFinal)
		{
			this->data = data;
			this->isFinal = isFinal;
		}
	};

	DeflateBlock testDeflateBlock(Params params = Params())
	{
		return DeflateBlock(params.data, params.isFinal);
	}

	TEST(CppRayTracerChallenge_Core_Compression_Deflate, deflate_block_sets_header_bfinal_true)
	{
		auto block = testDeflateBlock();

		auto const& data = block.data();

		EXPECT_EQ(data[0], 1);
	}

	TEST(CppRayTracerChallenge_Core_Compression_Deflate, deflate_block_sets_header_bfinal_false)
	{
		auto block = testDeflateBlock(false);

		auto const& data = block.data();

		EXPECT_EQ(data[0], 0);
	}

	TEST(CppRayTracerChallenge_Core_Compression_Deflate, deflate_block_sets_header_btype_to_uncompressed)
	{
		auto block = testDeflateBlock();

		auto const& data = block.data();

		EXPECT_EQ(data[1], 0);
		EXPECT_EQ(data[2], 0);
	}

	TEST(CppRayTracerChallenge_Core_Compression_Deflate, uncompressed_writes_padding)
	{
		auto block = testDeflateBlock();

		auto const& data = block.data();

		EXPECT_EQ(data[3], 0);
		EXPECT_EQ(data[4], 0);
		EXPECT_EQ(data[5], 0);
		EXPECT_EQ(data[6], 0);
		EXPECT_EQ(data[7], 0);
	}

	TEST(CppRayTracerChallenge_Core_Compression_Deflate, uncompressed_writes_length_and_complement)
	{
		auto block = testDeflateBlock();

		auto const& data = block.data();

		// Length (16 bit value)
		int offset = 8;

		EXPECT_EQ(data[0 + offset], 1);
		EXPECT_EQ(data[1 + offset], 0);
		EXPECT_EQ(data[2 + offset], 1);
		EXPECT_EQ(data[3 + offset], 0);
		EXPECT_EQ(data[4 + offset], 0);
		EXPECT_EQ(data[5 + offset], 0);
		EXPECT_EQ(data[6 + offset], 0);
		EXPECT_EQ(data[7 + offset], 0);
		EXPECT_EQ(data[8 + offset], 0);
		EXPECT_EQ(data[9 + offset], 0);
		EXPECT_EQ(data[10 + offset], 0);
		EXPECT_EQ(data[11 + offset], 0);
		EXPECT_EQ(data[12 + offset], 0);
		EXPECT_EQ(data[13 + offset], 0);
		EXPECT_EQ(data[14 + offset], 0);
		EXPECT_EQ(data[15 + offset], 0);

		// Complement (16 bit value)
		offset = offset + 16;

		EXPECT_EQ(data[0 + offset], 0);
		EXPECT_EQ(data[1 + offset], 1);
		EXPECT_EQ(data[2 + offset], 0);
		EXPECT_EQ(data[3 + offset], 1);
		EXPECT_EQ(data[4 + offset], 1);
		EXPECT_EQ(data[5 + offset], 1);
		EXPECT_EQ(data[6 + offset], 1);
		EXPECT_EQ(data[7 + offset], 1);
		EXPECT_EQ(data[8 + offset], 1);
		EXPECT_EQ(data[9 + offset], 1);
		EXPECT_EQ(data[10 + offset], 1);
		EXPECT_EQ(data[11 + offset], 1);
		EXPECT_EQ(data[12 + offset], 1);
		EXPECT_EQ(data[13 + offset], 1);
		EXPECT_EQ(data[14 + offset], 1);
		EXPECT_EQ(data[15 + offset], 1);
	}

	unsigned char readByte(DeflateBlock::DeflateBitset const& bitset, int offset)
	{
		std::bitset<8> bits;

		bits[0] = bitset[0 + offset];
		bits[1] = bitset[1 + offset];
		bits[2] = bitset[2 + offset];
		bits[3] = bitset[3 + offset];
		bits[4] = bitset[4 + offset];
		bits[5] = bitset[5 + offset];
		bits[6] = bitset[6 + offset];
		bits[7] = bitset[7 + offset];

		return static_cast<unsigned char>(bits.to_ulong());
	}

	TEST(CppRayTracerChallenge_Core_Compression_Deflate, uncompressed_writes_bitstream)
	{
		auto block = testDeflateBlock();

		auto const& data = block.data();

		int offset = 40;

		EXPECT_EQ(readByte(data, offset + 0), 'h');
		EXPECT_EQ(readByte(data, offset + 8), 'e');
		EXPECT_EQ(readByte(data, offset + 16), 'l');
		EXPECT_EQ(readByte(data, offset + 24), 'l');
		EXPECT_EQ(readByte(data, offset + 32), 'o');
	}

	TEST(CppRayTracerChallenge_Core_Compression_Deflate, uncompressed_with_max_bytes_throws_no_error)
	{
		std::vector<unsigned char> input;

		// Fill input with MAX_BYTES for Deflate Block
		for (unsigned int i = 0; i < DeflateBlock::MAX_BYTES; ++i)
		{
			input.push_back('A');
		}

		EXPECT_NO_THROW(testDeflateBlock(input));
	}

	TEST(CppRayTracerChallenge_Core_Compression_Deflate, uncompressed_with_more_than_max_bytes_throws_error)
	{
		std::vector<unsigned char> input;

		// Fill input with too much data for the Deflate Block
		for (unsigned int i = 0; i < DeflateBlock::MAX_BYTES + 1; ++i)
		{
			input.push_back('A');
		}

		EXPECT_THROW(testDeflateBlock(input), std::logic_error);
	}
}
