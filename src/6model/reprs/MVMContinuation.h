/* Representation for a continuation in the VM. */
struct MVMContinuationBody {
    /* Top frame of the continuation. */
    MVMFrame *top;

    /* Address to resume execution at when the continuation is invoked. */
    MVMuint8 *addr;

    /* Register to put invoke argument into after resume. */
    MVMRegister *res_reg;

    /* Root of the continuation. */
    MVMFrame *root;

    /* Active exception handler(s) to restore. */
    MVMActiveHandler *active_handlers;
};
struct MVMContinuation {
    MVMObject common;
    MVMContinuationBody body;
};

/* Function for REPR setup. */
const MVMREPROps * MVMContinuation_initialize(MVMThreadContext *tc);
