#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <math.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char printf_buf[4096];
int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int len = vsprintf(printf_buf, fmt, ap);
  for (int i=0; i<len; i++) {
    putch(printf_buf[i]);
  }

  va_end(ap);
  return len;
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
      case 'x':
        int numx = va_arg(ap, int);
        if (numx < 0) {
          *d = '-';
          d++;
          numx = -numx;
        }
        int cntx = 0, numx_tmp = numx;
        while (numx_tmp != 0) {
          numx_tmp /= 16;
          cntx++;
        }
        for (size_t i = 0; i < cntx; i++) {
          int digit = numx % 16;
          numx /= 16;
          // printf("no.%d digit: %d\n", i, digit);
          if (digit < 10) {
            d[cntx - i - 1] = 48 + digit;
          } else {
            d[cntx - i - 1] = 65 - 10 + digit;
          }
        }
        d += cntx;
        break;
      default: /* panic("vsprintf not implented yet!!!"); */ break;
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
