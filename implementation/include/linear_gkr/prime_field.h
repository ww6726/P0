#pragma once

// #ifndef __prime_field_small
// #define __prime_field_small

#include <cassert>
#include <string>
#include <gmp.h>
#include <gmpxx.h>
#include <vector>
typedef unsigned long long uint64;

namespace prime_field
{
	void init(std::string, int);
	void init_random();
	
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
	};
	field_element random();

}