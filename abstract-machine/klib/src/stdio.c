#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <math.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char buf[4096];
int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int len = vsprintf(buf, fmt, ap);
  for (int i=0; i<len; i++) {
    putch(buf[i]);
  }

  va_end(ap);
  return len;
  // panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *d = out, *s = (char *) fmt;
  for (; *s != '\0'; s++) {
    if (*s != '%') {
      *d = *s;
      d++;
    } else {
      s++;
      switch (*s) {
      case 's':
        char *str = va_arg(ap, char *);
        strcpy(d, str);
        d += strlen(str);
        break;
      case 'd':
        int num = va_arg(ap, int);
        if (num < 0) {
          *d = '-';
          d++;
          num = -num;
        }
        int cnt = 0, num_tmp = num;
        while (num_tmp != 0) {
          num_tmp /= 10;
          cnt++;
        }
        for (size_t i = 0; i < cnt; i++) {
          int digit = num % 10;
          num /= 10;
          d[cnt - i - 1] = 48 + digit;
        }
        d += cnt;
        break;
      default: break;
      }
    }
  }
  *d = '\0'; // Null-terminate the destination string
  return d - out;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
