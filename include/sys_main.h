/* Copyright (c) 2000, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef SYS_MAIN_H
#define SYS_MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#define MY_FAE		8	/* Fatal if any error */
#define MY_WME		16	/* Write message on error */
#define MY_ZEROFILL	32	/* my_malloc(), fill array with zero */
#define MY_ALLOW_ZERO_PTR 64	/* my_realloc() ; zero ptr -> malloc */

#define	_MY_NMR	04	/* Numeral (digit) */
#define	_MY_SPC	010	/* Spacing character */

#define MY_CS_ILSEQ	0     /* Wrong by sequence: wb_wc                   */
#define MY_CS_ILUNI	0     /* Cannot encode Unicode to charset: wc_mb    */
#define MY_CS_PRIMARY	32     /* if primary collation           */
#define MY_CS_TOOSMALL  -101  /* Need at least one byte:    wc_mb and mb_wc */

typedef unsigned int uint;
typedef unsigned long ulong;
typedef int		myf_t;	/* Type of MyFlags in my_funcs */
/* Macros for converting *constants* to the right type */
#define MYF(v)		(myf) (v)

typedef struct st_dynamic_array
{
  uchar *buffer;
  uint elements, max_element;
  uint alloc_increment;
  uint size_of_element;
  PSI_memory_key m_psi_key;
} DYNAMIC_ARRAY;

typedef struct st_dynamic_string
{
  char *str;
  size_t length, max_length, alloc_increment;
} DYNAMIC_STRING;

#ifdef USE_MYSQL_HEADERS
typedef struct st_used_mem
{				   /* struct for once_alloc (block) */
  struct st_used_mem *next;	   /* Next block in use */
  unsigned int	left;		   /* memory left in block  */
  unsigned int	size;		   /* size of block */
} USED_MEM;

typedef struct st_mem_root
{
  USED_MEM *free;                  /* blocks with free memory in it */
  USED_MEM *used;                  /* blocks almost without free memory */
  USED_MEM *pre_alloc;             /* preallocated block */
  /* if block have less memory it will be put in 'used' list */
  size_t min_malloc;
  size_t block_size;               /* initial block size */
  unsigned int block_num;          /* allocated blocks counter */
  /*
  first free block in queue test counter (if it exceed
  MAX_BLOCK_USAGE_BEFORE_DROP block will be dropped in 'used' list)
  */
  unsigned int first_block_usage;

  /*
  Maximum amount of memory this mem_root can hold. A value of 0
  implies there is no limit.
  */
  size_t max_capacity;

  /* Allocated size for this mem_root */

  size_t allocated_size;

  /* Enable this for error reporting if capacity is exceeded */
  my_bool error_for_capacity_exceeded;

  void(*error_handler)(void);

  PSI_memory_key m_psi_key;
} MEM_ROOT;

typedef struct st_list {
  struct st_list *prev, *next;
  void *data;
} LIST;
#endif

extern void * mysys_malloc(size_t size, myf_t flags);
extern void mysys_free(void *ptr);

extern void * my_realloc(PSI_memory_key key, void *ptr, size_t size, myf_t flags);
extern void my_claim(void *ptr);
extern void * my_memdup(PSI_memory_key key, const void *from, size_t length, myf_t flags);
extern char * my_strdup(PSI_memory_key key, const char *from, myf_t flags);
extern char * my_strndup(PSI_memory_key key, const char *from, size_t length, myf_t flags);

extern	char *my_stpmov(char *dst, const char *src);
extern	char *strxmov(char *dst, const char *src, ...);

extern void *alloc_dynamic(DYNAMIC_ARRAY *array);
extern void delete_dynamic(DYNAMIC_ARRAY *array);
extern my_bool insert_dynamic(DYNAMIC_ARRAY *array, const void *element);
extern my_bool dynstr_realloc(DYNAMIC_STRING *str, size_t additional_size);
static inline void reset_dynamic(DYNAMIC_ARRAY *array)
{
  array->elements = 0;
}


extern my_bool my_init_dynamic_array(DYNAMIC_ARRAY *array,
                                     PSI_memory_key key,
                                     uint element_size,
                                     void *init_buffer,
                                     uint init_alloc,
                                     uint alloc_increment);

extern my_bool init_dynamic_array(DYNAMIC_ARRAY *array, uint element_size,
                                  uint init_alloc, uint alloc_increment);

extern my_bool init_dynamic_string(DYNAMIC_STRING *str, const char *init_str,
                                   size_t init_alloc, size_t alloc_increment);
extern my_bool dynstr_append_mem(DYNAMIC_STRING *str, const char *append,
                          size_t length);
extern my_bool dynstr_append(DYNAMIC_STRING *str, const char *append);

extern my_bool dynstr_append_os_quoted(DYNAMIC_STRING *str, const char *append,
                                       ...);

extern void dynstr_free(DYNAMIC_STRING *str);

extern void *alloc_root(MEM_ROOT *mem_root, size_t Size);
extern char *int2str(long val, char *dst, int radix, int upcase);
extern char *strdup_root(MEM_ROOT *root, const char *str);
extern char *strmake_root(MEM_ROOT *root, const char *str, size_t len);
extern void free_root(MEM_ROOT *root, myf MyFLAGS);

extern LIST *list_add(LIST *root, LIST *element);
extern LIST *list_delete(LIST *root, LIST *element);
extern size_t my_snprintf(char* to, size_t n, const char* fmt, ...);

extern void my_thread_end();

extern my_bool my_sys_init(void);
extern void my_end(int infoflag);
extern void my_qsort(void *base_ptr, size_t total_elems, size_t size, qsort_cmp cmp);

extern int is_prefix(const char *a, const char *b);

extern char *strfill(char * s, size_t len, pchar fill);
extern char *strmake(char *dst, const char *src, size_t length);
#define my_strcasecmp(s, a, b)        ((s)->coll->strcasecmp((s), (a), (b)))

/* Charsets */

#define my_wc_t ulong
typedef struct my_uni_idx_st
{
  uint16      from;
  uint16      to;
  const uchar *tab;
} MY_UNI_IDX;

typedef struct
{
  uint beg;
  uint end;
  uint mb_len;
} my_match_t;

struct charset_info_st;

typedef struct my_charset_loader_st
{
  char error[128];
  void *(*once_alloc)(size_t);
  void *(*mem_malloc)(size_t);
  void *(*mem_realloc)(void *, size_t);
  void(*mem_free)(void *);
  void(*reporter)(enum loglevel, const char *format, ...);
  int(*add_collation)(struct charset_info_st *cs);
} MY_CHARSET_LOADER;

#define MY_UCA_MAX_CONTRACTION 6
#define MY_UCA_MAX_WEIGHT_SIZE 8
#define MY_UCA_WEIGHT_LEVELS   1

typedef struct my_contraction_t
{
  my_wc_t ch[MY_UCA_MAX_CONTRACTION];   /* Character sequence              */
  uint16 weight[MY_UCA_MAX_WEIGHT_SIZE];/* Its weight string, 0-terminated */
  my_bool with_context;
} MY_CONTRACTION;


typedef struct my_contraction_list_t
{
  size_t nitems;         /* Number of items in the list                  */
  MY_CONTRACTION *item;  /* List of contractions                         */
  char *flags;           /* Character flags, e.g. "is contraction head") */
} MY_CONTRACTIONS;

/* Collation weights on a single level (e.g. primary, secondary, tertiarty) */
typedef struct my_uca_level_info_st
{
  my_wc_t maxchar;
  uchar   *lengths;
  uint16  **weights;
  MY_CONTRACTIONS contractions;
} MY_UCA_WEIGHT_LEVEL;

typedef struct uca_info_st
{
  MY_UCA_WEIGHT_LEVEL level[MY_UCA_WEIGHT_LEVELS];

  /* Logical positions */
  my_wc_t first_non_ignorable;
  my_wc_t last_non_ignorable;
  my_wc_t first_primary_ignorable;
  my_wc_t last_primary_ignorable;
  my_wc_t first_secondary_ignorable;
  my_wc_t last_secondary_ignorable;
  my_wc_t first_tertiary_ignorable;
  my_wc_t last_tertiary_ignorable;
  my_wc_t first_trailing;
  my_wc_t last_trailing;
  my_wc_t first_variable;
  my_wc_t last_variable;

} MY_UCA_INFO;

typedef struct unicase_info_char_st
{
  uint32 toupper;
  uint32 tolower;
  uint32 sort;
} MY_UNICASE_CHARACTER;


typedef struct unicase_info_st
{
  my_wc_t maxchar;
  const MY_UNICASE_CHARACTER **page;
} MY_UNICASE_INFO;

#ifndef MY_ATTRIBUTE
#if defined(__GNUC__)
#  define MY_ATTRIBUTE(A) __attribute__(A)
#else
#  define MY_ATTRIBUTE(A)
#endif
#endif

enum MY_ATTRIBUTE((__packed__)) my_lex_states
{
  MY_LEX_START, MY_LEX_CHAR, MY_LEX_IDENT,
  MY_LEX_IDENT_SEP, MY_LEX_IDENT_START,
  MY_LEX_REAL, MY_LEX_HEX_NUMBER, MY_LEX_BIN_NUMBER,
  MY_LEX_CMP_OP, MY_LEX_LONG_CMP_OP, MY_LEX_STRING, MY_LEX_COMMENT, MY_LEX_END,
  MY_LEX_OPERATOR_OR_IDENT, MY_LEX_NUMBER_IDENT, MY_LEX_INT_OR_REAL,
  MY_LEX_REAL_OR_POINT, MY_LEX_BOOL, MY_LEX_EOL, MY_LEX_ESCAPE,
  MY_LEX_LONG_COMMENT, MY_LEX_END_LONG_COMMENT, MY_LEX_SEMICOLON,
  MY_LEX_SET_VAR, MY_LEX_USER_END, MY_LEX_HOSTNAME, MY_LEX_SKIP,
  MY_LEX_USER_VARIABLE_DELIMITER, MY_LEX_SYSTEM_VAR,
  MY_LEX_IDENT_OR_KEYWORD,
  MY_LEX_IDENT_OR_HEX, MY_LEX_IDENT_OR_BIN, MY_LEX_IDENT_OR_NCHAR,
  MY_LEX_STRING_OR_DELIMITER
};


enum MY_ATTRIBUTE((__packed__)) hint_lex_char_classes
{
  HINT_CHR_ASTERISK,                    // [*]
  HINT_CHR_AT,                          // [@]
  HINT_CHR_BACKQUOTE,                   // [`]
  HINT_CHR_CHAR,                        // default state
  HINT_CHR_DIGIT,                       // [[:digit:]]
  HINT_CHR_DOUBLEQUOTE,                 // ["]
  HINT_CHR_EOF,                         // pseudo-class
  HINT_CHR_IDENT,                       // [_$[:alpha:]]
  HINT_CHR_MB,                          // multibyte character
  HINT_CHR_NL,                          // \n
  HINT_CHR_SLASH,                       // [/]
  HINT_CHR_SPACE                        // [[:space:]] excluding \n
};


/* See strings/CHARSET_INFO.txt for information about this structure  */
typedef struct my_collation_handler_st
{
  my_bool(*init)(struct charset_info_st *, MY_CHARSET_LOADER *);
  /* Collation routines */
  int(*strnncoll)(const struct charset_info_st *,
                  const uchar *, size_t, const uchar *, size_t, my_bool);
  int(*strnncollsp)(const struct charset_info_st *,
                    const uchar *, size_t, const uchar *, size_t,
                    my_bool diff_if_only_endspace_difference);
  size_t(*strnxfrm)(const struct charset_info_st *,
                    uchar *dst, size_t dstlen, uint nweights,
                    const uchar *src, size_t srclen, uint flags);
  size_t(*strnxfrmlen)(const struct charset_info_st *, size_t);
  my_bool(*like_range)(const struct charset_info_st *,
                       const char *s, size_t s_length,
                       pchar w_prefix, pchar w_one, pchar w_many,
                       size_t res_length,
                       char *min_str, char *max_str,
                       size_t *min_len, size_t *max_len);
  int(*wildcmp)(const struct charset_info_st *,
                const char *str, const char *str_end,
                const char *wildstr, const char *wildend,
                int escape, int w_one, int w_many);

  int(*strcasecmp)(const struct charset_info_st *, const char *,
                   const char *);

  uint(*instr)(const struct charset_info_st *,
               const char *b, size_t b_length,
               const char *s, size_t s_length,
               my_match_t *match, uint nmatch);

  /* Hash calculation */
  void(*hash_sort)(const struct charset_info_st *cs, const uchar *key,
                   size_t len, ulong *nr1, ulong *nr2);
  my_bool(*propagate)(const struct charset_info_st *cs, const uchar *str,
                      size_t len);
} MY_COLLATION_HANDLER;

/* Some typedef to make it easy for C++ to make function pointers */
typedef int(*my_charset_conv_mb_wc)(const struct charset_info_st *,
                                    my_wc_t *, const uchar *, const uchar *);
typedef int(*my_charset_conv_wc_mb)(const struct charset_info_st *, my_wc_t,
                                    uchar *, uchar *);
typedef size_t(*my_charset_conv_case)(const struct charset_info_st *,
                                      char *, size_t, char *, size_t);

/* See strings/CHARSET_INFO.txt about information on this structure  */
typedef struct my_charset_handler_st
{
  my_bool(*init)(struct charset_info_st *, MY_CHARSET_LOADER *loader);
  /* Multibyte routines */
  uint(*ismbchar)(const struct charset_info_st *, const char *,
                  const char *);
  uint(*mbcharlen)(const struct charset_info_st *, uint c);
  size_t(*numchars)(const struct charset_info_st *, const char *b,
                    const char *e);
  size_t(*charpos)(const struct charset_info_st *, const char *b,
                   const char *e, size_t pos);
  size_t(*well_formed_len)(const struct charset_info_st *,
                           const char *b, const char *e,
                           size_t nchars, int *error);
  size_t(*lengthsp)(const struct charset_info_st *, const char *ptr,
                    size_t length);
  size_t(*numcells)(const struct charset_info_st *, const char *b,
                    const char *e);

  /* Unicode conversion */
  my_charset_conv_mb_wc mb_wc;
  my_charset_conv_wc_mb wc_mb;

  /* CTYPE scanner */
  int(*ctype)(const struct charset_info_st *cs, int *ctype,
              const uchar *s, const uchar *e);

  /* Functions for case and sort conversion */
  size_t(*caseup_str)(const struct charset_info_st *, char *);
  size_t(*casedn_str)(const struct charset_info_st *, char *);

  my_charset_conv_case caseup;
  my_charset_conv_case casedn;

  /* Charset dependant snprintf() */
  size_t(*snprintf)(const struct charset_info_st *, char *to, size_t n,
                    const char *fmt,
                    ...) MY_ATTRIBUTE((format(printf, 4, 5)));
  size_t(*long10_to_str)(const struct charset_info_st *, char *to, size_t n,
                         int radix, long int val);
  size_t(*longlong10_to_str)(const struct charset_info_st *, char *to,
                             size_t n, int radix, longlong val);

  void(*fill)(const struct charset_info_st *, char *to, size_t len,
              int fill);

  /* String-to-number conversion routines */
  long(*strntol)(const struct charset_info_st *, const char *s,
                 size_t l, int base, char **e, int *err);
  ulong(*strntoul)(const struct charset_info_st *, const char *s,
                   size_t l, int base, char **e, int *err);
  longlong(*strntoll)(const struct charset_info_st *, const char *s,
                      size_t l, int base, char **e, int *err);
  ulonglong(*strntoull)(const struct charset_info_st *, const char *s,
                        size_t l, int base, char **e, int *err);
  double(*strntod)(const struct charset_info_st *, char *s,
                   size_t l, char **e, int *err);
  longlong(*strtoll10)(const struct charset_info_st *cs,
                       const char *nptr, char **endptr, int *error);
  ulonglong(*strntoull10rnd)(const struct charset_info_st *cs,
                             const char *str, size_t length,
                             int unsigned_fl,
                             char **endptr, int *error);
  size_t(*scan)(const struct charset_info_st *, const char *b,
                const char *e, int sq);
} MY_CHARSET_HANDLER;


struct lex_state_maps_st
{
  enum my_lex_states main_map[256];
  enum hint_lex_char_classes hint_map[256];
};

typedef struct charset_info_st
{
  uint      number;
  uint      primary_number;
  uint      binary_number;
  uint      state;
  const char *csname;
  const char *name;
  const char *comment;
  const char *tailoring;
  const uchar *ctype;
  const uchar *to_lower;
  const uchar *to_upper;
  const uchar *sort_order;
  MY_UCA_INFO *uca; /* This can be changed in apply_one_rule() */
  const uint16     *tab_to_uni;
  const MY_UNI_IDX *tab_from_uni;
  const MY_UNICASE_INFO *caseinfo;
  const struct lex_state_maps_st *state_maps; /* parser internal data */
  const uchar *ident_map; /* parser internal data */
  uint      strxfrm_multiply;
  uchar     caseup_multiply;
  uchar     casedn_multiply;
  uint      mbminlen;
  uint      mbmaxlen;
  uint      mbmaxlenlen;
  my_wc_t   min_sort_char;
  my_wc_t   max_sort_char; /* For LIKE optimization */
  uchar     pad_char;
  my_bool   escape_with_backslash_is_dangerous;
  uchar     levels_for_compare;
  uchar     levels_for_order;

  MY_CHARSET_HANDLER *cset;
  MY_COLLATION_HANDLER *coll;

} CHARSET_INFO;


extern CHARSET_INFO *get_charset(uint cs_number, myf flags);
extern CHARSET_INFO *get_charset_by_name(const char *cs_name, myf flags);
extern CHARSET_INFO *my_collation_get_by_name(MY_CHARSET_LOADER *loader,
                                              const char *name, myf flags);
extern CHARSET_INFO *get_charset_by_csname(const char *cs_name,
                                           uint cs_flags, myf my_flags);

extern CHARSET_INFO *default_charset_info;
extern CHARSET_INFO my_charset_latin1;

#define use_mb(s)                     ((s)->cset->ismbchar != NULL)
#define my_ismbchar(s, a, b)          ((s)->cset->ismbchar((s), (a), (b)))
#define my_mbcharlen(s, a)            ((s)->cset->mbcharlen((s),(a)))


/* Mutexes, threads etc. */

#ifdef _WIN32
typedef DWORD thread_local_key_t;
typedef CRITICAL_SECTION native_mutex_t;
typedef int native_mutexattr_t;
#else
typedef pthread_key_t thread_local_key_t;
typedef pthread_mutex_t native_mutex_t;
typedef pthread_mutexattr_t native_mutexattr_t;
#endif

static inline int native_mutex_init(native_mutex_t *mutex,
                                    const native_mutexattr_t *attr)
{
#ifdef _WIN32
  InitializeCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_init(mutex, attr);
#endif
}

static inline int native_mutex_lock(native_mutex_t *mutex)
{
#ifdef _WIN32
  EnterCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_lock(mutex);
#endif
}

static inline int native_mutex_trylock(native_mutex_t *mutex)
{
#ifdef _WIN32
  if (TryEnterCriticalSection(mutex))
  {
    /* Don't allow recursive lock */
    if (mutex->RecursionCount > 1){
      LeaveCriticalSection(mutex);
      return EBUSY;
    }
    return 0;
  }
  return EBUSY;
#else
  return pthread_mutex_trylock(mutex);
#endif
}

static inline int native_mutex_unlock(native_mutex_t *mutex)
{
#ifdef _WIN32
  LeaveCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_unlock(mutex);
#endif
}

static inline int native_mutex_destroy(native_mutex_t *mutex)
{
#ifdef _WIN32
  DeleteCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_destroy(mutex);
#endif
}

/* Debugging */
#define DBUG_ENTER(a1)
#define DBUG_LEAVE
#define DBUG_RETURN(a1)                 do { return(a1); } while(0)
#define DBUG_VOID_RETURN                do { return; } while(0)
#define DBUG_EXECUTE(keyword,a1)        do { } while(0)
#define DBUG_EXECUTE_IF(keyword,a1)     do { } while(0)
#define DBUG_EVALUATE(keyword,a1,a2) (a2)
#define DBUG_EVALUATE_IF(keyword,a1,a2) (a2)
#define DBUG_PRINT(keyword,arglist)     do { } while(0)
#define DBUG_PUTS(keyword,arg)          do { } while(0)
#define DBUG_LOG(keyword,arglist)       do { } while(0)
#define DBUG_PUSH(a1)                   do { } while(0)
#define DBUG_SET(a1)                    do { } while(0)
#define DBUG_SET_INITIAL(a1)            do { } while(0)
#define DBUG_POP()                      do { } while(0)
#define DBUG_PROCESS(a1)                do { } while(0)
#define DBUG_SETJMP(a1) setjmp(a1)
#define DBUG_LONGJMP(a1) longjmp(a1)
#define DBUG_DUMP(keyword,a1,a2)        do { } while(0)
#define DBUG_END()                      do { } while(0)
#define DBUG_ASSERT(A)                  do { } while(0)
#define DBUG_LOCK_FILE                  do { } while(0)
#define DBUG_FILE (stderr)
#define DBUG_UNLOCK_FILE                do { } while(0)
#define DBUG_EXPLAIN(buf,len)
#define DBUG_EXPLAIN_INITIAL(buf,len)
#define DEBUGGER_OFF                    do { } while(0)
#define DEBUGGER_ON                     do { } while(0)
#define DBUG_ABORT()                    do { } while(0)
#define DBUG_CRASH_ENTER(func)
#define DBUG_CRASH_RETURN(val)          do { return(val); } while(0)
#define DBUG_CRASH_VOID_RETURN          do { return; } while(0)
#define DBUG_SUICIDE()                  do { } while(0)


#ifdef	__cplusplus
}
#endif

#endif