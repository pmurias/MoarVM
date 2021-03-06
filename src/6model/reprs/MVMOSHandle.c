#include "moarvm.h"

/* This representation's function pointer table. */
static MVMREPROps *this_repr;

/* Creates a new type object of this representation, and associates it with
 * the given HOW. */
static MVMObject * type_object_for(MVMThreadContext *tc, MVMObject *HOW) {
    MVMSTable *st  = MVM_gc_allocate_stable(tc, this_repr, HOW);
    MVMObject *obj = MVM_gc_allocate_type_object(tc, st);
    MVM_ASSIGN_REF(tc, st, st->WHAT, obj);
    st->size = sizeof(MVMOSHandle);
    return st->WHAT;
}

/* Creates a new instance based on the type object. */
static MVMObject * allocate(MVMThreadContext *tc, MVMSTable *st) {
    return MVM_gc_allocate_object(tc, st);
}

/* Initializes a new instance. */
static void initialize(MVMThreadContext *tc, MVMSTable *st, MVMObject *root, void *data) {
}

/* Copies the body of one object to another. */
static void copy_to(MVMThreadContext *tc, MVMSTable *st, void *src, MVMObject *dest_root, void *dest) {
    /* can't be copied because then we could never know when gc_free should
     * close the handle (unless we did some refcounting on a shared container).
     * note - 12:25 <jnthn> I mean, Perl 6 will has an attribute which
     * is the MoarVM handle, so a .clone() on a Perl 6 IO object
     * won't trigger cloning of the underlying handle.            */
    MVM_exception_throw_adhoc(tc, "Cannot copy object with repr OSHandle");
}

/* Called by the VM in order to free memory associated with this object. */
static void gc_free(MVMThreadContext *tc, MVMObject *obj) {
    MVMOSHandle *handle = (MVMOSHandle *)obj;
    switch(handle->body.handle_type) {
        case MVM_OSHANDLE_UNINIT:
            break;
        case MVM_OSHANDLE_FILE:
            apr_file_close(handle->body.file_handle);
            break;
        case MVM_OSHANDLE_DIR:
            apr_dir_close(handle->body.dir_handle);
            break;
        case MVM_OSHANDLE_SOCKET:
            apr_socket_close(handle->body.socket);
            break;
    }
    apr_pool_destroy(handle->body.mem_pool);
}

/* Gets the storage specification for this representation. */
static MVMStorageSpec get_storage_spec(MVMThreadContext *tc, MVMSTable *st) {
    MVMStorageSpec spec;
    spec.inlineable      = MVM_STORAGE_SPEC_REFERENCE;
    spec.boxed_primitive = MVM_STORAGE_SPEC_BP_NONE;
    spec.can_box         = 0;
    return spec;
}

/* Compose the representation. */
static void compose(MVMThreadContext *tc, MVMSTable *st, MVMObject *info) {
    /* Nothing to do for this REPR. */
}

/* Initializes the representation. */
MVMREPROps * MVMOSHandle_initialize(MVMThreadContext *tc) {
    /* Allocate and populate the representation function table. */
    if (!this_repr) {
        this_repr = malloc(sizeof(MVMREPROps));
        memset(this_repr, 0, sizeof(MVMREPROps));
        this_repr->type_object_for = type_object_for;
        this_repr->allocate = allocate;
        this_repr->initialize = initialize;
        this_repr->copy_to = copy_to;
        this_repr->gc_free = gc_free;
        this_repr->get_storage_spec = get_storage_spec;
        this_repr->compose = compose;
    }
    return this_repr;
}
