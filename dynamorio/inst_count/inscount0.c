#include "dr_api.h"

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

static dr_emit_flags_t
event_basic_block(void *drcontext, void *tag, instrlist_t *bb,
                  bool for_trace, bool translating)
{
    instr_t *instr, *first = instrlist_first(bb), *point = NULL;
    uint num_instrs;
	uint flags;
	uint need_restore;

	/* count instruction */
    for (instr  = first, num_instrs = 0;
         instr != NULL;
         instr = instr_get_next(instr)) {

        num_instrs++;

    }
	
	need_restore = 0;
	flags = instr_get_arith_flags(instr);
	/* eflags are not dead save eflags to register */
	if (!(TESTALL(EFLAGS_WRITE_6, flags) && !TESTANY(EFLAGS_READ_6, flags))) {
		need_restore = 1;
		dr_save_reg(drcontext, bb, first, DR_REG_XAX, SPILL_SLOT_1);
		dr_save_arith_flags_to_xax(drcontext, bb, first);
	}

	/* add the instruction count */
    instrlist_meta_preinsert
        (bb, first,
         INSTR_CREATE_add(drcontext, OPND_CREATE_ABSMEM
                               ((byte *)&global_count, OPSZ_4), OPND_CREATE_INT32(num_instrs)));

	/* Need to carry since it is a 8 byte variable. */
    instrlist_meta_preinsert
        (bb, first,
         INSTR_CREATE_adc(drcontext, OPND_CREATE_ABSMEM
                               ((byte *)&global_count + 4, OPSZ_4), OPND_CREATE_INT32(0)));

	/* resotre eflags */
	if (need_restore) {
		dr_restore_arith_flags_from_xax(drcontext, bb, first);
		dr_restore_reg(drcontext, bb, first, DR_REG_XAX, SPILL_SLOT_1);
	}

    return DR_EMIT_DEFAULT;
}
