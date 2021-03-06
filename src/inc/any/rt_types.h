/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>
#include <any/errno.h>
#include <any/list.h>
#include <any/task.h>
#include <any/timer.h>

// Forward declarations.
struct aactor_t;

// Specify the operation to be performed by the instructions.
typedef enum {
    AOC_NOP = 0,
    AOC_POP = 1,

    AOC_LDK = 10,
    AOC_NIL = 11,
    AOC_LDB = 12,
    AOC_LSI = 13,
    AOC_LLV = 14,
    AOC_SLV = 15,
    AOC_IMP = 16,
    AOC_CLS = 17,

    AOC_JMP = 30,
    AOC_JIN = 31,

    AOC_IVK = 40,
    AOC_RET = 41,

    AOC_SND = 50,
    AOC_RCV = 51,
    AOC_RMV = 52,
    AOC_RWD = 53
} aopcode_t;

/** Base type.
\rst
======  =======
8 bits  24 bits
======  =======
opcode  payload
======  =======
\endrst
*/
typedef struct {
    uint32_t opcode : 8;
    uint32_t _ : 24;
} ai_base_t;

/** No operation
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_NOP  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_nop_t;

/** Pop `n` elements from the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_POP  n
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t n : 24;
} ai_pop_t;

/** Push a constant from const pool at `idx` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LDK  idx
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_ldk_t;

/** Push a nil value onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_NIL  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_nil_t;

/** Push a bool value `val` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LDB  val
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t val : 24;
} ai_ldb_t;

/** Push a small integer value (24 bits) `val` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LSI  val
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t val : 24;
} ai_lsi_t;

/** Push a stack value at `idx` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LLV  idx
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_llv_t;

/** Pop a value from the stack and set it to `idx`.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_SLV  idx
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_slv_t;

/** Push a value from import pool at `idx` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_IMP  idx
=======  =======
\endrst
*/
typedef struct  {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_imp_t;

/** Make a closure of prototype at `idx` and push it onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_cls  idx
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_cls_t;

/** Unconditional jump, with signed `displacement`.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_JMP  displacement
=======  ============
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t displacement : 24;
} ai_jmp_t;

/** Conditional jump, with signed `displacement`.
\brief Pop a boolean value from the stack and jump if it's nil or false.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_JIN  displacement
=======  ============
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t displacement : 24;
} ai_jin_t;

/** Function call.
\brief Please refer \ref aactor_t for the protocol.
\note Result will be placed on top of the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_IVK  nargs
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t nargs : 24;
} ai_ivk_t;

/** Returning from a function call.
\brief Top of the stack will be returned as the result.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_RET  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_ret_t;

/** Pop message and next a target pid from the stack and send it.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_SND  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_snd_t;

/** Picks up next message in the queue and replace top of the stack.
\brief If there is no message, jump to signed `displacement`.
A `timeout` microseconds value will be popped from the stack, that will be used
by AVM to determine if it must wait for this period before it decided that there
is no more message in the queue. This `timeout` is in microseconds.
\note This instruction will not remove messages from the queue.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_RCV  displacement
=======  ============
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t displacement : 24;
} ai_rcv_t;

#define AINFINITE  (-1)
#define ADONT_WAIT (0)

static AINLINE aint_t amsec(aint_t ms)
{
    return ms*1000000;
}

/** Remove current message which is previously peeked by \ref ai_rcv_t.
\brief The peek pointer will be rewound to the front.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_RMV  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_rmv_t;

/** Rewind the peek pointer to the front.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_RMV  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_rwd_t;

/// Variant of instruction types, instruction size is fixed 4 bytes.
typedef union {
    ai_base_t b;
    ai_nop_t nop;
    ai_pop_t pop;
    ai_ldk_t ldk;
    ai_nil_t nil;
    ai_ldb_t ldb;
    ai_lsi_t lsi;
    ai_llv_t llv;
    ai_slv_t slv;
    ai_imp_t imp;
    ai_cls_t cls;
    ai_jmp_t jmp;
    ai_jin_t jin;
    ai_ivk_t ivk;
    ai_ret_t ret;
    ai_snd_t snd;
    ai_rcv_t rcv;
    ai_rmv_t rmv;
    ai_rwd_t rwd;
} ainstruction_t;

ASTATIC_ASSERT(sizeof(ainstruction_t) == 4);

// Instruction constructors.
static AINLINE ainstruction_t ai_nop()
{
    ainstruction_t i;
    i.b.opcode = AOC_NOP;
    return i;
}

static AINLINE ainstruction_t ai_pop(aint_t n)
{
    ainstruction_t i;
    i.b.opcode = AOC_POP;
    i.pop.n = (int32_t)n;
    return i;
}

static AINLINE ainstruction_t ai_ldk(aint_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_LDK;
    i.ldk.idx = (int32_t)idx;
    return i;
}

static AINLINE ainstruction_t ai_nil()
{
    ainstruction_t i;
    i.b.opcode = AOC_NIL;
    return i;
}

static AINLINE ainstruction_t ai_ldb(int32_t val)
{
    ainstruction_t i;
    i.b.opcode = AOC_LDB;
    i.ldb.val = val;
    return i;
}

static AINLINE ainstruction_t ai_lsi(aint_t val)
{
    ainstruction_t i;
    i.b.opcode = AOC_LSI;
    i.lsi.val = (int32_t)val;
    return i;
}

static AINLINE ainstruction_t ai_llv(aint_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_LLV;
    i.llv.idx = (int32_t)idx;
    return i;
}

static AINLINE ainstruction_t ai_slv(aint_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_SLV;
    i.slv.idx = (int32_t)idx;
    return i;
}

static AINLINE ainstruction_t ai_imp(aint_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_IMP;
    i.imp.idx = (int32_t)idx;
    return i;
}

static AINLINE ainstruction_t ai_cls(aint_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_CLS;
    i.cls.idx = (int32_t)idx;
    return i;
}

static AINLINE ainstruction_t ai_jmp(aint_t displacement)
{
    ainstruction_t i;
    i.b.opcode = AOC_JMP;
    i.jmp.displacement = (int32_t)displacement;
    return i;
}

static AINLINE ainstruction_t ai_jin(aint_t displacement)
{
    ainstruction_t i;
    i.b.opcode = AOC_JIN;
    i.jin.displacement = (int32_t)displacement;
    return i;
}

static AINLINE ainstruction_t ai_ivk(aint_t nargs)
{
    ainstruction_t i;
    i.b.opcode = AOC_IVK;
    i.ivk.nargs = (int32_t)nargs;
    return i;
}

static AINLINE ainstruction_t ai_ret()
{
    ainstruction_t i;
    i.b.opcode = AOC_RET;
    return i;
}

static AINLINE ainstruction_t ai_snd()
{
    ainstruction_t i;
    i.b.opcode = AOC_SND;
    return i;
}

static AINLINE ainstruction_t ai_rcv(aint_t displacement)
{
    ainstruction_t i;
    i.b.opcode = AOC_RCV;
    i.rcv.displacement = (int32_t)displacement;
    return i;
}

static AINLINE ainstruction_t ai_rmv()
{
    ainstruction_t i;
    i.b.opcode = AOC_RMV;
    return i;
}

static AINLINE ainstruction_t ai_rwd()
{
    ainstruction_t i;
    i.b.opcode = AOC_RWD;
    return i;
}

/** Allocator interface.
\brief
`old` = 0 to malloc,
`sz`  = 0 to free,
otherwise realloc.
*/
typedef void* (*aalloc_t)(void* ud, void* old, aint_t sz);

/// Process identifier.
typedef uint32_t apid_t;
typedef uint32_t apid_idx_t;
typedef uint32_t apid_gen_t;
enum { APID_BITS = sizeof(apid_t)*8 };

/// Make a pid from index and generation.
static AINLINE apid_t apid_from(
    int8_t idx_bits, int8_t gen_bits, apid_idx_t idx, apid_gen_t gen)
{
    return
        (idx << (APID_BITS - idx_bits)) |
        (gen << (APID_BITS - idx_bits - gen_bits));
}

/// Get index part.
static AINLINE apid_idx_t apid_idx(int8_t idx_bits, apid_t pid)
{
    return pid >> (APID_BITS - idx_bits);
}

/// Get generation part.
static AINLINE apid_gen_t apid_gen(int8_t idx_bits, int8_t gen_bits, apid_t pid)
{
    return (pid << idx_bits) >> (APID_BITS - gen_bits);
}

/// Native function.
typedef void(*anative_func_t)(struct aactor_t*);

// Constant types.
typedef enum {
    ACT_INTEGER,
    ACT_STRING,
    ACT_REAL
} actype_t;

#pragma pack(push, 1)

/// Prototype constant.
typedef struct APACKED {
    uint32_t type;
    /// ACT_INTEGER.
    aint_t integer;
    /// ACT_STRING.
    aint_t string;
    /// ACT_REAL.
    areal_t real;
} aconstant_t;

#pragma pack(pop)

// Constant constructors.
static AINLINE aconstant_t ac_integer(aint_t val)
{
    aconstant_t c;
    c.type = ACT_INTEGER;
    c.integer = val;
    return c;
}

static AINLINE aconstant_t ac_string(aint_t string)
{
    aconstant_t c;
    c.type = ACT_STRING;
    c.string = string;
    return c;
}

static AINLINE aconstant_t ac_real(areal_t val)
{
    aconstant_t c;
    c.type = ACT_REAL;
    c.real = val;
    return c;
}

/// Value tags.
typedef enum {
    /// No value.
    AVT_NIL,
    /// Process identifier.
    AVT_PID,
    /// Either `true`: 1 or `false`: 0.
    AVT_BOOLEAN,
    /// Raw pointer.
    AVT_POINTER,
    /// No fractional.
    AVT_INTEGER,
    /// Floating-point number.
    AVT_REAL,
    /// C lib function.
    AVT_NATIVE_FUNC,
    /// Byte code function.
    AVT_BYTE_CODE_FUNC,
    /// Collectable fixed size buffer.
    AVT_FIXED_BUFFER,
    /// Collectable buffer.
    AVT_BUFFER,
    /// Collectable string.
    AVT_STRING,
    /// Collectable tuple.
    AVT_TUPLE,
    /// Collectable array.
    AVT_ARRAY,
    /// Collectable table.
    AVT_TABLE
} atype_t;

/// Value tag.
typedef struct {
    int8_t type;
    int8_t collectable;
    int8_t _[2];
} avalue_tag_t;

/// Tagged value.
typedef struct {
    avalue_tag_t tag;
    union {
        /// \ref AVT_PID.
        apid_t pid;
        /// \ref AVT_BOOLEAN.
        int32_t boolean;
        /// \ref AVT_POINTER.
        void* ptr;
        /// \ref AVT_INTEGER.
        aint_t integer;
        /// \ref AVT_REAL.
        areal_t real;
        /// \ref AVT_NATIVE.
        anative_func_t func;
        /// \ref AVT_AVM.
        struct aprototype_t* avm_func;
        /// collectable value.
        aint_t heap_idx;
    } v;
} avalue_t;

// Value constructors.
static AINLINE void av_nil(avalue_t* v)
{
    v->tag.type = AVT_NIL;
    v->tag.collectable = FALSE;
}

static AINLINE void av_pid(avalue_t* v, apid_t pid)
{
    v->tag.type = AVT_PID;
    v->tag.collectable = FALSE;
    v->v.pid = pid;
}

static AINLINE void av_boolean(avalue_t* v, int32_t b)
{
    v->tag.type = AVT_BOOLEAN;
    v->tag.collectable = FALSE;
    v->v.boolean = b;
}

static AINLINE void av_pointer(avalue_t* v, void* p)
{
    v->tag.type = AVT_POINTER;
    v->tag.collectable = FALSE;
    v->v.ptr = p;
}

static AINLINE void av_integer(avalue_t* v, aint_t i)
{
    v->tag.type = AVT_INTEGER;
    v->tag.collectable = FALSE;
    v->v.integer = i;
}

static AINLINE void av_real(avalue_t* v, areal_t r)
{
    v->tag.type = AVT_REAL;
    v->tag.collectable = FALSE;
    v->v.real = r;
}

static AINLINE void av_native_func(avalue_t* v, anative_func_t f)
{
    v->tag.type = AVT_NATIVE_FUNC;
    v->tag.collectable = FALSE;
    v->v.func = f;
}

static AINLINE void av_byte_code_func(avalue_t* v, struct aprototype_t* f)
{
    v->tag.type = AVT_BYTE_CODE_FUNC;
    v->tag.collectable = FALSE;
    v->v.avm_func = f;
}

static AINLINE void av_collectable(avalue_t* v, atype_t type, aint_t heap_idx)
{
    v->tag.type = type;
    v->tag.collectable = TRUE;
    v->v.heap_idx = heap_idx;
}

/// Garbage collector.
typedef struct {
    aalloc_t alloc;
    void* alloc_ud;
    uint8_t* cur_heap;
    uint8_t* new_heap;
    aint_t heap_cap;
    aint_t heap_sz;
    aint_t scan;
} agc_t;

/// Collectable value header.
typedef struct {
    atype_t type;
    aint_t sz;
    aint_t forwared;
} agc_header_t;

#define AGC_CAST(T, gc, idx) \
    ((T*)((gc)->cur_heap + idx + sizeof(agc_header_t)))

/// Collectable buffer.
typedef struct {
    avalue_t buff;
    aint_t cap;
    aint_t sz;
} agc_buffer_t;

/// Combination of hash and length.
typedef struct {
    uint32_t hash;
    aint_t length;
} ahash_and_length_t;

/// Collectable string.
typedef struct {
    ahash_and_length_t hal;
} agc_string_t;

/// Collectable tuple.
typedef struct {
    aint_t sz;
} agc_tuple_t;

/// Collectable array.
typedef struct {
    avalue_t buff;
    aint_t cap;
    aint_t sz;
} agc_array_t;

/// Collectable map.
typedef struct {
    avalue_t buff;
    aint_t cap;
    aint_t sz;
} agc_map_t;

#pragma pack(push, 1)

/** Byte code chunk.
\brief Layout: [`header`] [`module prototype`].
AVM is dynamic, stack based abstract machine which rely on a strong coupled,
relocatable and recursive intermediate representation called `byte code chunk`.
That is designed to be loaded and executed directly from the storages with as
least effort as possible. The byte code chunk includes everything we need for
a module to be executed except the external value (called import). This chunk
is portable in theory. However, there is no need to actually do that in real-
life. AVM will not even try to make any effort to load an incompatible chunks.
Of course, if we really **need** this as a feature, byte code rewriter could
be implemented as another tool.
\par Header format.
\rst
=====  ===================  ===================
bytes  description          value
=====  ===================  ===================
4      signature            0x41 0x6E 0x79 0x00
1      version number       0x{MAJOR}{MINOR}
1      is big endian?       1 = big, 0 = little
1      size of integer      default 8 bytes
1      size of float        default 8 bytes
1      size of instruction  always 4
3      _                    _
=====  ===================  ===================
\endrst
\note The header portion is not affected by endianess.
*/
typedef struct APACKED {
    uint8_t  signature[4];
    uint32_t major_ver       : 4;
    uint32_t minor_ver       : 4;
    uint32_t big_endian      : 8;
    uint32_t size_of_integer : 8;
    uint32_t size_of_float   : 8;
    uint8_t  size_of_instruction;
    uint8_t  _[3];
} achunk_header_t;

#pragma pack(pop)

ASTATIC_ASSERT(sizeof(achunk_header_t) == 12);

/// Function import.
typedef struct {
    aint_t module;
    aint_t name;
} aimport_t;

static AINLINE aimport_t aimport(aint_t module, aint_t name)
{
    aimport_t i;
    i.module = module;
    i.name = name;
    return i;
}

/** Function prototype.
\brief
This struct represents the header portion of a function prototype. Prototype is
self-contained and relocatable, references are relative to their corresponding
header. That means we can safety copy a memory blob of prototype and move that
around, save to and loaded from disk without any need for patching.

The first prototype in chunk is `module prototype`, there are no global scope in
AVM. The most outer scope is `module`, which is only consists of nested function
and constant. There are no instruction, up-value, argument, local var and import
in this kind of prototype. Either that make non-sense or strictly prohibited to
prevent the possibility of race condition. Nested of module prototype is `module
function`. Which can be exported by the name specified in `symbol`. That allows
such functions to be imported to use by other modules.

In AVM, there are two kind of string, `static` and `collectable`. Compile time
string are `static` and must be referenced by `astring_ref_t` which is just an
offset to the dedicated **per prototype** string pool. We don't want to share
these pools between prototype to make relocating trivial. General speaking, the
whole prototype is read-only, which is only accessible using related instruction
to ensure the thread safety and security, we could easily mmap a prototype from
storages, as well as traditional OS executable.

\par Memory layout:
\rst
====================================  ======================
bytes                                 description
====================================  ======================
sizeof(aint_t)                        source file name
sizeof(aint_t)                        to be exported
sizeof(aint_t)                        num of string bytes
sizeof(aint_t)                        num of instructions
sizeof(aint_t)                        num of nesteds
sizeof(aint_t)                        num of constants
sizeof(aint_t)                        num of imports
num of string bytes                   strings
n * sizeof(:c:type:`ainstruction_t`)  instructions
n * sizeof(:c:type:`aconstant_t`)     constants
n * sizeof(:c:type:`aimport_t`)       imports
_                                     nested prototypes
====================================  ======================
\endrst
*/
typedef struct {
    aint_t source;
    aint_t symbol;
    aint_t strings_sz;
    aint_t num_instructions;
    aint_t num_nesteds;
    aint_t num_constants;
    aint_t num_imports;
} aprototype_header_t;

/// Lib function.
typedef struct {
    const char* name;
    anative_func_t func;
} alib_func_t;

/// Lib module.
typedef struct alib_t {
    const char* name;
    const alib_func_t* funcs;
    alist_node_t node;
} alib_t;

/// Runtime prototype.
typedef struct aprototype_t {
    struct achunk_t* chunk;
    aprototype_header_t* header;
    const char* strings;
    ainstruction_t* instructions;
    aconstant_t* constants;
    aimport_t* imports;
    struct aprototype_t* nesteds;
    avalue_t* import_values;
} aprototype_t;

/// Runtime byte code chunk.
typedef struct achunk_t {
    achunk_header_t* header;
    aalloc_t alloc;
    void* alloc_ud;
    avalue_t* imports;
    aprototype_t* prototypes;
    alist_node_t node;
    int32_t retain;
} achunk_t;

/// On linking failed handler.
typedef void(*aon_unresolved_t)(const char* m, const char* n);

/** Byte code loader.
\brief
AVM byte code loading and linking is done by `aloader_t`, with heavily focused
to be able to support `hot reloading`. That means we can introduce a new version
of already loaded module on-the-fly, without the need of shut down the system.
There are 3 state of chunk which is managed by loader, `pending`, `running` and
`garbage`. Newly added chunk is `pending`, which is invisible until successfully
be linked with other chunk and get its imports resolved. `running` is where the
latest chunks are stored, `aloader_find` will try to search in there. The last
state is `garbage`, as the name suggested, is out-of-date but still be there so
already referenced may continue to work. `aloader_sweep` could be used to free
these chunks.
*/
typedef struct {
    aalloc_t alloc;
    void* alloc_ud;
    alist_t pendings;
    alist_t runnings;
    alist_t garbages;
    alist_t libs;
    aon_unresolved_t on_unresolved;
} aloader_t;

/// Value stack.
typedef struct {
    aalloc_t alloc;
    void* alloc_ud;
    avalue_t* v;
    aint_t sp;
    aint_t cap;
} astack_t;

/// Process flags.
typedef enum {
    APF_EXIT = 1 << 0
} APFLAGS;

/// Process stack frame.
typedef struct aframe_t {
    struct aframe_t* prev;
    aprototype_t* pt;
    aint_t ip;
    aint_t bp;
    aint_t nargs;
} aframe_t;

/// Error catching points.
typedef struct acatch_t {
    struct acatch_t* prev;
    volatile aerror_t status;
    jmp_buf jbuff;
} acatch_t;

/** The actor.
\brief
AVM concurrency is rely on `actor model`, which is inspired by Erlang. In this
model, each concurrent task will be represented as a `aactor_t`. Which is, in
general have isolated state, that means such state can not be touched by other
actor. The only way an actor can affect each others is through message, to tell
the owner of that state to make the modification by itself. Therefore, we avoid
the need for explicitly locking to the state and lot of consequence trouble like
deadlock. Well, frankly speaking, deadlock still be possible if there are actors
that is at the same time blocking wait for messages from each other, but mostly
that remains easier to deal with. AVM actor is also light weight in compared
to most of OS threads. The overhead in memory footprint, creation, termination
and scheduling is low. Therefore, a massive amount of actor could be spawned.

\par Virtual stack.
Besides of the native C stack, AVM also using a dedicated, dynamic sized stack
for `avalue_t`. Indexing for this stack is relative to current function frame.
Negative value is used to point to the argument, that must be passed in reversed
order. Underflow always result as a nil value.

\rst
=====  ===========  =======
index  description  pointer
=====  ===========  =======
 4     overflow     sp
 3     local var3   top
 2     local var2
 1     local var1
 0     local var0   bp
-1     argument 1
-2     argument 2
-3     argument 3
-4     nil
-5     nil
=====  ===========  =======
\endrst
*/
typedef struct aactor_t {
    int32_t flags;
    aalloc_t alloc;
    void* alloc_ud;
    struct ascheduler_t* owner;
    acatch_t* error_jmp;
    aframe_t* frame;
    astack_t stack;
    astack_t msbox;
    aint_t msg_pp;
    agc_t gc;
} aactor_t;

/// Process task.
typedef struct {
    alist_node_t node;
    atask_t task;
} aprocess_task_t;

/// Light-weight process.
typedef struct {
    int32_t dead;
    apid_t pid;
    aactor_t actor;
    aprocess_task_t ptask;
    aint_t wait_for;
    int32_t msg_wake;
} aprocess_t;

/// Fatal error handler.
typedef void(*aon_panic_t)(struct aactor_t*);

/** Process scheduler.
\brief
AVM is designed to be resumable, that sounds tricky and non-portable at first.
However, we believe that its worth. Generally, we don't want the users to worry
about whether C stack is resumable or not, and if not then what happen. Yielding
native function and across native function is just mandatory use cases in AVM.
A new \ref atask_t is required for each new actor. That allows AVM to save the
context of a actor and comeback later, in native side.
*/
typedef struct ascheduler_t {
    aalloc_t alloc;
    void* alloc_ud;
    aprocess_t* procs;
    aint_t num_procs;
    aloader_t loader;
    int8_t idx_bits;
    int8_t gen_bits;
    apid_idx_t next_idx;
    aprocess_task_t root;
    alist_t pendings;
    alist_t runnings;
    alist_t waitings;
    atimer_t timer;
    int32_t first_run;
    aon_panic_t on_panic;
} ascheduler_t;
