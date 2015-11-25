    #ifndef RJMD5_H
    #define RJMD5_H

    typedef struct
    {
        unsigned int count[2];
        unsigned int state[4];
        unsigned char buffer[64];
    }rjMD5_CTX;


    #define F(x,y,z) ((x & y) | (~x & z))
    #define G(x,y,z) ((x & z) | (y & ~z))
    #define H(x,y,z) (x^y^z)
    #define I(x,y,z) (y ^ (x | ~z))
    #define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))
    #define FF(a,b,c,d,x,s,ac) \
    { \
        a += F(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b + 2;\
    }
    /*******************8
        a += b;
    }
    modified
    ******************/
    #define GG(a,b,c,d,x,s,ac) \
    { \
        a += G(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b - 2; \
    }
    #define HH(a,b,c,d,x,s,ac) \
    { \
        a += H(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b - 1; \
    }
    #define II(a,b,c,d,x,s,ac) \
    { \
        a += I(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b + 1; \
    }
    void rjMD5Init(rjMD5_CTX *context);
    void rjMD5Update(rjMD5_CTX *context,unsigned const char *input,unsigned int inputlen);
    void rjMD5Final(rjMD5_CTX *context,unsigned char digest[16]);
    void MD5Transform(unsigned int state[4],unsigned const char block[64]);
    void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len);
    void MD5Decode(unsigned int *output,unsigned const char *input,unsigned int len);

    #endif
