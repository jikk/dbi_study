#include "pin.H"
#include <string.h>
#include <iostream>

#define RBUF_SIZE 0x10000

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

UINT32 RBufIdx = 0;
UINT32 RBuf[RBUF_SIZE + 1024]; //safety buffer added


VOID RBufInit() {
  RBufIdx = 0;
  memset(RBuf, 0, sizeof RBUF_SIZE);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr <<
        "print usage\n"
        "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* ===================================================================== */

VOID PIN_FAST_ANALYSIS_CALL
MemAddrEnq1(ADDRINT ea0)
{
  RBuf[RBufIdx] = ea0;
  RBufIdx++;
}

VOID PIN_FAST_ANALYSIS_CALL
MemAddrEnq2(ADDRINT ea0, ADDRINT ea1)
{
  RBuf[RBufIdx] = ea0;
  RBufIdx++;

  RBuf[RBufIdx] = ea1;
  RBufIdx++;
}

VOID PIN_FAST_ANALYSIS_CALL
MemAddrEnq3(ADDRINT ea0, ADDRINT ea1, ADDRINT ea2)
{
  RBuf[RBufIdx] = ea0;
  RBufIdx++;

  RBuf[RBufIdx] = ea1;
  RBufIdx++;

  RBuf[RBufIdx] = ea2;
  RBufIdx++;
}

VOID PIN_FAST_ANALYSIS_CALL
MemAddrEnq4(ADDRINT ea0, ADDRINT ea1, ADDRINT ea2, ADDRINT ea3)
{
  RBuf[RBufIdx] = ea0;
  RBufIdx++;

  RBuf[RBufIdx] = ea1;
  RBufIdx++;

  RBuf[RBufIdx] = ea2;
  RBufIdx++;

  RBuf[RBufIdx] = ea3;
  RBufIdx++;
}

ADDRINT PIN_FAST_ANALYSIS_CALL
RBufChk(UINT32 BlkMOprCnt) {
  return (RBufIdx + BlkMOprCnt) - RBUF_SIZE;
}

VOID PIN_FAST_ANALYSIS_CALL
RBufReset() {
  RBufIdx = 0;
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID *v)
{
  UINT32 BlkMOprCnt =0;
  UINT32 InsMOprCnt = 0;

  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); BBL_Next(bbl))
  {
    BlkMOprCnt = 0;
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); INS_Next(ins))
    {
      InsMOprCnt = INS_MemoryOperandCount(ins);
      BlkMOprCnt += InsMOprCnt;

      switch(InsMOprCnt) {
        case 0:
          // Do not instrument here.
          break;
        case 1:
          INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) MemAddrEnq1,
                         IARG_FAST_ANALYSIS_CALL,
                         IARG_MEMORYOP_EA, 0,
                         IARG_END);
          break;
        case 2:
          INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) MemAddrEnq2,
                         IARG_FAST_ANALYSIS_CALL,
                         IARG_MEMORYOP_EA, 0,
                         IARG_MEMORYOP_EA, 1,
                         IARG_END);
          break;
        case 3:
          INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) MemAddrEnq3,
                         IARG_FAST_ANALYSIS_CALL,
                         IARG_MEMORYOP_EA, 0,
                         IARG_MEMORYOP_EA, 1,
                         IARG_MEMORYOP_EA, 2,
                         IARG_END);
          break;

        case 4:
          INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) MemAddrEnq4,
                         IARG_FAST_ANALYSIS_CALL,
                         IARG_MEMORYOP_EA, 0,
                         IARG_MEMORYOP_EA, 1,
                         IARG_MEMORYOP_EA, 2,
                         IARG_MEMORYOP_EA, 3,
                         IARG_END);
          break;

        default:
          ASSERTX(! "More than 5 memory operands");
      }

      //FIXME: this is naive instrumentation and can cause correctness issue as
      //it can be instrumented after memory enqueueing for the first
      //instruction.

      INS_InsertIfCall(BBL_InsHead(bbl), IPOINT_BEFORE,
                       (AFUNPTR) RBufChk,
                       IARG_FAST_ANALYSIS_CALL,
                       IARG_UINT32, BlkMOprCnt,
                       IARG_END);

      INS_InsertThenCall(BBL_InsHead(bbl), IPOINT_BEFORE,
                         (AFUNPTR) RBufReset,
                         IARG_FAST_ANALYSIS_CALL,
                         IARG_END);
    }
  }
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
    cerr <<  "Analysis Done "   << endl;
    
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }

    RBufInit();
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
