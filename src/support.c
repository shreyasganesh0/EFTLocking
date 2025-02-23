#include <string.h>


void my_stream_buf(int num, int bal, buf_t *buf) {
    
    if (buf->capacity < buf->size + 1000) {
        buf->start = realloc(buf->start, buf->capacity * 2);
        buf->ptr = buf->start + buf->size;
    }
    
    static const int po10[5] = {1, 10, 100, 1000, 10000};
    
    int d_idx = 4;
    int divisor = po10[d_idx];
    int neg_num = (num < 0);
    
    if (neg_num) {
        num = -num;
        *buf->ptr++ = '-';
    }
        
    while (num/divisor == 0 && divisor > 1) {
        d_idx--;
        divisor = po10[d_idx];
    }

    while (d_idx >= 0) {
        int curr_dig = num/po10[d_idx];
        *buf->ptr++ = '0' + curr_dig;
        num -= curr_dig * po10[d_idx];
        buf->size++;
        d_idx--;
    }

    *buf->ptr++ = ' ';
    buf->size++;
    
    d_idx = 4;
    int neg_bal = (bal < 0);
    divisor = po10[d_idx];
    
    if (neg_bal) {
        bal = -bal;
        *buf->ptr++ = '-';
    }
    while (bal/divisor == 0 && divisor > 1) {
        d_idx--;
        divisor = po10[d_idx];
    }

    while (d_idx >= 0) {
        int curr_dig = bal/po10[d_idx];
        *buf->ptr++ = '0' + curr_dig;
        buf->size++;
        bal -= curr_dig * po10[d_idx];
        d_idx--;
    }
    
    *buf->ptr++ = '\n';
    buf->size += 2;
}
