#include "stdafx.h"
#include "BFInt.h"
#include <climits>
#include <random>


BFInt::BFInt() : positive(true)
{
	zerochars(1);
	size = 1;
}
BFInt::BFInt(const int in) : size(getsizeofint(in))
{
//	zerochars();
	*this = in;
}
BFInt::BFInt(const unsigned long long in) : size(getsizeofull(in))
{
//	zerochars();
	*this = in;
}
BFInt::BFInt(const BFInt &in) : positive(in.positive), size(in.size)
{
	copychars(in.bigint, in.size);
}
BFInt::BFInt(const BFInt &&in) : positive(in.positive), size(in.size)
{
	copychars(in.bigint, in.size);
}
void BFInt::copychars(const unsigned char in[], unsigned int in_size)
{
	if (in == NULL || in_size > arraysize)
		return;

	// if these are wrong, do nothing
	// if the in[] pointer is the same array as this->bigint[] then data may be lost.

	for (unsigned int i = 0; i < in_size; i++)
		bigint[i] = in[i];
	size = (size > arraysize) ? arraysize : size;
	for (unsigned int i = in_size; i < size; i++) // if in_size is smaller, pad with zeroes
		bigint[i] = 0x00;
	size = in_size;
}
void BFInt::zerochars(int in_size)
{
	in_size = (in_size > arraysize || in_size <= 0) ? arraysize : in_size;
	for (int i = 0; i < in_size; i++)
		bigint[i] = 0x00;
	size = in_size;
}
void BFInt::zerochars()
{
	zerochars(0);
	size = 1;
}
BFInt &BFInt::operator =(const BFInt &rhs)
{
	if (this != &rhs)
	{
		positive = rhs.positive;
		copychars(rhs.bigint, rhs.size);
	}
	return *this;
}
BFInt &BFInt::operator =(const BFInt &&rhs)
{
	positive = rhs.positive;
	copychars(rhs.bigint, rhs.size);
	return *this;
}
BFInt &BFInt::operator =(int rhs)
{
	if (rhs == 0)
	{
		positive = true;
		zerochars(1);
		return *this;
	}
	positive = (rhs >= 0);
	rhs = (rhs < 0) ? -rhs : rhs; // abs
	zerochars(getsizeofint(rhs));
	for (unsigned int i = 0; i < size; i++)
		bigint[i] = (unsigned char)(rhs >> i * 8);
	return *this;
}
BFInt &BFInt::operator =(const unsigned long long rhs)
{
	positive = true;

	if (rhs == 0ull)
	{
		zerochars(1);
		return *this;
	}

	zerochars(getsizeofull(rhs));
	for (unsigned int i = 0; i < size; i++)
		bigint[i] = (unsigned char)(rhs >> i * 8);
	return *this;
}
bool operator ==(const BFInt &lhs, const BFInt &rhs)
{
	if ((lhs.positive != rhs.positive) || (lhs.size != rhs.size))
		return false;
	for (unsigned int i = 0; i < lhs.size; i++) {
		if (lhs.bigint[i] != rhs.bigint[i])
			return false;
	}
	return true;
}
bool operator ==(const BFInt &lhs, const int rhs) {
	BFInt in(rhs);
	return lhs == in;
}
bool operator !=(const BFInt &lhs, const BFInt &rhs)
{
	return !(lhs == rhs);
}
bool operator >=(const BFInt &lhs, const BFInt &rhs)
{
	return !(lhs < rhs);
}
bool operator <(const BFInt &lhs, const BFInt &rhs)
{
	return (rhs > lhs);
}
bool operator <=(const BFInt &lhs, const BFInt &rhs)
{
	return !(lhs > rhs);
}
BFInt &BFInt::operator +=(const BFInt &rhs)
{
	BFInt temp = *this;
	if (positive != rhs.positive)
	{ // -a + b = -(a - b) = b - a or a + (-b) = a - b = -(b - a)
		if (abs(*this) > abs(rhs)) // -(a - b) or a - b
		{
			size = sub(temp.bigint, temp.size, rhs.bigint, rhs.size, bigint);
		}
		else
		{
			size = sub(rhs.bigint, rhs.size, temp.bigint, temp.size, bigint);
			positive = !positive;
		}
	}
	else
	{ // -a + (-b) = - a - b = -(a + b) or a + b
		size = add(temp.bigint, temp.size, rhs.bigint, rhs.size, bigint);
	}
	return *this;
}
BFInt operator +(const BFInt &lhs, const BFInt &rhs)
{
	return BFInt(lhs) += rhs;
}
BFInt &BFInt::operator -=(const BFInt &rhs)
{
	if (*this == rhs) {
		zerochars(1);
		positive = true;
		return *this;
	}

	BFInt temp = *this;
	if (temp.positive == rhs.positive) // a - b
	{
		if (abs(temp) > abs(rhs))
		{
			size = sub(temp.bigint, temp.size, rhs.bigint, rhs.size, bigint);
			return *this;
		}
		else
		{
			size = sub(rhs.bigint, rhs.size, temp.bigint, temp.size, bigint);
			positive = !positive;
			return *this;
		}
	}
	else  	// -a - b = -(a + b) or a - (-b) = a + b
	{
		size = add(temp.bigint, temp.size, rhs.bigint, rhs.size, bigint);
		return *this;
	}
	return *this;
}
BFInt operator -(const BFInt &op)
{
	BFInt temp(op);
	temp.positive = !temp.positive;
	return temp;
}
BFInt operator -(const BFInt &lhs, const BFInt &rhs)
{
	return BFInt(lhs) -= rhs;

}
BFInt &BFInt::operator *=(const BFInt &rhs)
{
	BFInt temp = *this;
	size = temp.size + rhs.size;
	
	if (size > arraysize)
		std::cout << "Multiplication overflow: size = " << size << std::endl;
	size = (size > arraysize) ? arraysize : size;
	zerochars(size); // *this is overwritten with the answer

	if (temp.size <= 4 && rhs.size <= 4)
	{
		unsigned long long int a = 0ull, b = 0ull;
		for (unsigned int i = 0; i < 4; i++)
		{
			if (i < temp.size)
				a |= ((unsigned long long) temp.bigint[i] << i * 8);
			if (i < rhs.size)
				b |= ((unsigned long long) rhs.bigint[i] << i * 8);
		}
		*this = a * b;
	}
	else
	{
		mul(temp.bigint, temp.size, rhs.bigint, rhs.size, bigint);
	}
	positive = !(temp.positive != rhs.positive);
	shrink();
	return *this;
}
BFInt operator *(const BFInt &lhs, const BFInt &rhs)
{
	return BFInt(lhs) *= rhs;
}
BFInt &BFInt::operator /= (const BFInt &rhs)
{
	if (abs(rhs) > abs(*this))
	{
		*this = 0;
		return *this;
	}
	if (abs(*this) == abs(rhs)) {
		*this = (positive != rhs.positive) ? -1 : 1;
		return *this;
	}


	// Trivial: fill up uints with bytes and do division
	// 32 bit

	if (size <= 4)
	{
		unsigned long long N = 0, D = 0;
		for (unsigned int i = 0; i < rhs.size; i++)
			D |= ((unsigned long long) rhs.bigint[i] << i*8);
		for (unsigned int i = 0; i < size; i++)
			N |= ((unsigned long long) bigint[i] << i*8);
		unsigned long long Q = N / D;
		*this = Q;
		positive = !(positive != rhs.positive);
		return *this;
	}

	//We'll use Newton for this

	unsigned int precision = countbits() + 16;
	unsigned int maxbits = (arraysize * 4);
	precision = (precision > maxbits) ? maxbits : precision;

	*this = (*this * inverse(rhs, precision) >> (precision));

//	if (bigint[0] >= 0x80u) {
//		*this >>= 8;
//		*this += 1;
//	} else *this >>= 8;

	shrink();
	positive = !(positive != rhs.positive);
	return *this;
}
BFInt operator /(const BFInt &lhs, const BFInt &rhs)
{
	return BFInt(lhs) /= rhs;
}
BFInt &BFInt::operator %=(const BFInt &rhs)
{
//	std::cout << *this << " / " << rhs << " = " << *this / rhs << std::endl;
	*this -= (*this / rhs) * rhs;
//	if (!positive)
//		*this += rhs;
	return *this;
}
BFInt operator %(const BFInt &lhs, const BFInt &rhs)
{
	return BFInt(lhs) %= rhs;
}
std::ostream &operator <<(std::ostream &out, const BFInt &bfintout)
{
	unsigned char c;
	if (!bfintout.positive)
		 out << "-";
	else out << " ";
	int k = (bfintout.size * 2) % 8;
	int j = 1;

	while (k % 8 != 0 && bfintout.size > 8)
	{
		out << "00";
		j += 2;
		k += 2;
	}

	for (int i = (int) ((bfintout.size - 1) * 2 + 1); i >= 0; i--)
	{
		c = bfintout.bigint[i / 2];
		if (i % 2 == 1)
			out << (unsigned int)((c & 0xF0) >> 4);
		else
			out << (unsigned int)(c & 0x0F);
		j++;
		if (j % 73 == 0 && i != 0)
		{
			out << std::endl << " ";
			j += 1;
		}
		if (i % 8 == 0) {
			out << ' ';
			j++;
		}
		if (j % 73 == 0 && i != 0)
		{
			out << std::endl << " ";
			j += 1;
		}
	}
	return out;
}
BFInt operator <<(const BFInt &lhs, unsigned int rhs)
{
	return BFInt(lhs) <<= rhs;
}
BFInt &BFInt::operator <<=(unsigned int rhs)
{
	if (rhs == 0)
		return *this;
	if (rhs > (arraysize * 8)) // 8 bits per byte
		return *this = 0;
	int bytes = (rhs >> 3); // byteshifts
	int bits = rhs & 0b111; // 8 bits per byte

	if (bytes > 0) {
		size += bytes;
		for (int i = size - 1; i >= 0; i--)
		{
			if ((i - bytes) < 0)
				bigint[i] = 0x00;
			else {
				bigint[i] = bigint[i - bytes];
				bigint[i - bytes] = 0x00;
			}
		}
	}
	if (bits > 0)
	{
		unsigned char carry = bigint[size - 1] >> (8 - bits);
		unsigned char mask = ~(0xFF << bits);

		for (int i = size - 1; i >= 0; i--)
		{
			bigint[i] = (i == 0) ? bigint[i] << bits : bigint[i] << bits | bigint[i - 1] >> (8 - bits);
		}
		if (carry != 0 && size < arraysize) {
			bigint[size] = carry;
			size++;
		}
	}
	shrink();
	return *this;
}
BFInt operator >>(const BFInt &lhs, unsigned int rhs)
{
	return BFInt(lhs) >>= rhs;
}
BFInt &BFInt::operator >>=(unsigned int rhs)
{
	if (rhs == 0)
		return *this;
	if (rhs > (arraysize * 8)) // 8 bits per byte
		return *this = 0;
	int bytes = rhs >> 3; // byteshifts
	int bits = rhs & 0b111;
	if (bytes > 0)
	{
		for (unsigned int i = 0; i < size; i++)
		{
			if ((i + bytes) < size) {
				bigint[i] = bigint[i + bytes];
				bigint[i + bytes] = 0x00;
			}
			else
				bigint[i] = 0x00;
		}
	}
	if (bits > 0)
	{
		unsigned char carry = 0u;
		unsigned char mask = ~(0xFF >> bits);

		for (unsigned int i = 0 ; i < size; i++)
			bigint[i] = (i == (size - 1)) ? bigint[i] >> bits :	bigint[i + 1] << (8 - bits) | bigint[i] >> bits;
	}
	shrink();
	return *this;
}
BFInt operator &(const BFInt &lhs, const BFInt &rhs)
{
	return BFInt(lhs) &= rhs;
}
BFInt &BFInt::operator &=(const BFInt &rhs)
{
	unsigned char a, b;
	unsigned int newsize = (size > rhs.size) ? size : rhs.size;
	BFInt temp(*this);
	zerochars(newsize);

	for (unsigned int i = 0; i < newsize; i++)
	{
		a = (i < size) ? temp.bigint[i] : 0x00u;
		b = (i < rhs.size) ? rhs.bigint[i] : 0x00u;
		bigint[i] = a & b;
	}
	return *this;
}
BFInt operator |(const BFInt &lhs, const BFInt &rhs)
{
	return BFInt(lhs) |= rhs;
}
BFInt &BFInt::operator |=(const BFInt &rhs)
{
	unsigned char a, b;
	unsigned int newsize = (size > rhs.size) ? size : rhs.size;
	BFInt temp(*this);
	zerochars(newsize);

	for (unsigned int i = 0; i < newsize; i++)
	{
		a = (i < size) ? temp.bigint[i] : 0x00u;
		b = (i < rhs.size) ? rhs.bigint[i] : 0x00u;
		bigint[i] = a | b;
	}
	return *this;
}
BFInt operator ^(const BFInt &lhs, const BFInt &rhs)
{
	return BFInt(lhs) ^= rhs;
}
BFInt &BFInt::operator ^=(const BFInt &rhs)
{
	unsigned char a, b;
	unsigned int newsize = (size > rhs.size) ? size : rhs.size;
	BFInt temp(*this);
	zerochars(newsize);

	for (unsigned int i = 0; i < newsize; i++)
	{
		a = (i < size) ? temp.bigint[i] : 0x00u;
		b = (i < rhs.size) ? rhs.bigint[i] : 0x00u;
		bigint[i] = a ^ b;
	}
	return *this;
}
BFInt &BFInt::operator ~()
{
	for (unsigned int i = 0; i < size; i++)
		bigint[i] = ~bigint[i];
	return *this;
}
BFInt &BFInt::operator ++()
{
	return *this += BFInt(1);
}
BFInt BFInt::operator ++(int i)
{
	BFInt tmp(*this);
	operator++();
	return tmp;
}
bool BFInt::isPrime()
{
	if (!positive)
		return false;
	if (size <= 8)
	{
		unsigned long long p = 0ull;
		for (unsigned int j = 0; j < size; j++)
			p |= (unsigned long long) bigint[j] << (j << 3);
		if (p < 2) return false;
		if (p == 2) return true;
		if (p == 3) return true;
		if (p % 2 == 0) return false;
		if (p % 3 == 0) return false;
		unsigned long long i = 6ull;
		unsigned long long q = (unsigned long long) ceil(sqrt(p));

		while (i - 1 <= q)
		{
			if (p % (i - 1) == 0 || p % (i + 1) == 0)
				return false;
			i += 6;
		}
	}
	else {
		BFInt i = 3, sqr = sqrt(*this), rem = 0;
		if (sqr * sqr < *this)
			++sqr;
		if (*this % 2 == 0)	return false;
		if (*this % 3 == 0) return false;

		for (i = 6; i <= sqr; i+=6) {
//			rem = *this % i;
//			std::cout << *this << " % " << i << " = " << rem << std::endl;
			if (*this % (i - 1) == 0)
				return false;
			if (*this % (i + 1) == 0)
				return false;
		}
	}
	return true;
}
BFInt BFInt::abs(const BFInt &a)
{
	BFInt answer(a);
	answer.positive = true;
	return answer;
}
void BFInt::randomfill(unsigned int in_size)
{
	in_size = (in_size > BFInt::arraysize) ? BFInt::arraysize : in_size;
	std::random_device random_device;
	std::mt19937 engine{ random_device() };
	std::uniform_int_distribution<> dist(0, 255);
	
	
	for (unsigned int i = 0; i < in_size; i++)					bigint[i] = (unsigned char) dist(engine);
	for (unsigned int i = in_size; i < BFInt::arraysize; i++)	bigint[i] = 0x00;
	size = in_size;
}
BFInt sqrt(const BFInt &a)
{
//	using namespace std;
	int precision = 32; // bits

	BFInt x0(1), x1(1);
	x1 <<= ((a.countbits() + precision) >> 1);
	BFInt target = a << precision;

	do {
		x0 = x1;
		x1 = (x0 + (target / x0)) >> 1;
//		cout << x1 << " =  " << (x0 + (target / x0) >> 1) << endl;
	} while (x0 != x1);

	x1 >>= (precision / 2);
	return x1;
}
BFInt karatsuba(const BFInt &x1, const BFInt &x2)
{
	using namespace std;
	
	if (x1.size <= 4 && x2.size <= 4) // trivial case;
	{
		unsigned long long int a = 0ull, b = 0ull;
		for (unsigned int i = 0; i < 4; i++)
		{
			if (i < x1.size)
				a |= ((unsigned long long) x1.bigint[i] << i * 8);
			if (i < x2.size)
				b |= ((unsigned long long) x2.bigint[i] << i * 8);
		}
		BFInt answer(a * b);
		answer.positive = !(x1.positive != x2.positive);
		return answer;
	}

	int size = (x1.size > x2.size) ? x1.size : x2.size;
	if (size % 2 != 0)
		size++;
	size >>= 1;
//	BFInt answer(0), storage(0);
//	mul(&x1.bigint[size], x1.size - size, &x2.bigint[size], x2.size - size, &answer.bigint[size *2]);	// ac
//	mul(&x1.bigint[0],    size,           &x2.bigint[0],    size,           &answer.bigint[0]);			// bd
//	return answer;

	BFInt a(0), b(0), c(0), d(0);
	BFInt z0(0), z1(0), z2(0);
	a.copychars(&x1.bigint[size], x1.size - size);
	b.copychars(&x1.bigint[0],size);
	c.copychars(&x2.bigint[size], x2.size - size);
	d.copychars(&x2.bigint[0], size);

//	cout << "x1 = " << x1 << endl;
//	cout << "x2 = " << x2 << endl;
//	cout << "a  = " << a << endl;
//	cout << "b  = " << b << endl;
//	cout << "c  = " << c << endl;
//	cout << "d  = " << d << endl;
	z0 = a * c;
	z2 = b * d;
	a += b; // a + b
	c += d; // c + d
	a *= c; //(a + b)(c + d)
	a -= z0;
	a -= z2;
	z0 <<= (size << 4);
	a <<= (size << 3);
	z0 += a;
	z0 += z2; 
	return z0;
}
bool operator >(const BFInt &lhs, const BFInt &rhs)
{
	if (lhs.positive != rhs.positive)
		return lhs.positive;
	if (lhs.size > rhs.size)
		return lhs.positive;
	if (lhs.size < rhs.size)
		return !lhs.positive;

	// check if left hand side is larger, starting with most significant byte
	unsigned char a, b;
	for (int i = rhs.size - 1; i >= 0;  i--)
	{
		a = lhs.bigint[i];
		b = rhs.bigint[i];
		if (a > b) return lhs.positive;
		if (a < b) return !lhs.positive;
		// if equal then next iteration
	}
	return false; // if all numbers are equal
}
BFInt BFInt::toBCD()
{
	BFInt r = 10, temp = *this, result;
	unsigned int newsize = 0;
	result.zerochars();

	while (temp >= 1ull)
	{
//		std::cout << temp << "%" << r << "=" << temp % r << std::endl;
		newsize ++;
		result <<= 8;
		result += temp % r;
		temp /= r;
	}
	result.size = (newsize == 0) ? 1 : newsize;
	result.reverse();
	for (unsigned int i = 0; i < result.size - 1; i++) {
		result.bigint[i] = result.bigint[i * 2] | result.bigint[i * 2 + 1] << 4;
		if (i > 0)
			result.bigint[i * 2] = result.bigint[i * 2 + 1] = 0x00u;
		else
			result.bigint[i * 2 + 1] = 0x00u;
	}
	result.shrink();
	return result;
}
void BFInt::shrink()
{
	for (int i = size - 1; i >= 0; i--)
		if (bigint[i])
		{
			size = i + 1;
			return;
		}
	size = 1;
	return;
}
void BFInt::reverse()
{
	unsigned char c;
	int upindex = size - 1, downindex = 0;

	while (upindex - downindex > 0)
	{
		// ((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b));
		c = bigint[upindex];
		bigint[upindex] = bigint[downindex];
		bigint[downindex] = c;
		upindex--;
		downindex++;
	}
}
unsigned int BFInt::countbits() const
{
	unsigned int bits = 0, r = 0;

	r = bigint[size - 1];
	if (!r)
		return 0;
	while (r >>= 1)
		bits++;
	bits += ((size - 1) << 3);
	return bits;
}
void         mul(const unsigned char *lhs, unsigned int lhs_size, const unsigned char *rhs, unsigned int rhs_size, unsigned char *answer)
{
	if (lhs == nullptr || rhs == nullptr || answer == nullptr)
		return;
	unsigned int carry, i, j;
	for (i = 0; i < rhs_size; i++)
	{
		carry = 0;
		for (j = 0; j < lhs_size; j++)
		{
			carry += (rhs[i] * lhs[j]) + answer[i + j];
			answer[i + j] = (carry & 0xFF); // carry % 0x100
			carry >>= 8; // carry /= 0x100
		}
		answer[i + j] = (carry & 0xFF);
		carry >>= 8;
	}
//	answer[i + j] = carry & 0xFF;
}
unsigned int add(const unsigned char *lhs, unsigned int lhs_size, const unsigned char *rhs, unsigned int rhs_size, unsigned char *answer)
{
	// answer = |lhs| + |rhs|
	if (lhs == nullptr || rhs == nullptr || answer == nullptr)
		return 0;
	unsigned int carry = 0;
	unsigned int newsize = (lhs_size > rhs_size) ? lhs_size : rhs_size;

	for (unsigned int i = 0; i < newsize; i++)
	{
		carry += (i < lhs_size) ? lhs[i] : 0x00;
		carry += (i < rhs_size) ? rhs[i] : 0x00;
		answer[i] = carry & 0xFF;
		carry >>= 8;
	}
	if (carry != 0)
	{
		answer[newsize] = carry;
		newsize++;
	}
	return newsize;
}
unsigned int sub(const unsigned char *lhs, unsigned int lhs_size, const unsigned char *rhs, unsigned int rhs_size, unsigned char *answer)
{
	// answer = ||lhs| - |rhs||,  |lhs| > |rhs|
	if (lhs == nullptr || rhs == nullptr || answer == nullptr)
		return 0;
	unsigned char carry = 0u, temp = 0u;

	for (unsigned int i = 0; i < lhs_size; i++)
	{
		temp = (i >= rhs_size) ? 0x00 : rhs[i];
		if (lhs[i] < (carry + temp))
		{
			answer[i] = 0x100u + lhs[i] - (carry + temp);
			carry = 1u;
		}
		else
		{
			answer[i] = lhs[i] - (carry + temp);
			carry = 0u;
		}
	}
	unsigned int newsize = lhs_size;
	while (!answer[newsize - 1] && newsize > 1)
		newsize--;
	return newsize;
}
BFInt inverse(const BFInt &b, unsigned int precision)
{
	using namespace std;

	BFInt x0(1), x1(1), k(1);

	x1 <<= (precision - b.countbits());
	k <<= (precision + 1); // k = 2^(precision + 1)

	do {
		x0 = x1;
		x1 = x0 * (k - b * x0) >> precision;
//		cout << "x1 = " << endl << (x0 * (k - b * x0) >> precision) << endl;
	} while (x0 != x1);
	return x1;
}
BFInt::~BFInt()
{
}
unsigned int getsizeofint(int in)
{
	     if (in & 0xFF000000) return 4u;
	else if (in & 0x00FF0000) return 3u;
	else if (in & 0x0000FF00) return 2u;
	else return 1u;
}
unsigned int getsizeofull(unsigned long long in)
{
	     if (in & 0xFF00000000000000) return 8u;
	else if (in & 0x00FF000000000000) return 7u;
	else if (in & 0x0000FF0000000000) return 6u;
	else if (in & 0x000000FF00000000) return 5u;
	else if (in & 0x00000000FF000000) return 4u;
	else if (in & 0x0000000000FF0000) return 3u;
	else if (in & 0x000000000000FF00) return 2u;
	else return 1u;
}
