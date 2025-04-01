#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  assert(s != NULL);
  size_t len = 0;
  while (s[len] != '\0') {
    len++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  assert(dst != NULL && src != NULL);
  char *d = dst, *s = (char *)src;
  while (*s != '\0') {
    *d = *s;
    d++; s++;
  }
  *d = '\0'; // Null-terminate the destination string
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  assert(dst != NULL && src != NULL);
  char *d = dst, *s = (char *)src;
  if (d >= s && d < s + n - 1) {
    // Overlapping memory regions, from back to front
    d = dst + n - 1;
    *(d + 1) = '\0'; // Null-terminate the destination string
    s = (char *)src + n - 1;
    while (n > 0) {
      *d = *s;
      d--; s--; n--;
    }
  } else {
    while (n > 0) {
      *d = *s;
      d++; s++; n--;
    }
    *d = '\0'; // Null-terminate the destination string
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  assert(dst != NULL && src != NULL);
  char *d = dst;
  while (*d != '\0') {
    d++;
  }
  strcpy(d, src); // Use strcpy to copy the source string
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  assert(s1 != NULL && s2 != NULL);
  int ret = 0;
  while (*s1 != '\0' && *s2 != '\0') {
    if (*s1 != *s2) {
      ret = *s1 - *s2;
      break;
    }
    s1++; s2++;
  }
  if (ret < 0) {
    ret = -1;
  } else if (ret > 0) {
    ret = 1;
  }
  return ret;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  while (n != 0 && *p1 != '\0' && *p2 != '\0') {
    if (*p1 != *p2) {
      return (*p1 < *p2) ? -1 : 1;
    }
    p1++; p2++; n--;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  assert(s != NULL);
  unsigned char *p = (unsigned char *)s;
  while (n != 0) {
    *p = c;
    p++; n--;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  while (n != 0 && *p1 != '\0' && *p2 != '\0') {
    if (*p1 != *p2) {
      return (*p1 < *p2) ? -1 : 1;
    }
    p1++; p2++; n--;
  }
  return 0;
}

#endif
