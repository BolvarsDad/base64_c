/*
Author: BolvarsDad
Date: October 2023
Copyright: KEKW
*/

/*
   When encoding bytes to base64:
   Repack 3 bytes into 4 6-bit numbers.

   start		end		math
   (AAAAAA) AA	  ->	..AAAAAA	(>>2)
   AAAAAA (AA)	  ->	..AA....	(mod 4 << 4)
   (BBBB) BBBB	  ->	....BBBB	(>> 4)
   BBBB (BBBB)	  ->	..BBBB..	(mod 16 << 2)
   (CC) CCCCCC	  ->	......CC	(>> 6)
   CC (CCCCCC)	  -> 	..CCCCCC	(mod 64)

   [0] = A >> 2
   [1] = ((A % 4) << 4) | (B >> 4)
   [2] = ((B & 16) << 2) | (C >> 6)
   [3] = C % 64
*/

/*
   When decoding, do the reverse:
   Grab chunks of 4 from the string, use `strchr` for a lookup.
   The character's offset in the table is its value.

   start		end		math
   00 (AAAAAA)	  ->	AAAAAA..	(<<2)
   00 (BB) BBBB	  ->	......BB	(>>4)
   00 BB (BBBB)	  ->	BBBB....	(mod 16 << 4)
   00 (CCCC) CC   ->	....CCCC	(>>2)
   00 CCCC (CC)	  -> 	CC......	(mod 4 << 6)
   00 (DDDDDD) 	  ->	..DDDDDD	()

   [0] = (A << 2) | (B >> 4)
   [1] = ((B % 16) << 4) | (C >> 2)
   [2] = ((C % 4) << 6) | D
*/

#include <stdio.h>
#include <string.h>

#define BUFSZ 256

#define MIN(a,b) \
	((a) < (b) ? (a) : (b))

char const *radix = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

void
b64_decode(char const *in)
{
	size_t len = MIN(strlen(in), BUFSZ);
	
	if (len % 4 != 0)
		return;

	for (size_t i = 0; i < len; i += 4)
	{
		int val = 0;
		int pad = 2;
		int k	= 0;

		for (int j = 0; j < 4; ++j)
		{
			char *ch = strchr(radix, in[i+j]);

			if (ch == NULL)
				return;

			int idx = ch - radix;

			// Check for pad character
			if (idx == 64)
				idx = 0;

			val = (val << 6) | idx;
		}

		char chunk[3];
		chunk[0] = (val >> 16) & 0xFF;
		chunk[1] = (val >>  8) & 0xFF;
		chunk[2] = (val >>  0) & 0xFF;

		while (!chunk[pad])
			pad--;

		while (k <= pad)
			putchar(chunk[k++]);
	}
}

int
main(void)
{
	char buffer[BUFSZ];
	size_t nread;

	fgets(buffer, BUFSZ, stdin);
	buffer[strcspn(buffer, "\r\n")] = 0;

	char *str = buffer;

	b64_decode(str);

	return 0;
}
