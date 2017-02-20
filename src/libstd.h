#ifndef __LIBSTD_H__
#define __LIBSTD_H__

void *memset(void *str, int c, int n);
void *memcpy(void *dest, const void *src, int n);
 int memcmp(const void* s1, const void* s2, int n);

unsigned int getquotient(unsigned int dividend, unsigned int divisor);
unsigned int getremainder(unsigned int dividend, unsigned int divisor);

#endif // __LIBSTD_H__
