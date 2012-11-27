#include "dr_api.h"

# define DISPLAY_STRING(msg) dr_printf("%s\n", msg);

#define NULL_TERMINATE(buf) buf[(sizeof(buf)/sizeof(buf[0])) - 1] = '\0'

#define TESTALL(mask, var) (((mask) & (var)) == (mask))
#define TESTANY(mask, var) (((mask) & (var)) != 0)

static uint64 global_count;

static void event_exit(void);
static dr_emit_flags_t event_basic_block(void *drcontext, void *tag, instrlist_t *bb,
                                         bool for_trace, bool translating);

DR_EXPORT void 
dr_init(client_id_t id)
{
    /* register events */
    dr_register_exit_event(event_exit);
    dr_register_bb_event(event_basic_block);

}

static void 
event_exit(void)
{
    char msg[512];
    int len;

    len = dr_snprintf(msg, sizeof(msg)/sizeof(msg[0]),
                      "Instrumentation results: %llu instructions executed\n",
                      global_count);
    DR_ASSERT(len > 0);
    NULL_TERMINATE(msg);
    DISPLAY_STRING(msg);
}

/* basic block instrumentation */
static dr_emit_flags_t
event_basic_block(void *drcontext, void *tag, instrlist_t *bb,
                  bool for_trace, bool translating)
{
    instr_t *instr, *first = instrlist_first(bb);
	int need_restore;

	/* save arith flag */
	dr_save_arith_flags(drcontext, bb, first, SPILL_SLOT_1);
    for (instr  = first;
         instr != NULL;
         instr = instr_get_next(instr)) {
		 need_restore = 0;

		/* add add, adc instruction for every instruction */
		instrlist_meta_preinsert
			(bb, first,
			 INSTR_CREATE_add(drcontext, OPND_CREATE_ABSMEM
								   ((byte *)&global_count, OPSZ_4), OPND_CREATE_INT32(1)));
		instrlist_meta_preinsert
			(bb, first,
			 INSTR_CREATE_adc(drcontext, OPND_CREATE_ABSMEM
								   ((byte *)&global_count + 4, OPSZ_4), OPND_CREATE_INT32(0)));
	}
	dr_restore_arith_flags(drcontext, bb, first, SPILL_SLOT_1);
	/* restore arith flag */
    return DR_EMIT_DEFAULT;
}
