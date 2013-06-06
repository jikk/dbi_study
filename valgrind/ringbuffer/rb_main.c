
/*--------------------------------------------------------------------*/
/*--- Nulgrind: The minimal Valgrind tool.               rb_main.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Nulgrind, the minimal Valgrind tool,
   which does no instrumentation or analysis.

   Copyright (C) 2002-2011 Nicholas Nethercote
      njn@valgrind.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"

#define RBUF_SIZE 0x10000
#define RBUF_MASK 0xFFFF
   
UInt RBufIdx = 0;
Addr RBuf[RBUF_SIZE];

static void rb_post_clo_init(void)
{
}

static VG_REGPARM(1) void MemAddrEnq1(Addr ea0)
{
	RBuf[RBufIdx && RBUF_MASK] = ea0;
	RBufIdx++;
}

void addMemEnqEvent(IRSB *sb, IRExpr* addr, Int size)
{
	Char*      helperName = "MemAddrEnq1";
	void*      helperAddr = MemAddrEnq1;
	IRExpr**   argv;
	IRDirty*   di;

	argv = mkIRExprVec_1( addr);
	di   = unsafeIRDirty_0_N( 1, 
			helperName, VG_(fnptr_to_fnentry)( helperAddr ),
			argv );
	addStmtToIRSB( sb, IRStmt_Dirty(di) );
}

static
IRSB* rb_instrument ( VgCallbackClosure* closure,
                      IRSB* sbIn,
                      VexGuestLayout* layout, 
                      VexGuestExtents* vge,
                      IRType gWordTy, IRType hWordTy )
{
	Int i;
	IRSB*      sbOut;
	IRDirty*   di;
	IRTypeEnv* tyenv = sbIn->tyenv;


	// Make a new SB for return
	sbOut = deepCopyIRSBExceptStmts(sbIn);

	// check every statement in BB
	for (i = 0; i < sbIn->stmts_used; i++) {
		IRStmt* stmt = sbIn->stmts[i];

		if (!stmt || stmt->tag == Ist_NoOp) continue;

		// what kind of statement is it?
		switch (stmt->tag) {
			// Nothing to do with these instructions
			case Ist_NoOp:
			case Ist_AbiHint:
			case Ist_Put:
			case Ist_PutI:
			case Ist_MBE:
			case Ist_IMark:
				addStmtToIRSB( sbOut, stmt );
				break;

			case Ist_WrTmp : {
				 IRExpr *data = stmt->Ist.WrTmp.data;

				 if (data->tag == Iex_Load) {
					 addMemEnqEvent(sbOut, data->Iex.Load.addr, 
							 sizeofIRType(data->Iex.Load.ty));
				 }
				 addStmtToIRSB( sbOut, stmt );
				 break;
			 }

			case Ist_Store : {
				 IRExpr *data = stmt->Ist.Store.data;
				 addMemEnqEvent(sbOut, stmt->Ist.Store.addr,
						 sizeofIRType(typeOfIRExpr(tyenv, data)) );
				 addStmtToIRSB( sbOut, stmt );
				 break;
			 }
			case Ist_LLSC : {
				IRType dataTy;
				if (stmt->Ist.LLSC.storedata == NULL) {
					/* LL */
					dataTy = typeOfIRTemp(tyenv, stmt->Ist.LLSC.result);
					addMemEnqEvent( sbOut, stmt->Ist.LLSC.addr,
							sizeofIRType(dataTy) );
				} else {
					/* SC */
					dataTy = typeOfIRExpr(tyenv, stmt->Ist.LLSC.storedata);
					addMemEnqEvent( sbOut, stmt->Ist.LLSC.addr,
							sizeofIRType(dataTy) );
				}
				addStmtToIRSB( sbOut, stmt );
				break;
			}
			case Ist_Dirty: {
				Int      dsize;
				IRDirty* d = stmt->Ist.Dirty.details;
				if (d->mFx != Ifx_None) {
					// This dirty helper accesses memory.  Collect the details.
					dsize = d->mSize;
					if (d->mFx == Ifx_Read || d->mFx == Ifx_Modify) {
						addMemEnqEvent( sbOut, d->mAddr, dsize );
					}
					if (d->mFx == Ifx_Write || d->mFx == Ifx_Modify)
						addMemEnqEvent( sbOut, d->mAddr, dsize );
				} else {
				}
				addStmtToIRSB( sbOut, stmt );
				break;
			}

			case Ist_CAS: {
				Int    dataSize;
				IRType dataTy;
				IRCAS* cas = stmt->Ist.CAS.details;
				dataTy   = typeOfIRExpr(tyenv, cas->dataLo);
				dataSize = sizeofIRType(dataTy);
				if (cas->dataHi != NULL)
					dataSize *= 2; /* since it's a doubleword-CAS */

				addMemEnqEvent( sbOut, cas->addr, dataSize );
				addMemEnqEvent( sbOut, cas->addr, dataSize );

				addStmtToIRSB( sbOut, stmt );
				break;
			}
			case Ist_Exit:
				addStmtToIRSB( sbOut, stmt );      
				break;


			default :
				break;
		}

	}
	return sbOut;
}

static void rb_fini(Int exitcode)
{
}

static void rb_pre_clo_init(void)
{
   VG_(details_name)            ("Ring Buffer");
   VG_(details_version)         (NULL);
   VG_(details_description)     ("Ring Buffer Valgrind tool");
   VG_(details_copyright_author)(
      "Copyright (C) 2013, and GNU GPL'd, by Wonjoon Song.");
   VG_(details_bug_reports_to)  (VG_BUGS_TO);

   VG_(details_avg_translation_sizeB) ( 275 );

   VG_(basic_tool_funcs)        (rb_post_clo_init,
                                 rb_instrument,
                                 rb_fini);

   /* No needs, no core events to track */
}

VG_DETERMINE_INTERFACE_VERSION(rb_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
