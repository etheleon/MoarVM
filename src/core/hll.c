#include "moar.h"

MVMHLLConfig *MVM_hll_get_config_for(MVMThreadContext *tc, MVMString *name) {
    void *kdata;
    MVMHLLConfig *entry;
    size_t klen;

    MVM_HASH_EXTRACT_KEY(tc, &kdata, &klen, name, "get hll config needs concrete string");

    uv_mutex_lock(&tc->instance->mutex_hllconfigs);

    if (tc->instance->hll_compilee_depth)
        HASH_FIND(hash_handle, tc->instance->compilee_hll_configs, kdata, klen, entry);
    else
        HASH_FIND(hash_handle, tc->instance->compiler_hll_configs, kdata, klen, entry);

    if (!entry) {
        entry = calloc(sizeof(MVMHLLConfig), 1);
        entry->name = name;
        entry->int_box_type = tc->instance->boot_types.BOOTInt;
        entry->num_box_type = tc->instance->boot_types.BOOTNum;
        entry->str_box_type = tc->instance->boot_types.BOOTStr;
        entry->slurpy_array_type = tc->instance->boot_types.BOOTArray;
        entry->slurpy_hash_type = tc->instance->boot_types.BOOTHash;
        entry->array_iterator_type = tc->instance->boot_types.BOOTIter;
        entry->hash_iterator_type = tc->instance->boot_types.BOOTIter;
        entry->foreign_type_int = tc->instance->boot_types.BOOTInt;
        entry->foreign_type_num = tc->instance->boot_types.BOOTNum;
        entry->foreign_type_str = tc->instance->boot_types.BOOTStr;
        entry->foreign_transform_array = NULL;
        entry->foreign_transform_hash = NULL;
        entry->foreign_transform_code = NULL;
        entry->null_value = NULL;
        entry->exit_handler = NULL;
        entry->bind_error = NULL;
        entry->method_not_found_error = NULL;
        if (tc->instance->hll_compilee_depth)
            HASH_ADD_KEYPTR(hash_handle, tc->instance->compilee_hll_configs, kdata, klen, entry);
        else
            HASH_ADD_KEYPTR(hash_handle, tc->instance->compiler_hll_configs, kdata, klen, entry);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->int_box_type);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->num_box_type);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->str_box_type);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->slurpy_array_type);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->slurpy_hash_type);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->array_iterator_type);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->hash_iterator_type);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->foreign_type_int);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->foreign_type_num);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->foreign_type_str);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->foreign_transform_array);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->foreign_transform_hash);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->foreign_transform_code);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->null_value);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->exit_handler);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->bind_error);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->method_not_found_error);
        MVM_gc_root_add_permanent(tc, (MVMCollectable **)&entry->name);
    }

    uv_mutex_unlock(&tc->instance->mutex_hllconfigs);

    return entry;
}

#define check_config_key(tc, hash, name, member, config) do { \
    MVMString *key = MVM_string_utf8_decode((tc), (tc)->instance->VMString, (name), strlen((name))); \
    MVMObject *val = MVM_repr_at_key_o((tc), (hash), key); \
    if (val) (config)->member = val; \
} while (0)

MVMObject * MVM_hll_set_config(MVMThreadContext *tc, MVMString *name, MVMObject *config_hash) {
    MVMHLLConfig *config;

    config = MVM_hll_get_config_for(tc, name);

    if (!config_hash || REPR(config_hash)->ID != MVM_REPR_ID_MVMHash
            || !IS_CONCRETE(config_hash)) {
        MVM_exception_throw_adhoc(tc, "set hll config needs concrete hash");
    }

    /* MVM_string_utf8_decode() can potentially allocate, and hence gc. */
    MVMROOT(tc, config_hash, {
            check_config_key(tc, config_hash, "int_box", int_box_type, config);
            check_config_key(tc, config_hash, "num_box", num_box_type, config);
            check_config_key(tc, config_hash, "str_box", str_box_type, config);
            check_config_key(tc, config_hash, "slurpy_array", slurpy_array_type, config);
            check_config_key(tc, config_hash, "slurpy_hash", slurpy_hash_type, config);
            check_config_key(tc, config_hash, "array_iter", array_iterator_type, config);
            check_config_key(tc, config_hash, "hash_iter", hash_iterator_type, config);
            check_config_key(tc, config_hash, "foreign_type_int", foreign_type_int, config);
            check_config_key(tc, config_hash, "foreign_type_num", foreign_type_num, config);
            check_config_key(tc, config_hash, "foreign_type_str", foreign_type_str, config);
            check_config_key(tc, config_hash, "foreign_transform_array", foreign_transform_array, config);
            check_config_key(tc, config_hash, "foreign_transform_hash", foreign_transform_hash, config);
            check_config_key(tc, config_hash, "foreign_transform_code", foreign_transform_code, config);
            check_config_key(tc, config_hash, "null_value", null_value, config);
            check_config_key(tc, config_hash, "exit_handler", exit_handler, config);
            check_config_key(tc, config_hash, "bind_error", bind_error, config);
            check_config_key(tc, config_hash, "method_not_found_error", method_not_found_error, config);
        });

    return config_hash;
}

/* Gets the current HLL configuration. */
MVMHLLConfig *MVM_hll_current(MVMThreadContext *tc) {
    return (*tc->interp_cu)->body.hll_config;
}

/* Enter a level of compilee HLL configuration mode. */
void MVM_hll_enter_compilee_mode(MVMThreadContext *tc) {
    uv_mutex_lock(&tc->instance->mutex_hllconfigs);
    tc->instance->hll_compilee_depth++;
    uv_mutex_unlock(&tc->instance->mutex_hllconfigs);
}

/* Leave a level of compilee HLL configuration mode. */
void MVM_hll_leave_compilee_mode(MVMThreadContext *tc) {
    uv_mutex_lock(&tc->instance->mutex_hllconfigs);
    tc->instance->hll_compilee_depth--;
    uv_mutex_unlock(&tc->instance->mutex_hllconfigs);
}

/* Single object arg callsite. */
static MVMCallsiteEntry obj_arg_flags[] = { MVM_CALLSITE_ARG_OBJ };
static MVMCallsite     obj_arg_callsite = { obj_arg_flags, 1, 1, 0 };

/* Checks if an object belongs to the correct HLL, and does a type mapping
 * of it if not. */
void MVM_hll_map(MVMThreadContext *tc, MVMObject *obj, MVMHLLConfig *hll, MVMRegister *res_reg) {
    /* Null objects get mapped to null_value. */
    if (!obj) {
        res_reg->o = hll->null_value;
    }

    /* If the object belongs to the current HLL, we're done. */
    else if (STABLE(obj)->hll_owner == hll) {
        res_reg->o = obj;
    }

    /* Otherwise, need to try a mapping. */
    else {
        switch (STABLE(obj)->hll_role) {
            case MVM_HLL_ROLE_INT:
                if (hll->foreign_type_int)
                    res_reg->o = MVM_repr_box_int(tc, hll->foreign_type_int,
                        MVM_repr_get_int(tc, obj));
                else
                    res_reg->o = obj;
                break;
            case MVM_HLL_ROLE_NUM:
                if (hll->foreign_type_num)
                    res_reg->o = MVM_repr_box_num(tc, hll->foreign_type_num,
                        MVM_repr_get_num(tc, obj));
                else
                    res_reg->o = obj;
                break;
            case MVM_HLL_ROLE_STR:
                if (hll->foreign_type_str)
                    res_reg->o = MVM_repr_box_str(tc, hll->foreign_type_str,
                        MVM_repr_get_str(tc, obj));
                else
                    res_reg->o = obj;
                break;
            case MVM_HLL_ROLE_ARRAY:
                if (hll->foreign_transform_array) {
                    /* Invoke and set result register as return location. */
                    MVMObject *code = MVM_frame_find_invokee(tc, hll->foreign_transform_array, NULL);
                    MVM_args_setup_thunk(tc, res_reg, MVM_RETURN_OBJ, &obj_arg_callsite);
                    tc->cur_frame->args[0].o = obj;
                    STABLE(code)->invoke(tc, code, &obj_arg_callsite, tc->cur_frame->args);
                }
                else {
                    res_reg->o = obj;
                }
                break;
            case MVM_HLL_ROLE_HASH:
                if (hll->foreign_transform_hash) {
                    /* Invoke and set result register as return location. */
                    MVMObject *code = MVM_frame_find_invokee(tc, hll->foreign_transform_hash, NULL);
                    MVM_args_setup_thunk(tc, res_reg, MVM_RETURN_OBJ, &obj_arg_callsite);
                    tc->cur_frame->args[0].o = obj;
                    STABLE(code)->invoke(tc, code, &obj_arg_callsite, tc->cur_frame->args);
                }
                else {
                    res_reg->o = obj;
                }
                break;
            case MVM_HLL_ROLE_CODE:
                if (hll->foreign_transform_code) {
                    /* Invoke and set result register as return location. */
                    MVMObject *code = MVM_frame_find_invokee(tc, hll->foreign_transform_code, NULL);
                    MVM_args_setup_thunk(tc, res_reg, MVM_RETURN_OBJ, &obj_arg_callsite);
                    tc->cur_frame->args[0].o = obj;
                    STABLE(code)->invoke(tc, code, &obj_arg_callsite, tc->cur_frame->args);
                }
                else {
                    res_reg->o = obj;
                }
                break;
            default:
                res_reg->o = obj;
        }
    }
}

/* Looks up an object in the HLL symbols stash. */
MVMObject * MVM_hll_sym_get(MVMThreadContext *tc, MVMString *hll, MVMString *sym) {
    MVMObject *syms = tc->instance->hll_syms, *hash, *result;
    uv_mutex_lock(&tc->instance->mutex_hll_syms);
    hash = MVM_repr_at_key_o(tc, syms, hll);
    if (!hash) {
        MVMROOT(tc, hll, {
        MVMROOT(tc, syms, {
            hash = MVM_repr_alloc_init(tc, tc->instance->boot_types.BOOTHash);
        });
        });
        MVM_repr_bind_key_o(tc, syms, hll, hash);
        result = NULL;
    }
    else {
        result = MVM_repr_at_key_o(tc, hash, sym);
    }
    uv_mutex_unlock(&tc->instance->mutex_hll_syms);
    return result;
}
