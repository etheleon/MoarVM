#include "moar.h"
#include "../../math/littlebigintsupport.h"

/* This representation's function pointer table. */
static const MVMREPROps this_repr;


/* Creates a new type object of this representation, and associates it with
 * the given HOW. */
static MVMObject * type_object_for(MVMThreadContext *tc, MVMObject *HOW) {
    MVMSTable *st  = MVM_gc_allocate_stable(tc, &this_repr, HOW);

    MVMROOT(tc, st, {
        MVMObject *obj = MVM_gc_allocate_type_object(tc, st);
        MVM_ASSIGN_REF(tc, st, st->WHAT, obj);
        st->size = sizeof(MVMP6bigint);
    });

    return st->WHAT;
}

/* Creates a new instance based on the type object. */
static MVMObject * allocate(MVMThreadContext *tc, MVMSTable *st) {
    return MVM_gc_allocate_object(tc, st);
}

/* Initializes a new instance. */
static void initialize(MVMThreadContext *tc, MVMSTable *st, MVMObject *root, void *data) {
    MVMP6bigintBody *body = (MVMP6bigintBody *)data;
    MAKE_SBI(body);
    STORE_SBI(body, 0);
}

/* Copies the body of one object to another. */
static void copy_to(MVMThreadContext *tc, MVMSTable *st, void *src, MVMObject *dest_root, void *dest) {
    MVMP6bigintBody *src_body = (MVMP6bigintBody *)src;
    MVMP6bigintBody *dest_body = (MVMP6bigintBody *)dest;
    if (!IS_SBI(src))
        mp_init_copy(&dest_body->i, &src_body->i);
}

static void set_int(MVMThreadContext *tc, MVMSTable *st, MVMObject *root, void *data, MVMint64 value) {
    mp_int *i = &((MVMP6bigintBody *)data)->i;

    if (0 && (value < -0xffffffff || value > 0x7fffffff)) {
        printf("making a littlebigint in set_int: %d\n", value);
        if (!IS_SBI(data)) {
            mp_clear(i);
            MAKE_SBI(data);
        }
        MVMP6bigintBody *body = ((MVMP6bigintBody *)data);
        body->i.sign = 0;
        body->i.used = 0;
        body->i.alloc = 0;
        STORE_SBI(data, value);
    } else {
        if (IS_SBI(data)) {
            mp_init(i);
            mp_zero(i);
            if (value >= 0) {
                mp_set_long(i, value);
            } else {
                mp_set_long(i, -value);
                mp_neg(i, i);
            }
        }
    }
}
static MVMint64 get_int(MVMThreadContext *tc, MVMSTable *st, MVMObject *root, void *data) {
    MVMint64 ret;
    mp_int *i = &((MVMP6bigintBody *)data)->i;
    if (IS_SBI(data)) {
        printf("get_int on a SBI: %d\n", VALUE_SBI(data));
        return VALUE_SBI(data);
    } else {
        if (MP_LT == mp_cmp_d(i, 0)) {
            mp_neg(i, i);
            ret = mp_get_long(i);
            mp_neg(i, i);
            return -ret;
        }
        else {
            return mp_get_long(i);
        }
    }
}

static void * get_boxed_ref(MVMThreadContext *tc, MVMSTable *st, MVMObject *root, void *data, MVMuint32 repr_id) {
    if (repr_id == MVM_REPR_ID_P6bigint)
        return &((MVMP6bigintBody *)data)->i;

    MVM_exception_throw_adhoc(tc,
        "P6bigint representation cannot unbox to other types");
}

/* Gets the storage specification for this representation. */
static MVMStorageSpec get_storage_spec(MVMThreadContext *tc, MVMSTable *st) {
    MVMStorageSpec spec;
    spec.inlineable      = MVM_STORAGE_SPEC_INLINED;
    spec.bits            = sizeof(mp_int) * 8;
    spec.boxed_primitive = MVM_STORAGE_SPEC_BP_INT;
    spec.can_box         = MVM_STORAGE_SPEC_CAN_BOX_INT;
    return spec;
}

/* Compose the representation. */
static void compose(MVMThreadContext *tc, MVMSTable *st, MVMObject *info) {
    setup_littlebigint();
}

static void gc_cleanup(MVMThreadContext *tc, MVMSTable *st, void *data) {
    if(!IS_SBI(data)) {
        mp_clear(&((MVMP6bigintBody *)data)->i);
    }
}

static void gc_free(MVMThreadContext *tc, MVMObject *obj) {
    if(!IS_SBI(obj)) {
        mp_clear(&((MVMP6bigintBody *)obj)->i);
    }
}

/* Serializes the bigint. */
static void serialize(MVMThreadContext *tc, MVMSTable *st, void *data, MVMSerializationWriter *writer) {
    OBTAIN_BIN((MVMP6littlebigintBody *)data, i);
    int len;
    char *buf;
    MVMString *str;
    mp_radix_size(i, 10, &len);
    buf = (char *)malloc(len);
    mp_toradix(i, buf, 10);

    /* len - 1 because buf is \0-terminated */
    str = MVM_string_ascii_decode(tc, tc->instance->VMString, buf, len - 1);

    writer->write_str(tc, writer, str);
    free(buf);
    CLEANUP_BIN((MVMP6littlebigintBody *)data, i);
}

/* Set the size on the STable. */
static void deserialize_stable_size(MVMThreadContext *tc, MVMSTable *st, MVMSerializationReader *reader) {
    st->size = sizeof(MVMP6bigint);
}

/* Deserializes the bigint. */
static void deserialize(MVMThreadContext *tc, MVMSTable *st, MVMObject *root, void *data, MVMSerializationReader *reader) {
    MVMP6bigintBody *body = (MVMP6bigintBody *)data;
    MVMuint64 output_size;
    const char *buf = MVM_string_ascii_encode(tc, reader->read_str(tc, reader), &output_size);
    mp_init(&body->i);
    mp_read_radix(&body->i, buf, 10);
    /*if (IS_SBI(data)) {*/
        /*force_littlebigint(data);*/
    /*}*/
}

/* Initializes the representation. */
const MVMREPROps * MVMP6bigint_initialize(MVMThreadContext *tc) {
    return &this_repr;
}

static const MVMREPROps this_repr = {
    type_object_for,
    allocate,
    initialize,
    copy_to,
    MVM_REPR_DEFAULT_ATTR_FUNCS,
    {
        set_int,
        get_int,
        MVM_REPR_DEFAULT_SET_NUM,
        MVM_REPR_DEFAULT_GET_NUM,
        MVM_REPR_DEFAULT_SET_STR,
        MVM_REPR_DEFAULT_GET_STR,
        get_boxed_ref
    },    /* box_funcs */
    MVM_REPR_DEFAULT_POS_FUNCS,
    MVM_REPR_DEFAULT_ASS_FUNCS,
    MVM_REPR_DEFAULT_ELEMS,
    get_storage_spec,
    NULL, /* change_type */
    serialize,
    deserialize,
    NULL, /* serialize_repr_data */
    NULL, /* deserialize_repr_data */
    deserialize_stable_size,
    NULL, /* gc_mark */
    gc_free,
    gc_cleanup,
    NULL, /* gc_mark_repr_data */
    NULL, /* gc_free_repr_data */
    compose,
    "P6bigint", /* name */
    MVM_REPR_ID_P6bigint,
    0, /* refs_frames */
};
