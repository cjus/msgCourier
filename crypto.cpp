/* crypto.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

crypto.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

crypto.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with crypto.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
#include "cryptopp/crc.h"
#include "cryptopp/adler32.h"
#include "cryptopp/md2.h"
#include "cryptopp/md5.h"
#include "cryptopp/md5mac.h"
#include "cryptopp/sha.h"
#include "cryptopp/haval.h"
#include "cryptopp/tiger.h"
#include "cryptopp/ripemd.h"
#include "cryptopp/panama.h"
#include "cryptopp/whrlpool.h"
#include "cryptopp/idea.h"
#include "cryptopp/des.h"
#include "cryptopp/rc2.h"
#include "cryptopp/arc4.h"
#include "cryptopp/rc5.h"
#include "cryptopp/blowfish.h"
#include "cryptopp/wake.h"
#include "cryptopp/3way.h"
#include "cryptopp/safer.h"
#include "cryptopp/gost.h"
#include "cryptopp/shark.h"
#include "cryptopp/cast.h"
#include "cryptopp/square.h"
#include "cryptopp/skipjack.h"
#include "cryptopp/seal.h"
#include "cryptopp/rc6.h"
#include "cryptopp/mars.h"
#include "cryptopp/rijndael.h"
#include "cryptopp/twofish.h"
#include "cryptopp/serpent.h"
#include "cryptopp/shacal2.h"
#include "cryptopp/camellia.h"
#include "cryptopp/hmac.h"
#include "cryptopp/xormac.h"
#include "cryptopp/cbcmac.h"
#include "cryptopp/dmac.h"
#include "cryptopp/ttmac.h"
#include "cryptopp/blumshub.h"
#include "cryptopp/rng.h"
#include "cryptopp/files.h"
#include "cryptopp/hex.h"
#include "cryptopp/modes.h"
#include "cryptopp/mdc.h"
#include "cryptopp/lubyrack.h"
#include "cryptopp/tea.h"
#include "cryptopp/randpool.h"
#include "cryptopp/rsa.h"
#include "cryptopp/base64.h"
*/
#include <iostream>
#include "crypto.h"

using namespace std;
//using namespace CryptoPP;


void cCrypto::CreateSHA(const char *pData, int len, string &shaoutput)
{
	/*	
	byte shahash[SHA::DIGESTSIZE];
	SHA().CalculateDigest(shahash, (const byte*)pData, len);

	byte buffer[1 + 2 * SHA::DIGESTSIZE];

	HexEncoder hex;
	hex.Put(shahash, SHA::DIGESTSIZE);
	hex.MessageEnd();
	hex.Get(buffer, 2*SHA::DIGESTSIZE);
	buffer[2 * SHA::DIGESTSIZE] = 0;
	shaoutput = (const char *)buffer;
	*/
}

void cCrypto::GenerateCerts()
{
	/*
	//Print("Generation of public/private key certificates is a time consuming task.\nPlease be patient...");
	try
	{
		//unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed
		unsigned int keyLength = 4096;
		const char *privFilename = "msgCourier.prv";
		const char *pubFilename = "msgCourier.pub";

		// generate seed
		char timestring[20];
		sprintf(timestring, "%x", time(0));
		byte shahash[SHA::DIGESTSIZE];
		SHA().CalculateDigest(shahash, (const byte*)timestring, strlen(timestring));
		
		byte seed[1 + 2 * SHA::DIGESTSIZE];
		HexEncoder hex;
		hex.Put(shahash, SHA::DIGESTSIZE);
		hex.MessageEnd();
		hex.Get(seed, 2*SHA::DIGESTSIZE);
		seed[2 * SHA::DIGESTSIZE] = 0;
		
		// create certificates
		RandomPool randPool;	
		randPool.Put((byte*)seed, strlen((char*)seed));

		RSAES_OAEP_SHA_Decryptor priv(randPool, keyLength);
		HexEncoder privFile(new FileSink(privFilename));
		priv.DEREncode(privFile);
		privFile.MessageEnd();

		RSAES_OAEP_SHA_Encryptor pub(priv);
		HexEncoder pubFile(new FileSink(pubFilename));
		pub.DEREncode(pubFile);
		pubFile.MessageEnd();
		//Print("\nGeneration of public/private key certificates succeeded.\nThe files msgCourier.prv and msgCourier.pub have been created.\n");
	}
	catch (...)
	{
		//Print("\nGeneration of public/private key certificates failed.\n");
	}
	*/
}


