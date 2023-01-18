#include <iostream>
#include <Windows.h>
#pragma comment(lib, "ntdll.lib")

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

extern "C" {
NTSYSAPI
NTSTATUS
NTAPI
NtQueryInformationJobObject(IN HANDLE JobHandle,
                            IN JOBOBJECTINFOCLASS JobInformationClass,
                            OUT PVOID JobInformation,
                            IN ULONG JobInformationLength,
                            OUT PULONG ReturnLength OPTIONAL);
}
int main() {
  SILO_USER_SHARED_DATA t{};
  NTSTATUS Status = NtQueryInformationJobObject(
      NULL, JobObjectReserved17Information, &t, sizeof(t), NULL);

  if (Status != 0) printf("%x\n", Status);

  printf("%x\n", t.ServiceSessionId);


  return 0;
}
