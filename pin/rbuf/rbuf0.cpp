#include "pin.H"
#include <string.h>
#include <iostream>

#define RBUF_SIZE 0x10000
#define RBUF_MASK 0xFFFF

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

UINT32 RBufIdx = 0;
UINT32 RBuf[RBUF_SIZE];


VOID RBufInit() {
  RBufIdx = 0;
  memset(RBuf, 0, sizeof RBUF_SIZE);
}

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */


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
  RBuf[RBufIdx && RBUF_MASK] = ea0;
  RBufIdx++;
}

VOID PIN_FAST_ANALYSIS_CALL
MemAddrEnq2(ADDRINT ea0, ADDRINT ea1)
{
  RBuf[RBufIdx && RBUF_MASK] = ea0;
  RBufIdx++;

  RBuf[RBufIdx && RBUF_MASK] = ea1;
  RBufIdx++;
}

VOID PIN_FAST_ANALYSIS_CALL
MemAddrEnq3(ADDRINT ea0, ADDRINT ea1, ADDRINT ea2)
{
  RBuf[RBufIdx && RBUF_MASK] = ea0;
  RBufIdx++;

  RBuf[RBufIdx && RBUF_MASK] = ea1;
  RBufIdx++;

  RBuf[RBufIdx && RBUF_MASK] = ea2;
  RBufIdx++;
}

VOID PIN_FAST_ANALYSIS_CALL
MemAddrEnq4(ADDRINT ea0, ADDRINT ea1, ADDRINT ea2, ADDRINT ea3)
{
  RBuf[RBufIdx && RBUF_MASK] = ea0;
  RBufIdx++;

  RBuf[RBufIdx && RBUF_MASK] = ea1;
  RBufIdx++;

  RBuf[RBufIdx && RBUF_MASK] = ea2;
  RBufIdx++;

  RBuf[RBufIdx && RBUF_MASK] = ea3;
  RBufIdx++;
}


/* ===================================================================== */

VOID Trace(TRACE trace, VOID *v)
{
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); BBL_Next(bbl))
  {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); INS_Next(ins))
    {
      UINT32 MOprCnt = INS_MemoryOperandCount(ins);
      switch(MOprCnt) {
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
