#include "linear_gkr/prime_field.h"
#include <cmath>
#include <climits>
#include <ctime>
#include <iostream>
using namespace std;
typedef unsigned long long uint64;

#define MASK 4294967295 //2^32-1
#define PRIME 18446744069414584321 //2^61-1

#define MOD_HI 4294967295
#define MOD_LOW 1
uint128 mod_;
uint128 mod2_;

namespace prime_field
{	

	string toBits(uint64_t value) {
		if (value == 0) {
			return "0";
		}

		std::string binaryString;
		bool leadingZero = true;
		
		// Iterate over each bit, starting from the most significant bit (MSB)
		for (int i = 63; i >= 0; --i) {
			// Check if the ith bit is set
			if (value & (1ULL << i)) {
				leadingZero = false; // We've hit the first 1 bit, stop skipping zeros
				binaryString += '1';
			} else if (!leadingZero) {
				binaryString += '0';
			}
		}

		return binaryString;
	}
	uint64 abs(uint64 x){
		if(x>((18446744069414584321/2))){
			return mod - x;
		}
		return x;
	}
	uint64 myMod(uint64 x)
	{
		return (x) % mod;
	} 

	u64b::u64b()
	{
		value_64 = 0;
	}
	u64b::u64b(const int &b)
	{
		value_64 = b;
	}
	u64b::u64b(const uint64 &b)
	{
		value_64 = b;
	}
	void print_128(uint128 input){
		cout<<input.hi<<" "<<input.low<<endl;
	}
 	uint128 operator<<(const uint128& lhs, int shift) {
        uint128 result(lhs.hi, lhs.low);
        if (shift >= 64) {
            result.hi = result.low << (shift - 64);
            result.low = 0;
        } else if (shift > 0) {
            result.hi = (result.hi << shift) | (result.low >> (64 - shift));
            result.low <<= shift;
        }
        return result;
    }

    uint128 operator>>(const uint128& lhs, int shift) {
        uint128 result(lhs.hi, lhs.low);
        if (shift >= 64) {
            result.low = result.hi >> (shift - 64);
            result.hi = 0;
        } else if (shift > 0) {
            result.low = (result.low >> shift) | (result.hi << (64 - shift));
            result.hi >>= shift;
        }
        return result;
    }
	uint128 operator%(const uint128& lhs, const uint128& rhs) {		

		if (rhs.hi == 0 && rhs.low == 0) {
            throw std::invalid_argument("Modulo by zero");
        }
        
        uint128 dividend = lhs;
        uint128 divisor = rhs;

        uint128 quotient(0, 0);
        uint128 remainder(0, 0);

        for (int i = 127; i >= 0; --i) {
            remainder = remainder << 1;
            if (dividend.hi & (1ULL << 63)) {
                remainder.low |= 1;
            }
            dividend = dividend << 1;

            if (remainder >= divisor) {
                remainder = remainder - divisor;
                quotient.low |= 1;
            }
        }
        return remainder;
	
	
	}

	
	uint128 operator - (const uint128& a, const uint128& b) {
		
		uint64 result_hi, result_low;
		// Subtract the low parts
		
		if (a.low < b.low) {
			// Borrow from the high part
			result_low = a.low - b.low + (1ULL << 32);
			result_hi = a.hi - b.hi - 1;
		} else {
			result_low = a.low - b.low;
			result_hi = a.hi - b.hi;
		}
		uint128 result;
		result.hi = result_hi;
		result.low = result_low;
		result = result % mod_;
		return (result);
	}
	uint128 operator + (const uint128& lhs, const uint128& rhs) {
		 uint64_t low = lhs.low + rhs.low;
        uint64_t carry = (low < lhs.low) ? 1 : 0;
        uint64_t hi = lhs.hi + rhs.hi + carry;
		uint128 result(hi,low);
		result = result % mod_;
        return result;
	}

	bool operator<(const uint128& lhs, const uint128& rhs) {
		if (lhs.hi < rhs.hi) {
			return true;
		} else if (lhs.hi > rhs.hi) {
			return false;
		} else {
			// lhs.hi == rhs.hi, compare low parts
			return lhs.low < rhs.low;
		}
	}
	// Define the greater than operator
	bool operator>(const uint128& lhs, const uint128& rhs) {
		if (lhs.hi > rhs.hi) {
			return true;
		} else if (lhs.hi < rhs.hi) {
			return false;
		} else {
			// lhs.hi == rhs.hi, compare low parts
			return lhs.low > rhs.low;
		}
	}

	bool operator==(const uint128& lhs, const uint128& rhs) {
        return (lhs.hi == rhs.hi) && (lhs.low == rhs.low);
    }
	// Define the less than or equal to operator as a non-member function
 	bool operator<=(const uint128& lhs, const uint128& rhs) {
        return (lhs < rhs) || (lhs == rhs);
    }

    // Define the greater than or equal to operator as a non-member function
    bool operator>=(const uint128& lhs, const uint128& rhs) {
        return (lhs > rhs) || (lhs == rhs);
    }

	uint128 to128b(const uint64& input){
		uint128 result;
		result.hi = input >> 32;
		result.low = input & MASK;
		return result;
	}
	uint64 to64b(const uint128& input){
		uint64 result = 0;
		result += input.low;
		result += (input.hi << 32);
		return result;
	}
	uint128 add(uint64_t a, uint64_t b) {
        uint64_t low = a + b;
        uint64_t carry = (low < a) ? 1 : 0;
        return uint128(carry, low);
    }
	u64b u64b::operator + (const u64b &b) const
	{	
		uint128 result = add(value_64,b.value_64);
		uint64 ret = result % PRIME;
	
		return ret;
	}
	u64b u64b::operator - (const u64b &b) const
	{

		uint64 result;
		if (value_64 >= b.value_64) {
			result = (value_64 - b.value_64) % mod;
		} else {
			result = (mod - ((b.value_64 - value_64) % mod)) % mod;
		}
		return result;

		
	}
	uint64 mod_add(uint64 a, uint64 b, uint64 mod) {
		uint64 sum = a + b;
		return (sum >= mod) ? (sum - mod) : sum;
	}

	// Helper function to subtract two numbers modulo mod
	uint64 mod_sub(uint64 a, uint64 b, uint64 mod) {
		return (a >= b) ? (a - b) : (a + mod - b);
	}

	uint128 mult(uint64_t a, uint64_t b) {
		uint128 result;
		
		// Perform the multiplication using __uint128_t type
		__uint128_t full_result = (__uint128_t)a * (__uint128_t)b;
		
		// Split the result into high and low parts
		result.low = (uint64_t)(full_result);
		result.hi = (uint64_t)(full_result >> 64);
		
		return result;
	}	
	uint64_t operator % (uint128 number, uint64_t divisor) {
		// Start with the high part of the 128-bit number
		__uint128_t remainder = number.hi % divisor;
		
		// Combine the remainder with the low part
		remainder = (remainder << 64) | number.low;
		
		// Return the final modulus
		return (uint64_t)(remainder % divisor);
	}
	u64b u64b::operator * (const u64b &b) const
	{	

		uint128 result = mult(value_64,b.value_64);
		uint64 ret = result%PRIME;
		return ret;
		
	}
	u64b u64b::operator % (const uint64 &b) const
	{
		u64b ret;
		ret.value_64 = myMod(value_64);
		return ret;
	}
	bool u64b::operator < (const u64b &b) const
	{
		if(value_64<b.value_64)
		{
			return true;
		}
		else
			return false;
	}
	bool u64b::operator > (const u64b &b) const
	{
		if(value_64 > b.value_64)
			return true;
		if(value_64 <= b.value_64)
			return false;
		
	}
	bool u64b::operator >= (const u64b &b) const
	{
		if(value_64 >= b.value_64)
			return true;
		if(value_64 < b.value_64)
			return false;
		
	}

	bool u64b::operator != (const u64b &b) const
	{
		if(myMod(value_64)%PRIME != myMod(b.value_64)%PRIME)
			return true;
		else
			return false;
		
	}

	uint64 mod;
	bool initialized = false;
	void init(std::string s, int base)
	{
		assert(base == 10);
		initialized = true;
		mod = stoull(s);
		mod_.hi = MOD_HI;
		mod_.low = MOD_LOW;
		mod2_.hi = 0;
		mod2_.low = 18446744069414584321;
	}

	void init_random()
	{
	}
	mpz_class field_element::to_gmp_class()
	{
		mpz_class ret(std::to_string(myMod(value.value_64)%mod));
		return ret;
	}
	field_element::field_element()
	{
		value.value_64 = (uint64)(0);
	}
	field_element::field_element(const int x)
	{
		value.value_64 = (uint64)(x);
	}
	field_element::field_element(const unsigned long long x)
	{
		value.value_64 = (uint64)(myMod(x));
	}
	field_element::field_element(std::string str)
	{
		value.value_64 = strtoull(str.c_str(), nullptr, 10);
	}
	field_element field_element::operator + (const field_element &b) const
	{

		field_element ret;
		ret.value = value + b.value;
		return ret;
	}
	field_element field_element::operator - (const field_element &b) const
	{

		field_element ret;
		ret.value = value - b.value;
		return ret;
	}
	field_element field_element::operator * (const field_element &b) const
	{
		field_element ret;
		ret.value = value * b.value;
		return ret;
	}
	bool field_element::operator == (const field_element &b) const
	{
		return !(*this != b);
	}
	bool field_element::operator >= (const field_element &b) const
	{
		if(value >= b.value){
			return true;
		}return false;
	}
	bool field_element::operator > (const field_element &b) const
	{
		if(value>b.value){
			return true;
		}	
		return false;
	}
	bool field_element::operator <= (const field_element &b) const
	{
		if(value >b.value){
			return false;
		}return true;
	}
	bool field_element::operator < (const field_element &b) const
	{
		if(value <b.value){
			return true;
		}return false;
	}
	bool field_element::operator != (const field_element &b) const
	{
		return (value) != (b.value);
	}
	field_element random()
	{
		field_element ret;
		ret.value = rand()%100;
		ret.value = 32; //change this later
		return ret;
	}

	void field_element::print() {
		cout << this->to_gmp_class() << endl;
	}



}
