#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

ULong total;
Int rep_count;
Addr last_rep_addr;

static void ic_post_clo_init(void)
{
}

static VG_REGPARM(1) void per_instruction_BBV(int count)
{
	total += count;
}

static VG_REGPARM(0) void per_instruction()
{
	total++;
}

/* optimized by bb version */
static
IRSB* ic_bb_instrument ( VgCallbackClosure* closure,
                      IRSB* sbIn,
                      VexGuestLayout* layout, 
                      VexGuestExtents* vge,
                      IRType gWordTy, IRType hWordTy )
{
   Int      i;
   IRSB     *sbOut;
   IRStmt   *st;
   IRDirty  *di;
   IRExpr   **argv, *arg1;
   Int		inscount = 0;

   /* Set up new SB */
   sbOut = deepCopyIRSBExceptStmts(sbIn);
   i = 0;

   /* skip if the statement is used */
   while ((i < sbIn->stmts_used) && (sbIn->stmts[i]->tag!=Ist_IMark)) {
   	  i++;
   }

   /* 
   *  iterate statements and increase number when it is IMark
   *  IMark marks the start of the statements that represents
   *  single machine instruction
   */
   while (i < sbIn->stmts_used) {
      st = sbIn->stmts[i];

      if (st->tag == Ist_IMark)
	  	inscount++;
		
	  i++;
   }

   /*
   * make a function call to instruction counter increase function
   * add to the newly created super block
   */
   argv = mkIRExprVec_1(mkIRExpr_HWord( (HWord)inscount));
   di = unsafeIRDirty_0_N( 1, "per_instruction_BBV",
							VG_(fnptr_to_fnentry)( &per_instruction_BBV ), argv);
								
   addStmtToIRSB( sbOut,  IRStmt_Dirty(di));

   /*
   * copy the rest of the statements
   * to the created super block
   */
   i = 0;
   while ( (i < sbIn->stmts_used)) {
      addStmtToIRSB( sbOut, sbIn->stmts[i] );
      i++;
   }

   return sbOut;
}

/*
	naive version	
*/
static
IRSB* ic_inst_instrument ( VgCallbackClosure* closure,
                      IRSB* sbIn,
                      VexGuestLayout* layout, 
                      VexGuestExtents* vge,
                      IRType gWordTy, IRType hWordTy )
{
   Int      i;
   IRSB     *sbOut;
   IRStmt   *st;
   IRDirty  *di;
   IRExpr   **argv, *arg1;
   Int		inscount = 0;


   /* Set up SB */
   sbOut = deepCopyIRSBExceptStmts(sbIn);
   i = 0;

   /* skip if the statement is used */
   while ( (i < sbIn->stmts_used) && (sbIn->stmts[i]->tag!=Ist_IMark)) {
      addStmtToIRSB( sbOut, sbIn->stmts[i] );
   	  i++;
   }

   while (i < sbIn->stmts_used) {
      st = sbIn->stmts[i];

      /*
	  * If IMark (1 machine instruction) call instruction 
	  * counter increase function
	  */
      if (st->tag == Ist_IMark) {

         argv = mkIRExprVec_0();
	     di = unsafeIRDirty_0_N( 0, "per_instruction_BBV",
							VG_(fnptr_to_fnentry)( &per_instruction), argv);
								
         addStmtToIRSB( sbOut,  IRStmt_Dirty(di));
	  }

      addStmtToIRSB( sbOut, st );
	  i++;
	}

   return sbOut;
}

static void ic_fini(Int exitcode)
{
	VG_(printf)("Instruction Count = %lld\n", total);
}

static void ic_pre_clo_init(void)
{
   VG_(details_name)            ("Instruction Count");
   VG_(details_version)         ("0.5");
   VG_(details_description)     ("Instruction Count");
   VG_(details_copyright_author)(
      "Copyright (C) 2002-2011, and GNU GPL'd, by Wonjoon Song.");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   /*
   * ic_inst_instrument : naive instruction counter
   * ic_bb_instrument : optimized bb instruction counter
   */
   VG_(basic_tool_funcs)        (ic_post_clo_init,
//                                 ic_inst_instrument,
                                 ic_bb_instrument,
                                 ic_fini);

   /* No needs, no core events to track */
}

VG_DETERMINE_INTERFACE_VERSION(ic_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
