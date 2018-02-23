/* common_func.h */
#ifndef COMMON_FUNC_H
#define COMMON_FUNC_H

/* use 64-bit off_t.
 * these macros must be defined before any included file */
#undef _LARGEFILE64_SOURCE
#undef _FILE_OFFSET_BITS
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

/* internationalization support via gettext/libintl */
#ifdef USE_GETTEXT
# include <libintl.h>
# define _(str) gettext(str)
# define TEXT_DOMAIN "rhash"
# ifndef LOCALEDIR
#  define LOCALEDIR "/usr/share/locale"
# endif /* LOCALEDIR */
#else
# define _(str) (str)
#endif /* USE_GETTEXT */

#include <stdint.h>
#include <stdio.h>
#include <time.h> /* for time_t */
#include <stddef.h> /* for wchar_t */

#if !defined( _WIN32) && !defined(__CYGWIN__)
# include <sys/time.h> /* for timeval */
#elif _MSC_VER > 1300
# include "platform.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* string function */
void sprintI64(char *dst, uint64_t number, int max_width);
int  int_len(uint64_t num);

int  urlencode(char *dst, const char *name);
int  is_binary_string(const char* str);
char* str_tolower(const char* str);
char* str_trim(char* str);
char* str_set(char* buf, int ch, int size);
char* str_append(const char* orig, const char* append);
size_t strlen_utf8_c(const char *str);

#define IS_COMMENT(c) ((c) == ';' || (c) == '#')

#ifdef _WIN32
typedef wchar_t rsh_tchar;
# define RSH_T(str) L##str
#else
typedef  char rsh_tchar;
# define RSH_T(str) str
#endif /* _WIN32 */

#ifdef _WIN32
# define IF_WINDOWS(code) code
# define SYS_PATH_SEPARATOR '\\'
# define IS_PATH_SEPARATOR(c) ((c) == '\\' || (c) == '/')
# define IS_PATH_SEPARATOR_W(c) ((c) == L'\\' || (c) == L'/')
# define is_utf8() win_is_utf8()
# define to_utf8(str) win_to_utf8(str)
#else /* non _WIN32 part */
# define IF_WINDOWS(code)
# define SYS_PATH_SEPARATOR '/'
# define IS_PATH_SEPARATOR(c) ((c) == '/')
/* stub for utf8 */
# define is_utf8() 1
# define to_utf8(str) NULL
#endif /* _WIN32 */

/* version information */
const char* get_version_string(void);
const char* get_bt_program_name(void);


/**
 * Portable file information.
 */
typedef struct file_t
{
	char* path;
	wchar_t* wpath;
	uint64_t size;
	uint64_t mtime;
	unsigned mode;
} file_t;

/* bit constants for the file_t.mode bit mask */
#define FILE_IFDIR   0x01
#define FILE_IFLNK   0x02
#define FILE_IFREG   0x04
#define FILE_IFROOT  0x10
#define FILE_IFSTDIN 0x20
#define FILE_OPT_DONT_FREE_PATH  0x40

#define FILE_ISDIR(file) ((file)->mode & FILE_IFDIR)
#define FILE_ISLNK(file) ((file)->mode & FILE_IFLNK)
#define FILE_ISREG(file) ((file)->mode & FILE_IFREG)

/* file functions */

const char* get_basename(const char* path);
char* get_dirname(const char* path);
char* make_path(const char* dir, const char* filename);
int are_paths_equal(const rsh_tchar* a, const rsh_tchar* b);

void file_init(file_t* file, const char* path, int reuse_path);
void file_cleanup(file_t* file);
int file_stat(file_t* file);
int file_stat2(file_t* file, int use_lstat);
int is_regular_file(const char* path);
int if_file_exists(const char* path);

#ifdef _WIN32
# define get_file_tpath(file) ((file)->wpath)
int file_statw(file_t* file);
# define rsh_fopen_bin(path, mode) win_fopen_bin(path, mode)
# define rsh_fprintf win_fprintf
# define rsh_vfprintf win_vfprintf
# define rsh_fwrite win_fwrite
#else
# define get_file_tpath(file) ((file)->path)
# define rsh_fopen_bin(path, mode) fopen(path, mode)
# define rsh_fprintf fprintf
# define rsh_vfprintf vfprintf
# define rsh_fwrite fwrite
#endif


/* time data and functions */

/* portable timer definition */
#if defined( _WIN32) || defined(__CYGWIN__)
typedef unsigned long long timedelta_t;
#else
#include <sys/time.h> /* for timeval */
typedef struct timeval timedelta_t;
#endif

/**
 * Start a timer.
 *
 * @param timer timer to start
 */
void rsh_timer_start(timedelta_t* timer);

/**
 * Stop given timer.
 *
 * @param timer the timer to stop
 * @return number of seconds timed
 */
double rsh_timer_stop(timedelta_t* timer);

/**
 * Return ticks in milliseconds for time intervals measurement.
 * This function should be optimized for speed and retrieve
 * internal clock value, if possible.
 *
 * @return ticks count in milliseconds
 */
unsigned rhash_get_ticks(void);

/* program exit handlers */
typedef void (*exit_handler_t)(void);
void rsh_install_exit_handler(exit_handler_t handler);
void rsh_remove_exit_handler(void);
void rsh_exit(int code);

/* clever malloc with error detection */
#define rsh_malloc(size) rhash_malloc(size, __FILE__, __LINE__)
#define rsh_calloc(num, size) rhash_calloc(num, size, __FILE__, __LINE__)
#define rsh_strdup(str)  rhash_strdup(str,  __FILE__, __LINE__)
#define rsh_realloc(mem, size) rhash_realloc(mem, size, __FILE__, __LINE__)
void* rhash_malloc(size_t size, const char* srcfile, int srcline);
void* rhash_calloc(size_t num, size_t size, const char* srcfile, int srcline);
char* rhash_strdup(const char* str, const char* srcfile, int srcline);
void* rhash_realloc(void* mem, size_t size, const char* srcfile, int srcline);

#ifdef _WIN32
#define rsh_wcsdup(str) rhash_wcsdup(str, __FILE__, __LINE__)
wchar_t* rhash_wcsdup(const wchar_t* str, const char* srcfile, int srcline);
#endif

extern void (*rsh_report_error)(const char* srcfile, int srcline, const char* format, ...);

/* vector functions */
typedef struct vector_t
{
	void **array;
	size_t size;
	size_t allocated;
	void (*destructor)(void*);
} vector_t;

vector_t* rsh_vector_new(void (*destructor)(void*));
vector_t* rsh_vector_new_simple(void);
void rsh_vector_free(vector_t* vect);
void rsh_vector_destroy(vector_t* vect);
void rsh_vector_add_ptr(vector_t* vect, void *item);
void rsh_vector_add_empty(vector_t* vect, size_t item_size);
#define rsh_vector_add_uint32(vect, item) { \
	rsh_vector_add_empty(vect, item_size); \
	((unsigned*)(vect)->array)[(vect)->size - 1] = item; \
}
#define rsh_vector_add_item(vect, item, item_size) { \
	rsh_vector_add_empty(vect, item_size); \
	memcpy(((char*)(vect)->array) + item_size * ((vect)->size - 1), item, item_size); \
}

/* a vector pattern implementation, allocating elements by blocks */
typedef struct blocks_vector_t
{
	size_t size;
	vector_t blocks;
} blocks_vector_t;

void rsh_blocks_vector_init(blocks_vector_t*);
void rsh_blocks_vector_destroy(blocks_vector_t* vect);
#define rsh_blocks_vector_get_item(bvector, index, blocksize, item_type) \
	(&((item_type*)((bvector)->blocks.array[(index) / (blocksize)]))[(index) % (blocksize)])
#define rsh_blocks_vector_get_ptr(bvector, index, blocksize, item_size) \
	(&((unsigned char*)((bvector)->blocks.array[(index) / (blocksize)]))[(item_size) * ((index) % (blocksize))])
#define rsh_blocks_vector_add(bvector, item, blocksize, item_size) { \
	if (((bvector)->size % (blocksize)) == 0) \
	rsh_vector_add_ptr(&((bvector)->blocks), rsh_malloc((item_size) * (blocksize))); \
	memcpy(rsh_blocks_vector_get_ptr((bvector), (bvector)->size, (blocksize), (item_size)), (item), (item_size)); \
	(bvector)->size++; \
}
#define rsh_blocks_vector_add_ptr(bvector, ptr, blocksize) { \
	if (((bvector)->size % (blocksize)) == 0) \
	rsh_vector_add_ptr(&((bvector)->blocks), rsh_malloc(sizeof(void*) * (blocksize))); \
	((void***)(bvector)->blocks.array)[(bvector)->size / (blocksize)][(bvector)->size % (blocksize)] = (void*)ptr; \
	(bvector)->size++; \
}
#define rsh_blocks_vector_add_empty(bvector, blocksize, item_size) { \
	if ( (((bvector)->size++) % (blocksize)) == 0) \
	rsh_vector_add_ptr(&((bvector)->blocks), rsh_malloc((item_size) * (blocksize))); \
}

/* string buffer functions */
typedef struct strbuf_t
{
	char* str;
	size_t allocated;
	size_t len;
} strbuf_t;

strbuf_t* rsh_str_new(void);
void rsh_str_free(strbuf_t* buf);
void rsh_str_ensure_size(strbuf_t *str, size_t new_size);
void rsh_str_append_n(strbuf_t *str, const char* text, size_t len);
void rsh_str_append(strbuf_t *str, const char* text);

#define rsh_str_ensure_length(str, len) \
	if ((size_t)(len) >= (size_t)(str)->allocated) rsh_str_ensure_size((str), (len) + 1);
#define rsh_wstr_ensure_length(str, len) \
	if ((size_t)((len) + sizeof(wchar_t)) > (size_t)(str)->allocated) rsh_str_ensure_size((str), (len) + sizeof(wchar_t));

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* COMMON_FUNC_H */
