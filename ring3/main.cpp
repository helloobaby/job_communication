#include<ntifs.h>
#include<ntddk.h>
#include<wdm.h>
#include"Zydis.h"

#define DBG_FATAL_ERROR       print("fatal error %s %s\n",__FILE__, __LINE__)

typedef struct _SILO_USER_SHARED_DATA  // 9 elements, 0x248 bytes (sizeof)
{
  /*0x000*/ ULONG32 ServiceSessionId;
  /*0x004*/ ULONG32 ActiveConsoleId;
  /*0x008*/ INT64 ConsoleSessionForegroundProcessId;
  /*0x010*/ enum _NT_PRODUCT_TYPE NtProductType;
  /*0x014*/ ULONG32 SuiteMask;
  /*0x018*/ ULONG32 SharedUserSessionId;
  /*0x01C*/ UINT8 IsMultiSessionSku;
  /*0x01D*/ UINT8 _PADDING0_[0x1];
  /*0x01E*/ WCHAR NtSystemRoot[260];
  /*0x226*/ UINT16 UserModeGlobalLogger[16];
  /*0x246*/ UINT8 _PADDING1_[0x2];
} SILO_USER_SHARED_DATA, *PSILO_USER_SHARED_DATA;

template<typename... types>
void print(types... args)
{
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, args...);
}

ULONG64 kUserSharedData_Offset = 0;
ULONG64 kPspHostSiloGlobals;

void GetVersionBindOffset() {
  RTL_OSVERSIONINFOW Version;
  RtlGetVersion(&Version);

  print("buildnum %d\n", Version.dwBuildNumber);

  switch (Version.dwBuildNumber) {
    case 17763:
      kUserSharedData_Offset = 0x450;
      break;
    default:
      break;
  }

}

void GetGlobalVarOffset() {
  // 通过PsGetServerSiloServiceSessionId解析PspHostSiloGlobals

  PVOID StartAddress = &PsGetServerSiloServiceSessionId;

  // 解析20条指令
  int count = 0;
  while (count < 20) {
    ZydisDisassembledInstruction insn;
    ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, (ZyanU64)StartAddress,
                          StartAddress, 15, &insn);

    print("%s\n", insn.text);
    
  ZyanU64 ResultAddress;
  if (insn.info.mnemonic == ZYDIS_MNEMONIC_LEA) {
    ZydisCalcAbsoluteAddress(&insn.info, &insn.operands[1], (ZyanU64)StartAddress,
                             &ResultAddress);
    kPspHostSiloGlobals = ResultAddress;

    break;
  }

  ULONG64 tmp = (ULONG64)StartAddress+ insn.info.length;
  StartAddress = (PVOID) tmp;

  }

  print("kPspHostSiloGlobals : %llx\n", kPspHostSiloGlobals);
}

void DriverUnload(PDRIVER_OBJECT Driver) { return; }

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT drv, PUNICODE_STRING reg) {

    drv->DriverUnload = DriverUnload;

    GetVersionBindOffset();

    if (!kUserSharedData_Offset) {
      DBG_FATAL_ERROR;
        return STATUS_UNSUCCESSFUL;
    }

    auto silo = PsGetHostSilo();

    if (silo)
      print("has job\n");
    else
      print("no job");

    print("val1 %d\n", PsGetEffectiveServerSilo(silo));

    GetGlobalVarOffset();

    PSILO_USER_SHARED_DATA UserData = *(PSILO_USER_SHARED_DATA*)(kPspHostSiloGlobals + kUserSharedData_Offset);


    // magic num
    UserData->ServiceSessionId = 0x12345678;

    print("magic num %x\n", PsGetServerSiloServiceSessionId(NULL));



	return STATUS_SUCCESS;
}