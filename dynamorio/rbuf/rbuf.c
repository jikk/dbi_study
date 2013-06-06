#include <stddef.h> /* for offsetof */
#include "dr_api.h"
#include "drutil.h"

#define NULL_TERMINATE(buf) buf[(sizeof(buf)/sizeof(buf[0])) - 1] = '\0'

#define TESTALL(mask, var) (((mask) & (var)) == (mask))
#define TESTANY(mask, var) (((mask) & (var)) != 0)

#define RBUF_SIZE 0x10000
#define RBUF_MASK 0xFFFF

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

uint RBufIdx = 0;
uint RBuf[RBUF_SIZE];

static void event_exit(void);
static dr_emit_flags_t event_basic_block(void *drcontext, void *tag, instrlist_t *bb,
                                         bool for_trace, bool translating);
static void instrument_mem(void        *drcontext, 
                           instrlist_t *ilist, 
                           instr_t     *where, 
                           int          pos, 
                           bool         write);


DR_EXPORT void 
dr_init(client_id_t id)
{
    /* register events */
    dr_register_exit_event(event_exit);
    dr_register_bb_event(event_basic_block);

    /* make it easy to tell, by looking at log file, which client executed */
    dr_log(NULL, LOG_ALL, 1, "Client 'memtrace' initializing\n");
    if (dr_is_notify_on()) {
        dr_fprintf(STDERR, "Client is running\n");
    }
}

static void 
event_exit(void)
{
}

static dr_emit_flags_t
event_basic_block(void *drcontext, void *tag, instrlist_t *bb,
                  bool for_trace, bool translating)
{
	int i;
    instr_t *instr, *first = instrlist_first(bb);
    
    for (instr = first; instr != NULL; instr = instr_get_next(instr)) {
		if (instr_reads_memory(instr)) {
			for (i = 0; i < instr_num_srcs(instr); i++) {
				if (opnd_is_memory_reference(instr_get_src(instr, i))) {
					instrument_mem(drcontext, bb, instr, i, false);
				}
			}
		}
		if (instr_writes_memory(instr)) {
			for (i = 0; i < instr_num_dsts(instr); i++) {
				if (opnd_is_memory_reference(instr_get_dst(instr, i))) {
					instrument_mem(drcontext, bb, instr, i, true);
				}
			}
		}
    }
//	dr_printf("count %d\n",count);
    return DR_EMIT_DEFAULT;
}

static void
instrument_mem(void *drcontext, instrlist_t *ilist, instr_t *where, 
               int pos, bool write)
{
    instr_t *instr;
    opnd_t   ref, opnd1, opnd2;
    reg_id_t reg1 = DR_REG_XAX; /* We can optimize it by picking dead reg */
    reg_id_t reg2 = DR_REG_XCX; /* reg2 must be ECX or RCX for jecxz */

    if (write)
       ref = instr_get_dst(where, pos);
    else
       ref = instr_get_src(where, pos);

    dr_save_reg(drcontext, ilist, where, reg1, SPILL_SLOT_2);
    dr_save_reg(drcontext, ilist, where, reg2, SPILL_SLOT_3);

	// reg2 = RBufIdx
    opnd1 = opnd_create_reg(reg2);
    opnd2 = OPND_CREATE_ABSMEM((byte *)&RBufIdx, OPSZ_4);
    instr = INSTR_CREATE_mov_ld(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(ilist, where, instr);
	// save flags since we are using inc, and
	dr_save_arith_flags_to_xax(drcontext, ilist, where);	

	// reg2 = reg2 & RBUF_SIZE 
    opnd1 = opnd_create_reg(reg2);
    opnd2 = OPND_CREATE_INT32(RBUF_SIZE);
    instr = INSTR_CREATE_and(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(ilist, where, instr);
	dr_restore_arith_flags_from_xax(drcontext, ilist, where);

	// reg1 = &RBuf
    opnd1 = opnd_create_reg(reg1);
    opnd2 = OPND_CREATE_INTPTR(RBuf);
    instr = INSTR_CREATE_mov_imm(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(ilist, where, instr);

	// reg1 = reg1 + reg2 * sizeof(uint)
	// 		= RBuf + RBufIdx * sizeof(uint)
	// 		= RBuf[RBufIdx]
    opnd1 = opnd_create_reg(reg1);
    opnd2 = opnd_create_base_disp(reg1, reg2, sizeof(uint), 0, OPSZ_lea);
    instr = INSTR_CREATE_lea(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(ilist, where, instr);

	// RBuf[RBufIdx].addr = addr;
    opnd1 = OPND_CREATE_MEMPTR(reg1, 0);
    drutil_insert_get_mem_addr(drcontext, ilist, where, ref, reg2, reg1);
    opnd2 = opnd_create_reg(reg2);
    instr = INSTR_CREATE_mov_st(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(ilist, where, instr);

	dr_save_arith_flags_to_xax(drcontext, ilist, where);	

	// reg2 = RBufIdx
    opnd1 = opnd_create_reg(reg2);
    opnd2 = OPND_CREATE_ABSMEM((byte *)&RBufIdx, OPSZ_4);
    instr = INSTR_CREATE_mov_ld(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(ilist, where, instr);

	// reg2 = reg2 + 1
    opnd1 = opnd_create_reg(reg2);
    instr = INSTR_CREATE_inc(drcontext, opnd1);
    instrlist_meta_preinsert(ilist, where, instr);

	// RBufIdx = reg2
    opnd1 = OPND_CREATE_ABSMEM((byte *)&RBufIdx, OPSZ_4);
    opnd2 = opnd_create_reg(reg2);
    instr = INSTR_CREATE_mov_st(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(ilist, where, instr);

	dr_restore_arith_flags_from_xax(drcontext, ilist, where);

    dr_restore_reg(drcontext, ilist, where, reg1, SPILL_SLOT_2);
    dr_restore_reg(drcontext, ilist, where, reg2, SPILL_SLOT_3);
}
