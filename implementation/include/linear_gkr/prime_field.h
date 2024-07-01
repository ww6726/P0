#pragma once

// #ifndef __prime_field_small
// #define __prime_field_small

#include <cassert>
#include <string>
#include <gmp.h>
#include <gmpxx.h>
#include <vector>

typedef unsigned long long uint64;
struct uint128 {
    uint64_t hi;
    uint64_t low;

    uint128() : hi(0), low(0) {}
    uint128(uint64_t high, uint64_t low) : hi(high), low(low) {}
};
namespace prime_field
{

	void init(std::string, int);
	void init_random();
	void print_128(uint128 input);
	uint128 operator <<(const uint128& lhs, int shift);
 	uint128 operator >>(const uint128& lhs, int shift);
	uint128 operator %(const uint128& lhs, const uint128& rhs);

	uint64 to64b(const uint128& input);
	uint128 to128b(const uint64& input);
	uint128 operator - (const uint128& a, const uint128& b);
	uint128 operator + (const uint128& a, const uint128& b);
	uint128 operator * (const uint128& a, const uint128& b);


	bool operator>(const uint128& lhs, const uint128& rhs);
	bool operator>(const uint128& lhs, const uint128& rhs);
	bool operator==(const uint128& lhs, const uint128& rhs);
 	bool operator<=(const uint128& lhs, const uint128& rhs);
	bool operator>=(const uint128& lhs, const uint128& rhs);



	uint64_t operator % (uint128 number, uint64_t divisor);
	uint128 mult(uint64_t a, uint64_t b);
	uint64 mod_add(uint64 a, uint64 b, uint64 mod);
	extern uint64 mod;


	class u64b
	{
	private:
	public:
	

		uint64 value_64;
		u64b();
		u64b(const int &b);
		u64b(const unsigned long long &b);
		u64b operator + (const u64b &b) const;
		u64b operator * (const u64b &b) const;
		u64b operator / (const u64b &b) const;
		u64b operator - (const u64b &b) const;
		u64b operator % (const uint64 &b) const;

		bool operator != (const u64b &b) const;
		bool operator > (const u64b &b) const;
		bool operator >= (const u64b &b) const;
		bool operator < (const u64b &b) const;
		bool operator == (const u64b &b) const;
	};

	class field_element
	{
	private:
	public:
		u64b value;
		u64b hi;
		u64b low;
		mpz_class to_gmp_class();

		field_element();
		field_element(const int);
		field_element(const unsigned long long);
		field_element(std::string);
		field_element operator + (const field_element &b) const;
		field_element operator * (const field_element &b) const;
		field_element operator / (const field_element &b) const;
		field_element operator - (const field_element &b) const;
		bool operator == (const field_element &b) const;
		bool operator >= (const field_element &b) const;
		bool operator <= (const field_element &b) const;
		bool operator > (const field_element &b) const;
		bool operator < (const field_element &b) const;

		field_element mul_non_mod(const field_element &b) const;
		char* to_string();
		int bitLen();
		bool operator != (const field_element &b) const;
		std::vector<bool> bit_stream();
		void print();
	};
	field_element random();

}