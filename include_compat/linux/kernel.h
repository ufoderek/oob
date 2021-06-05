#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define GFP_KERNEL 0

#define kmalloc(size, flags) malloc(size)
#define kzalloc(size, flags) calloc(1, size)
#define kfree free

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define cpu_to_be32(x) __builtin_bswap32(x)
#define unlikely(x) x

#define __KERNEL_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_UP __KERNEL_DIV_ROUND_UP

#define KERN_ERR
#define printk printf

#define __WARN() printf("__WARN()")
#define WARN_ON(condition) ({			\
	int __ret_warn_on = !!(condition);	\
	if (unlikely(__ret_warn_on))		\
		__WARN();			\
	unlikely(__ret_warn_on);		\
})

static __always_inline int fls(unsigned int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

#define EXPORT_SYMBOL(x)
#define EXPORT_SYMBOL_GPL(x)

#define MODULE_LICENSE(x)
#define MODULE_AUTHOR(x)
#define MODULE_DESCRIPTION(x)

#endif /* __KERNEL_H__ */
