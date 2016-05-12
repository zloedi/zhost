#include "common.h"
#include "zps2_types.h"
#include "ee.h"

typedef union {
    uint64_t UINT64[2];
    uint32_t UINT32[4];
} eeGPReg_t;

typedef struct {
    eeGPReg_t GPR[32];
    eeGPReg_t HI;
    eeGPReg_t LO;
    uint32_t pc;
    uint32_t opcode;
} eeCPU_t;

typedef struct {
    byte    ram[32 * 1024 * 1024];
    eeCPU_t cpu;
} eeState_t;

typedef void ( *eeFunction_t )( uint32_t );

static eeState_t *ee;
static eeFunction_t *ee_functions;

void EE_Reset( elf_t *elf ) {
    progHeader_t *phs = ( progHeader_t* )&elf->data[elf->header.e_phoff];
    for ( int i = 0; i < elf->header.e_phnum; i++ ) {
        progHeader_t *ph = &phs[i];
        // PT_LOAD segment type is the only one we care about
        if ( ph->p_type == 1 ) {
            memcpy( &ee->ram[ph->p_paddr], &elf->data[ph->p_offset], ph->p_filesz );
        }
    }
    zps2_printf( "EE_Reset: Setting PC to 0x%x\n", elf->header.e_entry );
    // FIXME: is this always multiple of 4?
    ee->cpu.pc = elf->header.e_entry >> 2;
    EE_Execute();
}

#define FIELD_5(instruction) ((instruction)>>26)
#define FIELD_4(instruction) (((instruction)>>21)&31)
#define FIELD_3(instruction) (((instruction)>>16)&31)
#define FIELD_2(instruction) (((instruction)>>11)&31)
#define FIELD_1(instruction) (((instruction)>>6)&31)
#define FIELD_0(instruction) ((instruction)&63)

static void EE_ExecuteInstruction() {
    eeCPU_t *cpu = &ee->cpu;
    uint32_t instruction = ( ( uint32_t* )ee->ram )[cpu->pc];
    // goes down the subclasses too
    uint32_t func = FIELD_5( instruction );
    ee_functions[func]( instruction );
    cpu->pc++;
}

void EE_Execute() {
    for ( int i = 0; i < 100; i++ ) {
        EE_ExecuteInstruction();
    }
}

#define CLASS_SIZE 64

#define CAT_HELPER_0(a,b) a##b
#define CAT_HELPER(a,b) CAT_HELPER_0(a,b)

#define unsupported CAT_HELPER(UNSUP,__COUNTER__)
#define reserved    CAT_HELPER(RESRV,__COUNTER__)
#define undefined   CAT_HELPER(UNDFN,__COUNTER__)

// Instructions encoded by OpCode field
enum {
    SPECIAL,     REGIMM, J,           JAL,      BEQ,         BNE,         BLEZ,  BGTZ,
    ADDI,        ADDIU,  SLTI,        SLTIU,    ANDI,        ORI,         XORI,  LUI,
    COP0,        COP1,   COP2,        reserved, BEQL,        BNEL,        BLEZL, BGTZL,
    DADDI,       DADDIU, LDL,         LDR,      MMI,         reserved,    LQ,    SQ,
    LB,          LH,     LWL,         LW,       LBU,         LHU,         LWR,   LWU,
    SB,          SH,     SWL,         SW,       SDL,         SDR,         SWR,   CACHE,
    unsupported, LWC1,   unsupported, PREF,     unsupported, unsupported, LQC2,  LD,
    unsupported, SWC1,   unsupported, reserved, unsupported, unsupported, SQC2,  SD,

// SPECIAL Instruction Class (encoded in function field)
    SLL,  reserved, SRL,  SRA,  SLLV,        reserved,    SRLV,        SRAV,
    JR,   JALR,     MOVZ, MOVN, SYSCALL,     BREAK,       reserved,    SYNC,
    MFHI, MTHI,     MFLO, MTLO, DSLLV,       reserved,    DSRLV,       DSRAV,
    MULT, MULTU,    DIV,  DIVU, unsupported, unsupported, unsupported, unsupported,
    ADD,  ADDU,     SUB,  SUBU, AND,         OR,          XOR,         NOR,
    MFSA, MTSA,     SLT,  SLTU, DADD,        DADDU,       DSUB,        DSUBU,
    TGE,  TGEU,     TLT,  TLTU, TEQ,         reserved,    TNE,         reserved,
    DSLL, reserved, DSRL, DSRA, DSLL32,      reserved,    DSRL32,      DSRA32,

// REGIMM Instruction Class (encoded by rt field when OpCode = REGIMM)
    BLTZ,   BGEZ,   BLTZL,    BGEZL,    reserved, reserved, reserved, reserved,
    TGEI,   TGEIU,  TLTI,     TLTIU,    TEQI,     reserved, TNEI,     reserved,
    BLTZAL, BGEZAL, BLTZALL,  BGEZALL,  reserved, reserved, reserved, reserved,
    MTSAB,  MTSAH,  reserved, reserved, reserved, reserved, reserved, reserved,

// MMI Instruction Class (encoded by function field when OpCode = MMI)
    MADD,     MADDU,    reserved, reserved, PLZCW,    reserved, reserved, reserved,
    MMI0,     MMI2,     reserved, reserved, reserved, reserved, reserved, reserved,
    MFHI1,    MTHI1,    MFLO1,    MTLO1,    reserved, reserved, reserved, reserved,
    MULT1,    MULTU1,   DIV1,     DIVU1,    reserved, reserved, reserved, reserved,
    MADD1,    MADDU1,   reserved, reserved, reserved, reserved, reserved, reserved,
    MMI1,     MMI3,     reserved, reserved, reserved, reserved, reserved, reserved,
    PMFHL,    PMTHL,    reserved, reserved, PSLLH,    reserved, PSRLH,    PSRAH,
    reserved, reserved, reserved, reserved, PSLLW,    reserved, PSRLW,    PSRAW,

// MMI0 Instruction Class (encoded by function field when OpCode = MMI and bits 5..0 = MMI0)
    PADDW,    PSUBW,    PCGTW,    PMAXW,
    PADDH,    PSUBH,    PCGTH,    PMAXH,
    PADDB,    PSUBB,    PCGTB,    reserved,
    reserved, reserved, reserved, reserved,
    PADDSW,   PSUBSW,   PEXTLW,   PPACW,
    PADDSH,   PSUBSH,   PEXTLH,   PPACH,
    PADDSB,   PSUBSB,   PEXTLB,   PPACB,
    reserved, reserved, PEXT5,    PPAC5,

// MMI1 Instruction Class (encoded by function field when OpCode = MMI and bits 5..0 = MMI1)
    reserved, PABSW,    PCEQW,    PMINW,
    PADSBH,   PABSH,    PCEQH,    PMINH,
    reserved, reserved, PCEQB,    reserved,
    reserved, reserved, reserved, reserved,
    PADDUW,   PSUBUW,   PEXTUW,   reserved,
    PADDUH,   PSUBUH,   PEXTUH,   reserved,
    PADDUB,   PSUBUB,   PEXTUB,   QFSRV,
    reserved, reserved, reserved, reserved,

// MMI2 Instruction Class (encoded by function field when OpCode = MMI and bits 5..0 = MMI2)
    PMADDW,   reserved, PSLLVW,   PSRLVW,
    PMSUBW,   reserved, reserved, reserved,
    PMFHI,    PMFLO,    PINTH,    reserved,
    PMULTW,   PDIVW,    PCPYLD,   reserved,
    PMADDH,   PHMADH,   PAND,     PXOR,
    PMSUBH,   PHMSBH,   reserved, reserved,
    reserved, reserved, PEXEH,    PREVH,
    PMULTH,   PDIVBW,   PEXEW,    PROT3W,

// MMI3 Instruction Class (encoded by function field when OpCode = MMI and bits 5..0 = MMI3)
    PMADDUW,  reserved, reserved, PSRAVW,
    reserved, reserved, reserved, reserved,
    PMTHI,    PMTLO,    PINTEH,   reserved,
    PMULTUW,  PDIVUW,   PCPYUD,   reserved,
    reserved, reserved, POR,      PNOR,
    reserved, reserved, reserved, reserved,
    reserved, reserved, PEXCH,    PCPYH,
    reserved, reserved, PEXCW,    reserved,

// COP0 Instruction Class (encoded by rs field when OpCode = COP0)
    MF0,      reserved, reserved, reserved, MT0,      reserved, reserved, reserved,
    BC0,      reserved, reserved, reserved, reserved, reserved, reserved, reserved,
    C0,       reserved, reserved, reserved, reserved, reserved, reserved, reserved,
    reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

// BC0 Instruction Class (encoded by rt field when OpCode field = COP0 and rs field = BC0)
    BC0F,     BC0T,     BC0FL,    BC0TL,    reserved, reserved, reserved, reserved,
    reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,
    reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,
    reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

// C0 Instruction Class (encoded by function field when OpCode = COP0 and rs = C0)
    undefined, TLBR,      TLBWI,     undefined, undefined, undefined, TLBWR,     undefined,
    TLBP,      undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    ERET,      undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    EI,        DI,        undefined, undefined, undefined, undefined, undefined, undefined,

// COP1 Instruction Class (encoded by rs field when OpCode = COP1)
    MFC1,     reserved, CFC1,     reserved, MTC1,     reserved, CTC1,     reserved,
    BC1,      reserved, reserved, reserved, reserved, reserved, reserved, reserved,
    S,        reserved, reserved, reserved, W,        reserved, reserved, reserved,
    reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

// BC1 Instruction Class (encoded by rt field when OpCode field = COP1 and rs = BC1)
    BC1F,     BC1T,     BC1FL,    BC1TL,    reserved, reserved, reserved, reserved,
    reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,
    reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,
    reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

// S Instruction Class (encoded by function field when OpCode = COP1 and rs = S)
    S_ADD,     S_SUB,     MUL,       S_DIV,     SQRT,      ABS,       MOV,       NEG,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, RSQRT,     undefined,
    ADDA,      SUBA,      MULA,      undefined, S_MADD,    MSUB,      MADDA,     MSUBA,
    undefined, undefined, undefined, undefined, CVTW,      undefined, undefined, undefined,
    MAX,       MIN,       undefined, undefined, undefined, undefined, undefined, undefined,
    C_F,       undefined, C_EQ,      undefined, C_LT,      undefined, C_LE,      undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,

// W Instruction Class (encoded by function field when OpCode = COP1 and rs = W)
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    CVTS,      undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,
    undefined, undefined, undefined, undefined, undefined, undefined, undefined, undefined,

    NUM_INSTRUCTIONS
};

enum {
    BEGIN_OPCODE,
    BEGIN_SPECIAL = 64,
    BEGIN_REGIMM = BEGIN_SPECIAL + 64,
    BEGIN_MMI = BEGIN_REGIMM + 32,
    BEGIN_MMI0 = BEGIN_MMI + 64,
    BEGIN_MMI1 = BEGIN_MMI0 + 32,
    BEGIN_MMI2 = BEGIN_MMI1 + 32,
    BEGIN_MMI3 = BEGIN_MMI2 + 32,
    BEGIN_COP0 = BEGIN_MMI3 + 32,
    BEGIN_BC0 = BEGIN_COP0 + 32,
    BEGIN_C0 = BEGIN_BC0 + 32,
    BEGIN_COP1 = BEGIN_C0 + 64,
    BEGIN_BC1 = BEGIN_COP1 + 32,
    BEGIN_S = BEGIN_BC1 + 32,
    BEGIN_W = BEGIN_S + 64,
};

#ifdef EE_LOG_INTERPRETER
#define EE_LOG(...) zps2_printf(__VA_ARGS__)
#else
#define EE_LOG(...)
#endif

static void mmi( uint32_t instruction ) {
    EE_LOG( "mmi->" );
    uint32_t func = BEGIN_MMI + FIELD_0( instruction );
    ee_functions[func]( instruction );
}

static void mmi1( uint32_t instruction ) {
    EE_LOG( "mmi1->" );
    uint32_t func = BEGIN_MMI1 + FIELD_1( instruction );
    ee_functions[func]( instruction );
}

static void mthi1( uint32_t instruction ) {
    EE_LOG( "mthi1 not implemented\n" );
}

static void madd1( uint32_t instruction ) {
    EE_LOG( "madd1 not implemented\n" );
}

static void special( uint32_t instruction ) {
    EE_LOG( "special->" );
    uint32_t func = BEGIN_SPECIAL + FIELD_0( instruction );
    ee_functions[func]( instruction );
}

static void mthi( uint32_t instruction ) {
    EE_LOG( "mthi " );
    int rs = FIELD_4( instruction );
    eeCPU_t *cpu = &ee->cpu;
    cpu->HI.UINT64[0] = cpu->GPR[rs].UINT64[0];
    EE_LOG( "%llu\n", cpu->GPR[rs].UINT64[0] );
}

static void mtlo( uint32_t instruction ) {
    // FIXME: havent hit this one yet
    EE_LOG( "mtlo " );
    int rs = FIELD_4( instruction );
    eeCPU_t *cpu = &ee->cpu;
    cpu->LO.UINT64[0] = ee->cpu.GPR[rs].UINT64[0];
    EE_LOG( "%llu\n", cpu->GPR[rs].UINT64[0] );
}

static void padduw( uint32_t instruction ) {
    EE_LOG( "padduw" );
    eeGPReg_t *regs = ee->cpu.GPR;
    int rs = FIELD_4( instruction );
    int rt = FIELD_3( instruction );
    int rd = FIELD_2( instruction );
    for ( int i = 0; i < 4; i++ ) {
        uint32_t a = regs[rs].UINT32[i];
        uint32_t b = regs[rt].UINT32[i];
        uint32_t sum = a + b;
        regs[rd].UINT32[i] = sum >= a ?  sum : 0xffffffff;
        EE_LOG( " | %u + %u = %u", a, b, sum );
    }
    EE_LOG( "\n" );
}

static void UNKNOWN( uint32_t instruction ) {
    uint32_t op = FIELD_5(instruction);
    uint32_t rs = FIELD_4(instruction);
    uint32_t rt = FIELD_3(instruction);
    uint32_t i2 = FIELD_2(instruction);
    uint32_t i1 = FIELD_1(instruction);
    uint32_t i0 = FIELD_0(instruction);
    zps2_fatalf( "Unknown instruction\nx%.2x x%.2x x%.2x x%.2x x%.2x x%.2x\n %.2d  %.2d  %.2d  %.2d  %.2d  %.2d\n", op, rs, rt, i2, i1, i0, op, rs, rt, i2, i1, i0 );
}

void EE_Init() {
    ee = A_Static( sizeof( *ee ) );
    ee_functions = A_Static( NUM_INSTRUCTIONS * sizeof( *ee_functions ) );
    for ( int i = 0; i < NUM_INSTRUCTIONS; i++ ) {
        ee_functions[i] = UNKNOWN;
    }
    ee_functions[SPECIAL] = special;
    ee_functions[MTHI] = mthi;
    ee_functions[MTLO] = mtlo;
    ee_functions[MMI] = mmi;
    ee_functions[MMI1] = mmi1;
    ee_functions[MTHI1] = mthi1;
    ee_functions[MADD1] = madd1;
    ee_functions[PADDUW] = padduw;
}
