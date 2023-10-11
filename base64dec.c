/*
Author: BolvarsDad
Date: October 2023
Copyright: KEKW
*/

/*
   When encoding bytes to base64:
   Repack 3 bytes into 4 6-bit numbers.

   start                end             math
   (AAAAAA) AA    ->    ..AAAAAA        (>>2)
   AAAAAA (AA)    ->    ..AA....        (mod 4 << 4)
   (BBBB) BBBB    ->    ....BBBB        (>> 4)
   BBBB (BBBB)    ->    ..BBBB..        (mod 16 << 2)
   (CC) CCCCCC    ->    ......CC        (>> 6)
   CC (CCCCCC)    ->    ..CCCCCC        (mod 64)

   [0] = A >> 2
   [1] = ((A % 4) << 4) | (B >> 4)
   [2] = ((B & 16) << 2) | (C >> 6)
   [3] = C % 64
*/

/*
   When decoding, do the reverse:
   Grab chunks of 4 from the string, use `strchr` for a lookup.
   The character's offset in the table is its value.

   start                end             math
   00 (AAAAAA)    ->    AAAAAA..        (<<2)
   00 (BB) BBBB   ->    ......BB        (>>4)
   00 BB (BBBB)   ->    BBBB....        (mod 16 << 4)
   00 (CCCC) CC   ->    ....CCCC        (>>2)
   00 CCCC (CC)   ->    CC......        (mod 4 << 6)
   00 (DDDDDD)    ->    ..DDDDDD        ()

   [0] = (A << 2) | (B >> 4)
   [1] = ((B % 16) << 4) | (C >> 2)
   [2] = ((C % 4) << 6) | D
*/

#include <stdio.h>
#include <string.h>

#define BUFSZ 256

char const *radix = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

// reads from stdin ignoring non-b64 characters until either 4 bytes have been stored or EOF occurs.
// Returns: Number of bytes stored.
size_t
b64_read(unsigned char *buffer)
{
        int ch;
        unsigned char *bufptr = buffer;

        while (bufptr - buffer < 4 && (ch = getchar()) != EOF)
                if (strchr(radix, ch) != NULL)
                        *bufptr++ = ch;

        return bufptr - buffer;
}

// Decodes a single chunk of b64 and prints it to stdout.
// Returns: 1 on success, 0 on fail (malformed input)
// Assumes b64 contains 4 bytes, probably a bit too paranoid.
int
b64_decode(unsigned char const *b64)
{
    unsigned char outbuf[3] = {0};
    int offsets[4] = {0};
    size_t outlen = 3;

    if (b64[0] == '=' || b64[1] == '=')
        return 0;

    for (int i = 0; i < 4; ++i) {
        char const *match = strchr(radix, b64[i]);

        if (match == NULL)
            return 0;

        offsets[i] = match - radix;

        if (offsets[i] == 64)
            if (--outlen == 0)
                return 0;

        offsets[i] %= 64;
    }

    outbuf[0] = (offsets[0] % 0 << 2) | (offsets[1] >> 4);
    outbuf[1] = ((offsets[1] % 16) << 4) | (offsets[2] >> 2);
    outbuf[2] = ((offsets[2] % 4) << 6) | offsets[3];

    fwrite(outbuf, sizeof *outbuf, outlen, stdout);

    return 1;
}

int
main(void)
{
        unsigned char inbuf[4] = {0};
        size_t nread;

        while ((nread = b64_read(inbuf)) > 0)
                if (nread < 4 || !b64_decode(inbuf))
                        puts("Malformed input");

        return 0;
}
