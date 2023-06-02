/* AES - Advanced Encryption Standard
  
  source version 1.0, June, 2005

  Copyright (C) 2000-2005 Chris Lomont

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Chris Lomont
  chris@lomont.org

  The AES Standard is maintained by NIST
  http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf

  This legalese is patterned after the zlib compression library
*/

// PATCHED for aes_uint32_t --> 64 bit on OS X platform, not the standard C 32 bit!

#include <QDebug>

// code to implement Advanced Encryption Standard - Rijndael
// speed optimized version
#include "AES.h"
#include <assert.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#ifdef Q_OS_LINUX
#include <malloc.h>
#include <string.h>
#endif

// todo - make faster 128 blocksize version with 128 blocksize hardcoded as necessary

// internally data is stored in the state in order
//   0  1  2  3
//   4  5  6  7  
//   8  8 10 11
//   ...
// up to Nb of these
// NOTE: thus rows and columns are interchanged from the paper

// TODO - test against the attack at http://cr.yp.to/mac/variability1.html and make fixes if necessary

using namespace std;

namespace { // anonymous namespace for local linkage

// tables for inverses, byte sub
aes_byte_t gf2_8_inv[256];
aes_byte_t byte_sub[256];
aes_byte_t inv_byte_sub[256];

// this table needs Nb*(Nr+1)/Nk entries - up to 8*(15)/4 = 60
// todo - remove table, note cycles every 17(?) elements
aes_uint32_t Rcon[60];

// long tables for encryption stuff
aes_uint32_t T0[256];
aes_uint32_t T1[256];
aes_uint32_t T2[256];
aes_uint32_t T3[256];

// long tables for decryption stuff
aes_uint32_t I0[256];
aes_uint32_t I1[256];
aes_uint32_t I2[256];
aes_uint32_t I3[256];

// huge tables - todo - ifdef out
aes_uint32_t T4[256];
aes_uint32_t T5[256];
aes_uint32_t T6[256];
aes_uint32_t T7[256];
aes_uint32_t I4[256];
aes_uint32_t I5[256];
aes_uint32_t I6[256];
aes_uint32_t I7[256];

// have the tables been initialized?
bool tablesInitialized = false;

// define to mult a byte by x mod the proper poly
// todo - move magic numbers out?
#define xmult(a) ((a)<<1) ^ (((a)&128) ? 0x01B : 0)

// make 4 bytes (LSB first) into a 4 byte vector
//#define VEC4(a,b,c,d) (((aes_int32_t)(a)) | (((aes_int32_t)(b))<<8) | (((aes_int32_t)(c))<<16) | (((aes_int32_t)(d))<<24))
#define VEC4(a,b,c,d) ((static_cast<aes_uint32_t>(a)) | ((static_cast<aes_uint32_t>(b))<<8) | ((static_cast<aes_uint32_t>(c))<<16) | ((static_cast<aes_uint32_t>(d))<<24))

// get byte 0 to 3 from word a
#define GetByte(a,n) (static_cast<aes_byte_t>((a) >> (n<<3)))

//	bytes (a,b,c,d) -> (b,c,d,a) so low becomes high
#ifdef Q_OS_MAC
#define OWN_ROTLR
#endif

#ifdef Q_OS_LINUX
#define OWN_ROTLR
#endif

#ifdef OWN_ROTLR
    #ifndef CHAR_BIT
    #define CHAR_BIT 8
    #endif

    static inline aes_uint32_t rotl32 (aes_uint32_t value, unsigned int count) {
        const unsigned int mask = (CHAR_BIT*sizeof(value)-1);
        count &= mask;
        return (value<<count) | (value>>( (-count) & mask ));
    }

    static inline aes_uint32_t rotr32 (aes_uint32_t value, unsigned int count) {
        const unsigned int mask = (CHAR_BIT*sizeof(value)-1);
        count &= mask;
        return (value>>count) | (value<<( (-count) & mask ));
    }
    #define RotByte(a)  rotr32(a, 8)
    #define RotByteL(a) rotl32(a, 8)
#else
    #define RotByte(a) _rotr(a,8)
    #define RotByteL(a) _rotl(a,8)
#endif


// mult 2 elements using gf2_8_poly as a reduction
inline aes_byte_t GF2_8_mult(aes_byte_t a, aes_byte_t b)
{ // todo - make 4x4 table for nibbles, use lookup
    aes_byte_t result = 0;

	// should give 0x57 . 0x13 = 0xFE with poly 0x11B
	// 

	int count = 8;
	while (count--)
		{
		if (b&1)
			result ^= a;
		if (a&128)
			{
			a <<= 1;
			a ^= (0x1B);
			}
		else
			a <<= 1;
		b >>= 1;
		}
	return result;
} // GF2_8_mult

bool CheckLargeTables(bool create)
	{
	unsigned int i;
    aes_byte_t a1,a2,a3,b1,b2,b3,b4,b5;
	for (i = 0; i < 256; i++)
		{
		a1 = byte_sub[i];
		a2 = xmult(a1);
		a3 = a2^a1;

		b5 = inv_byte_sub[i];
		b1 = GF2_8_mult(0x0E,b5);
		b2 = GF2_8_mult(0x09,b5);
		b3 = GF2_8_mult(0x0D,b5);
		b4 = GF2_8_mult(0x0B,b5);

		if (create == true)
			{
			T0[i] = VEC4(a2,a1,a1,a3);
			T1[i] = RotByteL(T0[i]);
			T2[i] = RotByteL(T1[i]);
			T3[i] = RotByteL(T2[i]);

			T4[i] = VEC4(a1,0,0,0); // identity
			T5[i] = RotByteL(T4[i]);
			T6[i] = RotByteL(T5[i]);
			T7[i] = RotByteL(T6[i]);
			
			I0[i] = VEC4(b1,b2,b3,b4);
			I1[i] = RotByteL(I0[i]);
			I2[i] = RotByteL(I1[i]);
			I3[i] = RotByteL(I2[i]);

			I4[i] = VEC4(b5,0,0,0); // identity
			I5[i] = RotByteL(I4[i]);
			I6[i] = RotByteL(I5[i]);
			I7[i] = RotByteL(I6[i]);
			}
		else
			{
			if (T0[i] != VEC4(a2,a1,a1,a3))
				return false;
			if (T1[i] != RotByteL(T0[i]))
				return false;
			if (T2[i] != RotByteL(T1[i]))
				return false;
			if (T3[i] != RotByteL(T2[i]))
				return false;
			if (T4[i] != VEC4(a1,0,0,0))
				return false;
			if (T5[i] != RotByteL(T4[i]))
				return false;
			if (T6[i] != RotByteL(T5[i]))
				return false;
			if (T7[i] != RotByteL(T6[i]))
				return false;
			if (I0[i] != VEC4(b1,b2,b3,b4))
				return false;
			if (I1[i] != RotByte(I0[i]))
				return false;
			if (I2[i] != RotByte(I1[i]))
				return false;
			if (I3[i] != RotByte(I2[i]))
				return false;
			if (I4[i] != VEC4(b5,0,0,0))
				return false;
			if (I5[i] != RotByteL(I4[i]))
				return false;
			if (I6[i] != RotByteL(I5[i]))
				return false;
			if (I7[i] != RotByteL(I6[i]))
				return false;
			}
		}
	return true;
	} // CheckLargeTables

// some functions to create/verify table integrity
bool CheckInverses(bool create)
	{
	// we'll brute force the inverse table
	assert(GF2_8_mult(0x57,0x13) == 0xFE); // test these first
	assert(GF2_8_mult(0x01,0x01) == 0x01);
	assert(GF2_8_mult(0xFF,0x55) == 0xF8);


	unsigned int a,b; // need int here to prevent wraps in loop
	if (create == true)
		gf2_8_inv[0] = 0;
	else if (gf2_8_inv[0] != 0)
		return false;
	for (a = 1; a <= 255; a++)
		{
		b = 1;
        while (GF2_8_mult(static_cast<aes_byte_t>(a),static_cast<aes_byte_t>(b)) != 1)
			b++;

		if (create == true)
            gf2_8_inv[a] = static_cast<aes_byte_t>(b);
		else if (gf2_8_inv[a] != b)
			return false;
		}
	return true;
	} // CheckInverses

aes_byte_t BitSum(aes_byte_t byte)
	{ // return the sum of bits mod 2
	byte = (byte>>4)^(byte&15);
	byte = (byte>>2)^(byte&3);
	return (byte>>1)^(byte&1);
	} // BitSum

bool CheckByteSub(bool create)
	{
	if (CheckInverses(create) == false)
		return false; // we cannot do this without inverses
	
	unsigned int x,y; // need ints here to prevent wrap in loop
	for (x = 0; x <= 255; x++)
		{
		y = gf2_8_inv[x]; // inverse to start with
		
		// affine transform
		y = BitSum(y&0xF1) | (BitSum(y&0xE3)<<1) | (BitSum(y&0xC7)<<2) | (BitSum(y&0x8F)<<3) |
			(BitSum(y&0x1F)<<4) | (BitSum(y&0x3E)<<5) | (BitSum(y&0x7C)<<6) | (BitSum(y&0xF8)<<7);
		y = y ^ 0x63;
		if (create == true)
			byte_sub[x] = y;
		else if (byte_sub[x] != y)
			return false;
		}
	return true;
	} // CheckByteSub

bool CheckInvByteSub(bool create)
	{
	if (CheckInverses(create) == false)
		return false; // we cannot do this without inverses
	if (CheckByteSub(create) == false)
		return false; // we cannot do this without byte_sub
	
	unsigned int x,y; // need ints here to prevent wrap in loop
	for (x = 0; x <= 255; x++)
		{
		// we brute force it...
		y = 0;
		while (byte_sub[y] != x)
			y++;
		if (create == true)
			inv_byte_sub[x] = y;
		else if (inv_byte_sub[x] != y)
			return false;
		}
	return true;
	} // CheckInvByteSub

bool CheckRcon(bool create)
	{
    aes_byte_t Ri = 1; // start here

	if (create == true)
		Rcon[0] = 0;
	else if (Rcon[0] != 0)
        return false; // todo - this is unused still check?
    for (int i = 1; i < static_cast<int>(sizeof(Rcon)/sizeof(Rcon[0])-1); i++)
		{
		if (create == true)
			Rcon[i] = Ri;
		else if (Rcon[i] != Ri)
			return false;
		Ri = GF2_8_mult(Ri,0x02); // multiply by x - todo replace with xmult
		}
	return true;
	} // CheckRCon

// key adding for 4,6,8 column cases
#define AddRoundKey4(dest,src)	\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;

#define AddRoundKey6(dest,src)	\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;

#define AddRoundKey8(dest,src)	\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;\
	*dest++ = *r_ptr++ ^ *src++;

// this define computes one of the round vectors 
#define compute_one(dest,src2,j,C1,C2,C3,Nb)	*(dest+j) = \
	T0[GetByte(src2[j],0)]^T1[GetByte(src2[((j+C1+Nb)%Nb)],1)]^ \
	T2[GetByte(src2[((j+C2+Nb)%Nb)],2)]^T3[GetByte(src2[((j+C3+Nb)%Nb)],3)] \
	^*r_ptr++

// single table version, slower
#define compute_one_small(dest,src2,j,C1,C2,C3,Nb)	*(dest+j) = *r_ptr++^\
	T0[GetByte(src2[j],0)]^\
	RotByteL(T0[GetByte(src2[((j+C1+Nb)%Nb)],1)]^\
	RotByteL(T0[GetByte(src2[((j+C2+Nb)%Nb)],2)]^\
	RotByteL(T0[GetByte(src2[((j+C3+Nb)%Nb)],3)])))

#define Round4(d,s)	\
		compute_one(d,s,0,1,2,3,4); \
		compute_one(d,s,1,1,2,3,4);	\
		compute_one(d,s,2,1,2,3,4);	\
		compute_one(d,s,3,1,2,3,4); 

#define Round6(d,s) \
		compute_one(d,s,0,1,2,3,6); \
		compute_one(d,s,1,1,2,3,6); \
		compute_one(d,s,2,1,2,3,6); \
		compute_one(d,s,3,1,2,3,6); \
		compute_one(d,s,4,1,2,3,6); \
		compute_one(d,s,5,1,2,3,6);

#define Round8(d,s) \
		compute_one(d,s,0,1,3,4,8); \
		compute_one(d,s,1,1,3,4,8); \
		compute_one(d,s,2,1,3,4,8); \
		compute_one(d,s,3,1,3,4,8); \
		compute_one(d,s,4,1,3,4,8); \
		compute_one(d,s,5,1,3,4,8); \
		compute_one(d,s,6,1,3,4,8); \
		compute_one(d,s,7,1,3,4,8); 

#define compute_one_inv(dest,src2,j,C1,C2,C3,Nb)	*(dest+j) = \
	I0[GetByte(src2[j],0)]^I1[GetByte(src2[((j-C1+Nb)%Nb)],1)]^ \
	I2[GetByte(src2[((j-C2+Nb)%Nb)],2)]^I3[GetByte(src2[((j-C3+Nb)%Nb)],3)] \
	^*r_ptr++

#define InvRound4(d,s)	\
		compute_one_inv(d,s,0,1,2,3,4); \
		compute_one_inv(d,s,1,1,2,3,4);	\
		compute_one_inv(d,s,2,1,2,3,4);	\
		compute_one_inv(d,s,3,1,2,3,4); 

#define InvRound6(d,s)	\
		compute_one_inv(d,s,0,1,2,3,6); \
		compute_one_inv(d,s,1,1,2,3,6);	\
		compute_one_inv(d,s,2,1,2,3,6);	\
		compute_one_inv(d,s,3,1,2,3,6);	\
		compute_one_inv(d,s,4,1,2,3,6);	\
		compute_one_inv(d,s,5,1,2,3,6); 

#define InvRound8(d,s)	\
		compute_one_inv(d,s,0,1,3,4,8); \
		compute_one_inv(d,s,1,1,3,4,8);	\
		compute_one_inv(d,s,2,1,3,4,8);	\
		compute_one_inv(d,s,3,1,3,4,8);	\
		compute_one_inv(d,s,4,1,3,4,8);	\
		compute_one_inv(d,s,5,1,3,4,8);	\
		compute_one_inv(d,s,6,1,3,4,8);	\
		compute_one_inv(d,s,7,1,3,4,8); 

// this define computes one of the final round vectors
#define compute_one_final1(dest,src,j,C1,C2,C3,Nb)  *dest++ = \
	(T3[GetByte(src[j],0)]&0xFF)^\
	(T3[GetByte(src[((j+C1+Nb)%Nb)],1)]&0xFF00)^ \
	(T1[GetByte(src[((j+C2+Nb)%Nb)],2)]&0xFF0000)^ \
	(T1[GetByte(src[((j+C3+Nb)%Nb)],3)]&0xFF000000)^*r_ptr++

// for another 4K tables, we save 3 clock cycles - sick
#define compute_one_final(dest,src,j,C1,C2,C3,Nb)  *dest++ = \
	(T4[GetByte(src[j],0)])^\
	(T5[GetByte(src[((j+C1+Nb)%Nb)],1)])^ \
	(T6[GetByte(src[((j+C2+Nb)%Nb)],2)])^ \
	(T7[GetByte(src[((j+C3+Nb)%Nb)],3)])^*r_ptr++

// final round defines - this one is for case for 4 columns
#define FinalRound4(d,s) compute_one_final(d,s,0,1,2,3,4); \
						compute_one_final(d,s,1,1,2,3,4); \
						compute_one_final(d,s,2,1,2,3,4); \
						compute_one_final(d,s,3,1,2,3,4);

#define FinalRound6(d,s) compute_one_final(d,s,0,1,2,3,6); \
						compute_one_final(d,s,1,1,2,3,6); \
						compute_one_final(d,s,2,1,2,3,6); \
						compute_one_final(d,s,3,1,2,3,6); \
						compute_one_final(d,s,4,1,2,3,6); \
						compute_one_final(d,s,5,1,2,3,6);

#define FinalRound8(d,s) compute_one_final(d,s,0,1,3,4,8); \
						compute_one_final(d,s,1,1,3,4,8); \
						compute_one_final(d,s,2,1,3,4,8); \
						compute_one_final(d,s,3,1,3,4,8); \
						compute_one_final(d,s,4,1,3,4,8); \
						compute_one_final(d,s,5,1,3,4,8); \
						compute_one_final(d,s,6,1,3,4,8); \
						compute_one_final(d,s,7,1,3,4,8);

// inverse cipher stuff
#define compute_one_final_inv(dest,src,j,C1,C2,C3,Nb)  *dest++ = \
	(I4[GetByte(src[j],0)])^\
	(I5[GetByte(src[((j-C1+Nb)%Nb)],1)])^ \
	(I6[GetByte(src[((j-C2+Nb)%Nb)],2)])^ \
	(I7[GetByte(src[((j-C3+Nb)%Nb)],3)])^*r_ptr++

// final round defines - this one is for case for 4 columns
#define InvFinalRound4(d,s) compute_one_final_inv(d,s,0,1,2,3,4); \
						compute_one_final_inv(d,s,1,1,2,3,4); \
						compute_one_final_inv(d,s,2,1,2,3,4); \
						compute_one_final_inv(d,s,3,1,2,3,4);

#define InvFinalRound6(d,s) compute_one_final_inv(d,s,0,1,2,3,6); \
						compute_one_final_inv(d,s,1,1,2,3,6); \
						compute_one_final_inv(d,s,2,1,2,3,6); \
						compute_one_final_inv(d,s,3,1,2,3,6); \
						compute_one_final_inv(d,s,4,1,2,3,6); \
						compute_one_final_inv(d,s,5,1,2,3,6);

#define InvFinalRound8(d,s) compute_one_final_inv(d,s,0,1,3,4,8); \
						compute_one_final_inv(d,s,1,1,3,4,8); \
						compute_one_final_inv(d,s,2,1,3,4,8); \
						compute_one_final_inv(d,s,3,1,3,4,8); \
						compute_one_final_inv(d,s,4,1,3,4,8); \
						compute_one_final_inv(d,s,5,1,3,4,8); \
						compute_one_final_inv(d,s,6,1,3,4,8); \
						compute_one_final_inv(d,s,7,1,3,4,8); 

aes_uint32_t SubByte(aes_uint32_t data)
	{ // does the SBox on this 4 byte data
	unsigned result = 0;
	result = byte_sub[data>>24];
	result <<= 8;
	result |= byte_sub[(data>>16)&255];
	result <<= 8;
	result |= byte_sub[(data>>8)&255];
	result <<= 8;
	result |= byte_sub[data&255];
	return result;
	} // SubByte


void DumpCharTable(ostream & out, const char * name, const aes_byte_t * table, int length)
	{ // dump the contents of a table to a file
	int pos;
	out << name << endl << hex;
	for (pos = 0; pos < length; pos++)
		{
		out << "0x";
		if (table[pos] < 16)
			out << '0';
		out << static_cast<unsigned int>(table[pos]) << ',';
		if ((pos %16) == 15)
			out << endl;
		}
	out << dec;
	} // DumpCharTable

void DumpLongTable(ostream & out, const char * name, const aes_uint32_t * table, int length)
	{ // dump te contents of a table to a file
	int pos;
	out << name << endl << hex;
	for (pos = 0; pos < length; pos++)
		{
		out << "0x";
		if (table[pos] < 16)
			out << '0';
		if (table[pos] < 16*16)
			out << '0';
		if (table[pos] < 16*16*16)
			out << '0';
		if (table[pos] < 16*16*16*16)
			out << '0';
		if (table[pos] < 16*16*16*16*16)
			out << '0';
		if (table[pos] < 16*16*16*16*16*16)
			out << '0';
		if (table[pos] < 16*16*16*16*16*16*16)
			out << '0';
		out << static_cast<unsigned int>(table[pos]) << ',';
		if ((pos % 8) == 7)
			out << endl;
		}
	out << dec;
	} // DumpCharTable

// return true iff tables are valid. create = true fills them in if not
bool CreateAESTables(bool create, bool create_file)
	{
	bool retval = true;
	if (CheckInverses(create) == false)
		retval = false;
	if (CheckByteSub(create) == false)
		retval = false;
	if (CheckInvByteSub(create) == false)
		retval = false;
	if (CheckRcon(create) == false)
		return false;
	if (CheckLargeTables(create) == false)
		return false;

	if (create_file == true)
		{ // dump tables
		ofstream out;
		out.open("Tables.dat");
		if (out.is_open() == true)
			{
			DumpCharTable(out,"gf2_8_inv", gf2_8_inv, 256);
			out << "\n\n";
			DumpCharTable(out,"byte_sub", byte_sub, 256);
			out << "\n\n";
			DumpCharTable(out,"inv_byte_sub", inv_byte_sub, 256);
			out << "\n\n";
			DumpLongTable(out,"RCon", Rcon, 60);
			out << "\n\n";
			DumpLongTable(out,"T0", T0, 256);
			out << "\n\n";
			DumpLongTable(out,"T1", T1, 256);
			out << "\n\n";
			DumpLongTable(out,"T2", T2, 256);
			out << "\n\n";
			DumpLongTable(out,"T3", T3, 256);
			out << "\n\n";
			DumpLongTable(out,"T4", T4, 256);
			out << "\n\n";
			DumpLongTable(out,"I0", I0, 256);
			out << "\n\n";
			DumpLongTable(out,"I1", I1, 256);
			out << "\n\n";
			DumpLongTable(out,"I2", I2, 256);
			out << "\n\n";
			DumpLongTable(out,"I3", I3, 256);
			out << "\n\n";
			DumpLongTable(out,"I4", I4, 256);
			out.close();
			}
		}
	return retval;
	} // CreateAESTables

    /*
void DumpHex(const aes_byte_t * table, int length)
	{ // dump some hex values for debugging
	int pos;
	cerr << hex;
	for (pos = 0; pos < length; pos++)
		{
		if (table[pos] < 16)
			cerr << '0';
		cerr << static_cast<unsigned int>(table[pos]) << ' ';
		if ((pos %16) == 15)
			cerr << endl;
		}
	cerr << dec;
	} // DumpHex
*/

}// end of anonymous namespace

// Key expansion code - makes local copy
void AES::KeyExpansion(const aes_byte_t * key)
	{
	assert(Nk > 0);
	int i;
    aes_uint32_t temp, * Wb = reinterpret_cast<aes_uint32_t*>(W); // todo not portable - Endian problems
	if (Nk <= 6)
		{
		// todo - memcpy
		for (i = 0; i < 4*Nk; i++)
			W[i] = key[i];
		for (i = Nk; i < Nb*(Nr+1); i++)
			{
			temp = Wb[i-1];
			if ((i%Nk) == 0)
				temp = SubByte(RotByte(temp)) ^ Rcon[i/Nk];
			Wb[i] = Wb[i - Nk]^temp;
			}
		}
	else
		{
		// todo - memcpy
		for (i = 0; i < 4*Nk; i++)
			W[i] = key[i]; 
		for (i = Nk; i < Nb*(Nr+1); i++)
			{
			temp = Wb[i-1];
			if ((i%Nk) == 0)
				temp = SubByte(RotByte(temp)) ^ Rcon[i/Nk];
			else if ((i%Nk) == 4)
				temp = SubByte(temp);
			Wb[i] = Wb[i - Nk]^temp;
			}
		}
	} // KeyExpansion

void AES::SetParameters(int keylength, int blocklength)
	{
	Nk = Nr = Nb = 0; // default values

	if ((keylength != 128) && (keylength != 192) && (keylength != 256))
		return; // nothing - todo - throw error?
	if ((blocklength != 128) && (blocklength != 192) && (blocklength != 256))
		return; // nothing - todo - throw error?

	static int const parameters[] = {
//Nk*32 128     192     256
		10, 	12,  	14,  // Nb*32 = 128
		12, 	12,  	14,  // Nb*32 = 192
		14, 	14,  	14,  // Nb*32 = 256
		};
	
	// legal parameters, so fire it up
	Nk = keylength  /32;
	Nb = blocklength/32;
	Nr = parameters[((Nk-4)/2 + 3*(Nb-4)/2)];
	} // SetParameters

void AES::StartEncryption(const aes_byte_t * key)
	{
	KeyExpansion(key);
	} // StartEncryption

void AES::EncryptBlock(const aes_byte_t * datain1, aes_byte_t * dataout1)
	{ // todo ? allow in place encryption
	  // todo - clean up - lots of repeated macros
	  // we only encrypt one block from now on

    aes_uint32_t state[8*2]; // 2 buffers
    aes_uint32_t * r_ptr = reinterpret_cast<aes_uint32_t*>(W);
    aes_uint32_t * dest  = state;
    aes_uint32_t * src   = state;
    const aes_uint32_t * datain = reinterpret_cast<const aes_uint32_t*>(datain1);
    aes_uint32_t * dataout = reinterpret_cast<aes_uint32_t*>(dataout1);

	if (Nb == 4)
		{
		AddRoundKey4(dest,datain);

		if (Nr == 14)
			{
			Round4(dest,src);
			Round4(src,dest);
			Round4(dest,src);
			Round4(src,dest);
			}
		else if (Nr == 12)
			{
			Round4(dest,src);
			Round4(src,dest);
			}

		Round4(dest,src);
		Round4(src,dest);
		Round4(dest,src);
		Round4(src,dest);
		Round4(dest,src);
		Round4(src,dest);
		Round4(dest,src);
		Round4(src,dest);
		Round4(dest,src);

		FinalRound4(dataout,dest);
		}
	else if (Nb == 6)
		{
		AddRoundKey6(dest,datain);

		if (Nr == 14)
			{
			Round6(dest,src);
			Round6(src,dest);
			}

		Round6(dest,src);
		Round6(src,dest);
		Round6(dest,src);
		Round6(src,dest);
		Round6(dest,src);
		Round6(src,dest);
		Round6(dest,src);
		Round6(src,dest);
		Round6(dest,src);
		Round6(src,dest);
		Round6(dest,src);

		FinalRound6(dataout,dest);
		}
	else // Nb == 8
		{
		AddRoundKey8(dest,datain);

		Round8(dest,src);
		Round8(src,dest);
		Round8(dest,src);
		Round8(src,dest);
		Round8(dest,src);
		Round8(src,dest);
		Round8(dest,src);
		Round8(src,dest);
		Round8(dest,src);
		Round8(src,dest);
		Round8(dest,src);
		Round8(src,dest);
		Round8(dest,src);

		FinalRound8(dataout,dest);
		} // end switch on Nb

	} // Encrypt

// call this to encrypt any size block
void AES::Encrypt(const aes_byte_t * datain, aes_byte_t * dataout, aes_uint32_t numBlocks, BlockMode mode)
	{
	if (0 == numBlocks)
		return;
	unsigned int blocksize = Nb*4;
	switch (mode)
		{
		case ECB :
			while (numBlocks)
				{
				EncryptBlock(datain,dataout);
				datain   += blocksize;
				dataout  += blocksize;
				--numBlocks;
				}
			break;
		case CBC :
			{
            aes_byte_t buffer[64];
			memset(buffer,0,sizeof(buffer)); // clear out - todo - allow setting the Initialization Vector - needed for security
			while (numBlocks)
				{
				for (unsigned int pos = 0; pos < blocksize; ++pos)
					buffer[pos] ^= *datain++;
				EncryptBlock(buffer,dataout);
				memcpy(buffer,dataout,blocksize);
				dataout  += blocksize;
				--numBlocks;
				}
			}
			break;
		default :
			assert(!"Unknown mode!");
			break;
		}
	} // Encrypt

void AES::StartDecryption(const aes_byte_t * key)
{
	KeyExpansion(key);

    aes_byte_t a0,a1,a2,a3,b0,b1,b2,b3, * W_ptr = W;

	for (int col = Nb; col < (Nr)*Nb; col++) // do all but first and last round
		{
		a0 = W_ptr[4*col+0];
		a1 = W_ptr[4*col+1];
		a2 = W_ptr[4*col+2];
		a3 = W_ptr[4*col+3];
		
		b0 = GF2_8_mult(0x0E,a0)^GF2_8_mult(0x0B,a1)^
		     GF2_8_mult(0x0D,a2)^GF2_8_mult(0x09,a3);
		b1 = GF2_8_mult(0x09,a0)^GF2_8_mult(0x0E,a1)^
		     GF2_8_mult(0x0B,a2)^GF2_8_mult(0x0D,a3);
		b2 = GF2_8_mult(0x0D,a0)^GF2_8_mult(0x09,a1)^
		     GF2_8_mult(0x0E,a2)^GF2_8_mult(0x0B,a3);
		b3 = GF2_8_mult(0x0B,a0)^GF2_8_mult(0x0D,a1)^
		     GF2_8_mult(0x09,a2)^GF2_8_mult(0x0E,a3);

		W_ptr[4*col+0] = b0;
		W_ptr[4*col+1] = b1;
		W_ptr[4*col+2] = b2;
		W_ptr[4*col+3] = b3;
		}

	// we reverse the rounds to make decryption faster
    aes_uint32_t * WL = reinterpret_cast<aes_uint32_t*>(W);
	for (int pos = 0; pos < Nr/2; pos++)
		for (int col = 0; col < Nb; col++)
			swap(WL[col+pos*Nb],WL[col+(Nr-pos)*Nb]);
} // StartDecryption

void AES::DecryptBlock(const aes_byte_t * datain1, aes_byte_t * dataout1)
{
    aes_uint32_t state[8*2]; // 2 buffers
    aes_uint32_t * r_ptr = reinterpret_cast<aes_uint32_t*>(W);
    aes_uint32_t * dest  = state;
    aes_uint32_t * src   = state;

    const aes_uint32_t * datain = reinterpret_cast<const aes_uint32_t*>(datain1);
    aes_uint32_t * dataout = reinterpret_cast<aes_uint32_t*>(dataout1);

	if (Nb == 4)
		{
		AddRoundKey4(dest,datain);

		if (Nr == 14)
			{
			InvRound4(dest,src);
			InvRound4(src,dest);
			InvRound4(dest,src);
			InvRound4(src,dest);
			}
		else if (Nr == 12)
			{
			InvRound4(dest,src);
			InvRound4(src,dest);
			}

		InvRound4(dest,src);
		InvRound4(src,dest);
		InvRound4(dest,src);
		InvRound4(src,dest);
		InvRound4(dest,src);
		InvRound4(src,dest);
		InvRound4(dest,src);
		InvRound4(src,dest);
		InvRound4(dest,src);

		InvFinalRound4(dataout,dest);
		}
	else if (Nb == 6)
		{
		AddRoundKey6(dest,datain);

		if (Nr == 14)
			{
			InvRound6(dest,src);
			InvRound6(src,dest);
			}

		InvRound6(dest,src);
		InvRound6(src,dest);
		InvRound6(dest,src);
		InvRound6(src,dest);
		InvRound6(dest,src);
		InvRound6(src,dest);
		InvRound6(dest,src);
		InvRound6(src,dest);
		InvRound6(dest,src);
		InvRound6(src,dest);
		InvRound6(dest,src);

		InvFinalRound6(dataout,dest);
		}
	else // Nb == 8
		{
		AddRoundKey8(dest,datain);

		InvRound8(dest,src);
		InvRound8(src,dest);
		InvRound8(dest,src);
		InvRound8(src,dest);
		InvRound8(dest,src);
		InvRound8(src,dest);
		InvRound8(dest,src);
		InvRound8(src,dest);
		InvRound8(dest,src);
		InvRound8(src,dest);
		InvRound8(dest,src);
		InvRound8(src,dest);
		InvRound8(dest,src);

		InvFinalRound8(dataout,dest);
		} // end switch on Nb
} // Decrypt

// call this to decrypt any size block
void AES::Decrypt(const aes_byte_t * datain, aes_byte_t * dataout, aes_uint32_t numBlocks, BlockMode mode)
	{
	if (0 == numBlocks)
		return;
	unsigned int blocksize = Nb*4;
	switch (mode)
		{
		case ECB :
			while (numBlocks)
				{
				DecryptBlock(datain,dataout);
				datain   += blocksize;
				dataout  += blocksize;
				--numBlocks;
				}
			break;
		case CBC :
			{
            aes_byte_t buffer[64];
			memset(buffer,0,sizeof(buffer)); // clear out - todo - allow setting the Initialization Vector - needed for security
            DecryptBlock(datain, dataout); // do first block
			for (unsigned int pos = 0; pos < blocksize; ++pos)
				*dataout++ ^= buffer[pos];
			datain += blocksize;
			numBlocks--;

			while (numBlocks)
				{
				DecryptBlock(datain,dataout); // do first block
				for (unsigned int pos = 0; pos < blocksize; ++pos)
					*dataout++ ^= *(datain-blocksize+pos);
				datain  += blocksize;
				--numBlocks;
				}
			}
			break;
		default :
			assert(!"Unknown mode!");
		}
	} // Decrypt

// the constructor - makes sure local things are initialized
AES::AES(void)
	{
	if (false == tablesInitialized)
		tablesInitialized = CreateAESTables(true,false);
	if (false == tablesInitialized)
        qCritical() << "Tables failed to initialize";
	}

// end - AES.cpp
