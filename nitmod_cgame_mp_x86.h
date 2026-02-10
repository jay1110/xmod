typedef unsigned char   undefined;

typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned long long    GUID;
typedef pointer32 ImageBaseOffset32;

typedef long long    longlong;
typedef unsigned char    uchar;
typedef unsigned int    uint;
typedef unsigned long    ulong;
typedef unsigned char    undefined1;
typedef unsigned short    undefined2;
typedef unsigned int    undefined4;
typedef unsigned long long    undefined6;
typedef unsigned long long    undefined8;
typedef unsigned short    ushort;
typedef unsigned short    wchar16;
typedef short    wchar_t;
typedef unsigned short    word;
#define unkbyte9   unsigned long long
#define unkbyte10   unsigned long long
#define unkbyte11   unsigned long long
#define unkbyte12   unsigned long long
#define unkbyte13   unsigned long long
#define unkbyte14   unsigned long long
#define unkbyte15   unsigned long long
#define unkbyte16   unsigned long long

#define unkuint9   unsigned long long
#define unkuint10   unsigned long long
#define unkuint11   unsigned long long
#define unkuint12   unsigned long long
#define unkuint13   unsigned long long
#define unkuint14   unsigned long long
#define unkuint15   unsigned long long
#define unkuint16   unsigned long long

#define unkint9   long long
#define unkint10   long long
#define unkint11   long long
#define unkint12   long long
#define unkint13   long long
#define unkint14   long long
#define unkint15   long long
#define unkint16   long long

#define unkfloat1   float
#define unkfloat2   float
#define unkfloat3   float
#define unkfloat5   double
#define unkfloat6   double
#define unkfloat7   double
#define unkfloat9   long double
#define unkfloat11   long double
#define unkfloat12   long double
#define unkfloat13   long double
#define unkfloat14   long double
#define unkfloat15   long double
#define unkfloat16   long double

#define BADSPACEBASE   void
#define code   void

typedef struct CLIENT_ID CLIENT_ID, *PCLIENT_ID;

struct CLIENT_ID {
    void *UniqueProcess;
    void *UniqueThread;
};

typedef union IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion;

typedef struct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct;

struct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct {
    dword OffsetToDirectory:31;
    dword DataIsDirectory:1;
};

union IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion {
    dword OffsetToData;
    struct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct;
};

typedef struct _cpinfo _cpinfo, *P_cpinfo;

typedef uint UINT;

typedef uchar BYTE;

struct _cpinfo {
    UINT MaxCharSize;
    BYTE DefaultChar[2];
    BYTE LeadByte[12];
};

typedef struct _cpinfo *LPCPINFO;

typedef ulong DWORD;

typedef DWORD LCTYPE;

typedef struct _STARTUPINFOA _STARTUPINFOA, *P_STARTUPINFOA;

typedef char CHAR;

typedef CHAR *LPSTR;

typedef ushort WORD;

typedef BYTE *LPBYTE;

typedef void *HANDLE;

struct _STARTUPINFOA {
    DWORD cb;
    LPSTR lpReserved;
    LPSTR lpDesktop;
    LPSTR lpTitle;
    DWORD dwX;
    DWORD dwY;
    DWORD dwXSize;
    DWORD dwYSize;
    DWORD dwXCountChars;
    DWORD dwYCountChars;
    DWORD dwFillAttribute;
    DWORD dwFlags;
    WORD wShowWindow;
    WORD cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
};

typedef struct _SECURITY_ATTRIBUTES _SECURITY_ATTRIBUTES, *P_SECURITY_ATTRIBUTES;

typedef void *LPVOID;

typedef int BOOL;

struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
};

typedef struct _STARTUPINFOA *LPSTARTUPINFOA;

typedef struct _TIME_ZONE_INFORMATION _TIME_ZONE_INFORMATION, *P_TIME_ZONE_INFORMATION;

typedef long LONG;

typedef wchar_t WCHAR;

typedef struct _SYSTEMTIME _SYSTEMTIME, *P_SYSTEMTIME;

typedef struct _SYSTEMTIME SYSTEMTIME;

struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[32];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[32];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
};

typedef struct _OVERLAPPED _OVERLAPPED, *P_OVERLAPPED;

typedef ulong ULONG_PTR;

typedef union _union_518 _union_518, *P_union_518;

typedef struct _struct_519 _struct_519, *P_struct_519;

typedef void *PVOID;

struct _struct_519 {
    DWORD Offset;
    DWORD OffsetHigh;
};

union _union_518 {
    struct _struct_519 s;
    PVOID Pointer;
};

struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union _union_518 u;
    HANDLE hEvent;
};

typedef struct _TIME_ZONE_INFORMATION *LPTIME_ZONE_INFORMATION;

typedef struct _RTL_CRITICAL_SECTION _RTL_CRITICAL_SECTION, *P_RTL_CRITICAL_SECTION;

typedef struct _RTL_CRITICAL_SECTION *PRTL_CRITICAL_SECTION;

typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

typedef struct _RTL_CRITICAL_SECTION_DEBUG _RTL_CRITICAL_SECTION_DEBUG, *P_RTL_CRITICAL_SECTION_DEBUG;

typedef struct _RTL_CRITICAL_SECTION_DEBUG *PRTL_CRITICAL_SECTION_DEBUG;

typedef struct _LIST_ENTRY _LIST_ENTRY, *P_LIST_ENTRY;

typedef struct _LIST_ENTRY LIST_ENTRY;

struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;
};

struct _LIST_ENTRY {
    struct _LIST_ENTRY *Flink;
    struct _LIST_ENTRY *Blink;
};

struct _RTL_CRITICAL_SECTION_DEBUG {
    WORD Type;
    WORD CreatorBackTraceIndex;
    struct _RTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    DWORD EntryCount;
    DWORD ContentionCount;
    DWORD Flags;
    WORD CreatorBackTraceIndexHigh;
    WORD SpareWORD;
};

typedef DWORD (*PTHREAD_START_ROUTINE)(LPVOID);

typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef struct _OVERLAPPED *LPOVERLAPPED;

typedef struct _EXCEPTION_POINTERS _EXCEPTION_POINTERS, *P_EXCEPTION_POINTERS;

typedef LONG (*PTOP_LEVEL_EXCEPTION_FILTER)(struct _EXCEPTION_POINTERS *);

typedef struct _EXCEPTION_RECORD _EXCEPTION_RECORD, *P_EXCEPTION_RECORD;

typedef struct _EXCEPTION_RECORD EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _CONTEXT _CONTEXT, *P_CONTEXT;

typedef struct _CONTEXT CONTEXT;

typedef CONTEXT *PCONTEXT;

typedef struct _FLOATING_SAVE_AREA _FLOATING_SAVE_AREA, *P_FLOATING_SAVE_AREA;

typedef struct _FLOATING_SAVE_AREA FLOATING_SAVE_AREA;

struct _FLOATING_SAVE_AREA {
    DWORD ControlWord;
    DWORD StatusWord;
    DWORD TagWord;
    DWORD ErrorOffset;
    DWORD ErrorSelector;
    DWORD DataOffset;
    DWORD DataSelector;
    BYTE RegisterArea[80];
    DWORD Cr0NpxState;
};

struct _CONTEXT {
    DWORD ContextFlags;
    DWORD Dr0;
    DWORD Dr1;
    DWORD Dr2;
    DWORD Dr3;
    DWORD Dr6;
    DWORD Dr7;
    FLOATING_SAVE_AREA FloatSave;
    DWORD SegGs;
    DWORD SegFs;
    DWORD SegEs;
    DWORD SegDs;
    DWORD Edi;
    DWORD Esi;
    DWORD Ebx;
    DWORD Edx;
    DWORD Ecx;
    DWORD Eax;
    DWORD Ebp;
    DWORD Eip;
    DWORD SegCs;
    DWORD EFlags;
    DWORD Esp;
    DWORD SegSs;
    BYTE ExtendedRegisters[512];
};

struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[15];
};

struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
};

typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

typedef struct _MEMORY_BASIC_INFORMATION _MEMORY_BASIC_INFORMATION, *P_MEMORY_BASIC_INFORMATION;

typedef ULONG_PTR SIZE_T;

struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    DWORD AllocationProtect;
    SIZE_T RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
};

typedef union _LARGE_INTEGER _LARGE_INTEGER, *P_LARGE_INTEGER;

typedef struct _struct_19 _struct_19, *P_struct_19;

typedef struct _struct_20 _struct_20, *P_struct_20;

typedef double LONGLONG;

struct _struct_20 {
    DWORD LowPart;
    LONG HighPart;
};

struct _struct_19 {
    DWORD LowPart;
    LONG HighPart;
};

union _LARGE_INTEGER {
    struct _struct_19 s;
    struct _struct_20 u;
    LONGLONG QuadPart;
};

typedef union _LARGE_INTEGER LARGE_INTEGER;

typedef struct _TOKEN_PRIVILEGES _TOKEN_PRIVILEGES, *P_TOKEN_PRIVILEGES;

typedef struct _LUID_AND_ATTRIBUTES _LUID_AND_ATTRIBUTES, *P_LUID_AND_ATTRIBUTES;

typedef struct _LUID_AND_ATTRIBUTES LUID_AND_ATTRIBUTES;

typedef struct _LUID _LUID, *P_LUID;

typedef struct _LUID LUID;

struct _LUID {
    DWORD LowPart;
    LONG HighPart;
};

struct _LUID_AND_ATTRIBUTES {
    LUID Luid;
    DWORD Attributes;
};

struct _TOKEN_PRIVILEGES {
    DWORD PrivilegeCount;
    LUID_AND_ATTRIBUTES Privileges[1];
};

typedef struct _IMAGE_SECTION_HEADER _IMAGE_SECTION_HEADER, *P_IMAGE_SECTION_HEADER;

typedef union _union_226 _union_226, *P_union_226;

union _union_226 {
    DWORD PhysicalAddress;
    DWORD VirtualSize;
};

struct _IMAGE_SECTION_HEADER {
    BYTE Name[8];
    union _union_226 Misc;
    DWORD VirtualAddress;
    DWORD SizeOfRawData;
    DWORD PointerToRawData;
    DWORD PointerToRelocations;
    DWORD PointerToLinenumbers;
    WORD NumberOfRelocations;
    WORD NumberOfLinenumbers;
    DWORD Characteristics;
};

typedef WCHAR *LPWSTR;

typedef struct _IMAGE_SECTION_HEADER *PIMAGE_SECTION_HEADER;

typedef WCHAR *PCNZWCH;

typedef WCHAR *LPWCH;

typedef WCHAR *LPCWSTR;

typedef struct _LUID *PLUID;

typedef CHAR *LPCSTR;

typedef struct _MEMORY_BASIC_INFORMATION *PMEMORY_BASIC_INFORMATION;

typedef LONG *PLONG;

typedef CHAR *LPCH;

typedef struct _TOKEN_PRIVILEGES *PTOKEN_PRIVILEGES;

typedef DWORD LCID;

typedef CHAR *PCNZCH;

typedef HANDLE *PHANDLE;

typedef struct IMAGE_DOS_HEADER IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

struct IMAGE_DOS_HEADER {
    char e_magic[2]; // Magic number
    word e_cblp; // Bytes of last page
    word e_cp; // Pages in file
    word e_crlc; // Relocations
    word e_cparhdr; // Size of header in paragraphs
    word e_minalloc; // Minimum extra paragraphs needed
    word e_maxalloc; // Maximum extra paragraphs needed
    word e_ss; // Initial (relative) SS value
    word e_sp; // Initial SP value
    word e_csum; // Checksum
    word e_ip; // Initial IP value
    word e_cs; // Initial (relative) CS value
    word e_lfarlc; // File address of relocation table
    word e_ovno; // Overlay number
    word e_res[4][4]; // Reserved words
    word e_oemid; // OEM identifier (for e_oeminfo)
    word e_oeminfo; // OEM information; e_oemid specific
    word e_res2[10][10]; // Reserved words
    dword e_lfanew; // File address of new exe header
    byte e_program[64]; // Actual DOS program
};

typedef struct tm tm, *Ptm;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

typedef long clock_t;

typedef ULONG_PTR DWORD_PTR;

typedef uint UINT_PTR;

typedef void (*_PHNDLR)(int);

typedef struct in_addr in_addr, *Pin_addr;

typedef union _union_1226 _union_1226, *P_union_1226;

typedef struct _struct_1227 _struct_1227, *P_struct_1227;

typedef struct _struct_1228 _struct_1228, *P_struct_1228;

typedef ulong ULONG;

typedef uchar UCHAR;

typedef ushort USHORT;

struct _struct_1228 {
    USHORT s_w1;
    USHORT s_w2;
};

struct _struct_1227 {
    UCHAR s_b1;
    UCHAR s_b2;
    UCHAR s_b3;
    UCHAR s_b4;
};

union _union_1226 {
    struct _struct_1227 S_un_b;
    struct _struct_1228 S_un_w;
    ULONG S_addr;
};

struct in_addr {
    union _union_1226 S_un;
};

typedef struct _strflt _strflt, *P_strflt;

struct _strflt {
    int sign;
    int decpt;
    int flag;
    char *mantissa;
};

typedef struct _flt _flt, *P_flt;

struct _flt {
    int flags;
    int nbytes;
    long lval;
    double dval;
};

typedef struct _strflt *STRFLT;

typedef enum enum_3272 {
    INTRNCVT_OK=0,
    INTRNCVT_OVERFLOW=1,
    INTRNCVT_UNDERFLOW=2
} enum_3272;

typedef enum enum_3272 INTRNCVT_STATUS;

typedef struct _flt *FLT;

typedef struct _MODULEINFO _MODULEINFO, *P_MODULEINFO;

struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
};

typedef struct _MODULEINFO *LPMODULEINFO;

typedef struct _FILETIME _FILETIME, *P_FILETIME;

typedef struct _FILETIME *LPFILETIME;

struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

typedef int (*FARPROC)(void);

typedef WORD *LPWORD;

typedef DWORD *LPDWORD;

typedef struct HINSTANCE__ HINSTANCE__, *PHINSTANCE__;

struct HINSTANCE__ {
    int unused;
};

typedef DWORD *PDWORD;

typedef HANDLE HGLOBAL;

typedef BOOL *LPBOOL;

typedef BYTE *PBYTE;

typedef struct HINSTANCE__ *HINSTANCE;

typedef HINSTANCE HMODULE;

typedef void *LPCVOID;

typedef struct HWND__ HWND__, *PHWND__;

typedef struct HWND__ *HWND;

struct HWND__ {
    int unused;
};

typedef struct IMAGE_OPTIONAL_HEADER32 IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct IMAGE_DATA_DIRECTORY IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

struct IMAGE_DATA_DIRECTORY {
    ImageBaseOffset32 VirtualAddress;
    dword Size;
};

struct IMAGE_OPTIONAL_HEADER32 {
    word Magic;
    byte MajorLinkerVersion;
    byte MinorLinkerVersion;
    dword SizeOfCode;
    dword SizeOfInitializedData;
    dword SizeOfUninitializedData;
    ImageBaseOffset32 AddressOfEntryPoint;
    ImageBaseOffset32 BaseOfCode;
    ImageBaseOffset32 BaseOfData;
    pointer32 ImageBase;
    dword SectionAlignment;
    dword FileAlignment;
    word MajorOperatingSystemVersion;
    word MinorOperatingSystemVersion;
    word MajorImageVersion;
    word MinorImageVersion;
    word MajorSubsystemVersion;
    word MinorSubsystemVersion;
    dword Win32VersionValue;
    dword SizeOfImage;
    dword SizeOfHeaders;
    dword CheckSum;
    word Subsystem;
    word DllCharacteristics;
    dword SizeOfStackReserve;
    dword SizeOfStackCommit;
    dword SizeOfHeapReserve;
    dword SizeOfHeapCommit;
    dword LoaderFlags;
    dword NumberOfRvaAndSizes;
    struct IMAGE_DATA_DIRECTORY DataDirectory[16];
};

typedef struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct;

struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct {
    dword NameOffset:31;
    dword NameIsString:1;
};

typedef struct IMAGE_FILE_HEADER IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

struct IMAGE_FILE_HEADER {
    word Machine; // 332
    word NumberOfSections;
    dword TimeDateStamp;
    dword PointerToSymbolTable;
    dword NumberOfSymbols;
    word SizeOfOptionalHeader;
    word Characteristics;
};

typedef struct IMAGE_NT_HEADERS32 IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

struct IMAGE_NT_HEADERS32 {
    char Signature[4];
    struct IMAGE_FILE_HEADER FileHeader;
    struct IMAGE_OPTIONAL_HEADER32 OptionalHeader;
};

typedef struct IMAGE_RESOURCE_DIRECTORY_ENTRY IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;

typedef union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion;

union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion {
    struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct;
    dword Name;
    word Id;
};

struct IMAGE_RESOURCE_DIRECTORY_ENTRY {
    union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion NameUnion;
    union IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion DirectoryUnion;
};

typedef struct IMAGE_SECTION_HEADER IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef union Misc Misc, *PMisc;

typedef enum SectionFlags {
    IMAGE_SCN_TYPE_NO_PAD=8,
    IMAGE_SCN_RESERVED_0001=16,
    IMAGE_SCN_CNT_CODE=32,
    IMAGE_SCN_CNT_INITIALIZED_DATA=64,
    IMAGE_SCN_CNT_UNINITIALIZED_DATA=128,
    IMAGE_SCN_LNK_OTHER=256,
    IMAGE_SCN_LNK_INFO=512,
    IMAGE_SCN_RESERVED_0040=1024,
    IMAGE_SCN_LNK_REMOVE=2048,
    IMAGE_SCN_LNK_COMDAT=4096,
    IMAGE_SCN_GPREL=32768,
    IMAGE_SCN_MEM_16BIT=131072,
    IMAGE_SCN_MEM_PURGEABLE=131072,
    IMAGE_SCN_MEM_LOCKED=262144,
    IMAGE_SCN_MEM_PRELOAD=524288,
    IMAGE_SCN_ALIGN_1BYTES=1048576,
    IMAGE_SCN_ALIGN_2BYTES=2097152,
    IMAGE_SCN_ALIGN_4BYTES=3145728,
    IMAGE_SCN_ALIGN_8BYTES=4194304,
    IMAGE_SCN_ALIGN_16BYTES=5242880,
    IMAGE_SCN_ALIGN_32BYTES=6291456,
    IMAGE_SCN_ALIGN_64BYTES=7340032,
    IMAGE_SCN_ALIGN_128BYTES=8388608,
    IMAGE_SCN_ALIGN_256BYTES=9437184,
    IMAGE_SCN_ALIGN_512BYTES=10485760,
    IMAGE_SCN_ALIGN_1024BYTES=11534336,
    IMAGE_SCN_ALIGN_2048BYTES=12582912,
    IMAGE_SCN_ALIGN_4096BYTES=13631488,
    IMAGE_SCN_ALIGN_8192BYTES=14680064,
    IMAGE_SCN_LNK_NRELOC_OVFL=16777216,
    IMAGE_SCN_MEM_DISCARDABLE=33554432,
    IMAGE_SCN_MEM_NOT_CACHED=67108864,
    IMAGE_SCN_MEM_NOT_PAGED=134217728,
    IMAGE_SCN_MEM_SHARED=268435456,
    IMAGE_SCN_MEM_EXECUTE=536870912,
    IMAGE_SCN_MEM_READ=1073741824,
    IMAGE_SCN_MEM_WRITE=2147483648
} SectionFlags;

union Misc {
    dword PhysicalAddress;
    dword VirtualSize;
};

struct IMAGE_SECTION_HEADER {
    char Name[8];
    union Misc Misc;
    ImageBaseOffset32 VirtualAddress;
    dword SizeOfRawData;
    dword PointerToRawData;
    dword PointerToRelocations;
    dword PointerToLinenumbers;
    word NumberOfRelocations;
    word NumberOfLinenumbers;
    enum SectionFlags Characteristics;
};

typedef struct IMAGE_RESOURCE_DATA_ENTRY IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;

struct IMAGE_RESOURCE_DATA_ENTRY {
    dword OffsetToData;
    dword Size;
    dword CodePage;
    dword Reserved;
};

typedef struct IMAGE_RESOURCE_DIRECTORY IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;

struct IMAGE_RESOURCE_DIRECTORY {
    dword Characteristics;
    dword TimeDateStamp;
    word MajorVersion;
    word MinorVersion;
    word NumberOfNamedEntries;
    word NumberOfIdEntries;
};

typedef struct IMAGE_DIRECTORY_ENTRY_EXPORT IMAGE_DIRECTORY_ENTRY_EXPORT, *PIMAGE_DIRECTORY_ENTRY_EXPORT;

struct IMAGE_DIRECTORY_ENTRY_EXPORT {
    dword Characteristics;
    dword TimeDateStamp;
    word MajorVersion;
    word MinorVersion;
    ImageBaseOffset32 Name;
    dword Base;
    dword NumberOfFunctions;
    dword NumberOfNames;
    ImageBaseOffset32 AddressOfFunctions;
    ImageBaseOffset32 AddressOfNames;
    ImageBaseOffset32 AddressOfNameOrdinals;
};

typedef struct IMAGE_LOAD_CONFIG_DIRECTORY32 IMAGE_LOAD_CONFIG_DIRECTORY32, *PIMAGE_LOAD_CONFIG_DIRECTORY32;

struct IMAGE_LOAD_CONFIG_DIRECTORY32 {
    dword Size;
    dword TimeDateStamp;
    word MajorVersion;
    word MinorVersion;
    dword GlobalFlagsClear;
    dword GlobalFlagsSet;
    dword CriticalSectionDefaultTimeout;
    dword DeCommitFreeBlockThreshold;
    dword DeCommitTotalFreeThreshold;
    pointer32 LockPrefixTable;
    dword MaximumAllocationSize;
    dword VirtualMemoryThreshold;
    dword ProcessHeapFlags;
    dword ProcessAffinityMask;
    word CsdVersion;
    word DependentLoadFlags;
    pointer32 EditList;
    pointer32 SecurityCookie;
    pointer32 SEHandlerTable;
    dword SEHandlerCount;
};

typedef struct _iobuf _iobuf, *P_iobuf;

struct _iobuf {
    char *_ptr;
    int _cnt;
    char *_base;
    int _flag;
    int _file;
    int _charbuf;
    int _bufsiz;
    char *_tmpfname;
};

typedef struct _iobuf FILE;

typedef char *va_list;

typedef uint uintptr_t;

typedef ulong u_long;

typedef UINT_PTR SOCKET;

typedef ushort u_short;

typedef struct sockaddr sockaddr, *Psockaddr;

struct sockaddr {
    u_short sa_family;
    char sa_data[14];
};

typedef struct fd_set fd_set, *Pfd_set;

typedef uint u_int;

struct fd_set {
    u_int fd_count;
    SOCKET fd_array[64];
};

typedef struct timeval timeval, *Ptimeval;

struct timeval {
    long tv_sec;
    long tv_usec;
};

typedef struct hostent hostent, *Phostent;

struct hostent {
    char *h_name;
    char **h_aliases;
    short h_addrtype;
    short h_length;
    char **h_addr_list;
};

typedef struct _tiddata _tiddata, *P_tiddata;

typedef struct _tiddata *_ptiddata;

typedef struct threadmbcinfostruct threadmbcinfostruct, *Pthreadmbcinfostruct;

typedef struct threadmbcinfostruct *pthreadmbcinfo;

typedef struct threadlocaleinfostruct threadlocaleinfostruct, *Pthreadlocaleinfostruct;

typedef struct threadlocaleinfostruct *pthreadlocinfo;

typedef struct setloc_struct setloc_struct, *Psetloc_struct;

typedef struct setloc_struct _setloc_struct;

typedef struct localerefcount localerefcount, *Plocalerefcount;

typedef struct localerefcount locrefcount;

typedef struct lconv lconv, *Plconv;

typedef struct __lc_time_data __lc_time_data, *P__lc_time_data;

typedef struct _is_ctype_compatible _is_ctype_compatible, *P_is_ctype_compatible;

struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;
    char *int_curr_symbol;
    char *currency_symbol;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;
    char int_frac_digits;
    char frac_digits;
    char p_cs_precedes;
    char p_sep_by_space;
    char n_cs_precedes;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
    wchar_t *_W_decimal_point;
    wchar_t *_W_thousands_sep;
    wchar_t *_W_int_curr_symbol;
    wchar_t *_W_currency_symbol;
    wchar_t *_W_mon_decimal_point;
    wchar_t *_W_mon_thousands_sep;
    wchar_t *_W_positive_sign;
    wchar_t *_W_negative_sign;
};

struct _is_ctype_compatible {
    ulong id;
    int is_clike;
};

struct setloc_struct {
    wchar_t *pchLanguage;
    wchar_t *pchCountry;
    int iLocState;
    int iPrimaryLen;
    BOOL bAbbrevLanguage;
    BOOL bAbbrevCountry;
    UINT _cachecp;
    wchar_t _cachein[131];
    wchar_t _cacheout[131];
    struct _is_ctype_compatible _Loc_c[5];
    wchar_t _cacheLocaleName[85];
};

struct threadmbcinfostruct {
    int refcount;
    int mbcodepage;
    int ismbcodepage;
    ushort mbulinfo[6];
    uchar mbctype[257];
    uchar mbcasemap[256];
    wchar_t *mblocalename;
};

struct localerefcount {
    char *locale;
    wchar_t *wlocale;
    int *refcount;
    int *wrefcount;
};

struct threadlocaleinfostruct {
    int refcount;
    uint lc_codepage;
    uint lc_collate_cp;
    uint lc_time_cp;
    locrefcount lc_category[6];
    int lc_clike;
    int mb_cur_max;
    int *lconv_intl_refcount;
    int *lconv_num_refcount;
    int *lconv_mon_refcount;
    struct lconv *lconv;
    int *ctype1_refcount;
    ushort *ctype1;
    ushort *pctype;
    uchar *pclmap;
    uchar *pcumap;
    struct __lc_time_data *lc_time_curr;
    wchar_t *locale_name[6];
};

struct _tiddata {
    ulong _tid;
    uintptr_t _thandle;
    int _terrno;
    ulong _tdoserrno;
    uint _fpds;
    ulong _holdrand;
    char *_token;
    wchar_t *_wtoken;
    uchar *_mtoken;
    char *_errmsg;
    wchar_t *_werrmsg;
    char *_namebuf0;
    wchar_t *_wnamebuf0;
    char *_namebuf1;
    wchar_t *_wnamebuf1;
    char *_asctimebuf;
    wchar_t *_wasctimebuf;
    void *_gmtimebuf;
    char *_cvtbuf;
    uchar _con_ch_buf[5];
    ushort _ch_buf_used;
    void *_initaddr;
    void *_initarg;
    void *_pxcptacttab;
    void *_tpxcptinfoptrs;
    int _tfpecode;
    pthreadmbcinfo ptmbcinfo;
    pthreadlocinfo ptlocinfo;
    int _ownlocale;
    ulong _NLG_dwCode;
    void *_terminate;
    void *_unexpected;
    void *_translator;
    void *_purecall;
    void *_curexception;
    void *_curcontext;
    int _ProcessingThrow;
    void *_curexcspec;
    void *_pFrameInfoChain;
    _setloc_struct _setloc_data;
    void *_reserved1;
    void *_reserved2;
    void *_reserved3;
    void *_reserved4;
    void *_reserved5;
    int _cxxReThrow;
    ulong __initDomain;
    int _initapartment;
};

struct __lc_time_data {
    char *wday_abbr[7];
    char *wday[7];
    char *month_abbr[12];
    char *month[12];
    char *ampm[2];
    char *ww_sdatefmt;
    char *ww_ldatefmt;
    char *ww_timefmt;
    int ww_caltype;
    int refcount;
    wchar_t *_W_wday_abbr[7];
    wchar_t *_W_wday[7];
    wchar_t *_W_month_abbr[12];
    wchar_t *_W_month[12];
    wchar_t *_W_ampm[2];
    wchar_t *_W_ww_sdatefmt;
    wchar_t *_W_ww_ldatefmt;
    wchar_t *_W_ww_timefmt;
    wchar_t *_W_ww_locale_name;
};

typedef struct _LocaleUpdate _LocaleUpdate, *P_LocaleUpdate;

struct _LocaleUpdate { // PlaceHolder Structure
};

typedef struct _LDBL12 _LDBL12, *P_LDBL12;

struct _LDBL12 {
    uchar ld12[12];
};

typedef struct _CRT_FLOAT _CRT_FLOAT, *P_CRT_FLOAT;

struct _CRT_FLOAT {
    float f;
};

typedef struct _CRT_DOUBLE _CRT_DOUBLE, *P_CRT_DOUBLE;

struct _CRT_DOUBLE {
    double x;
};

typedef int (*_onexit_t)(void);

typedef ushort wint_t;

typedef longlong __time64_t;

typedef uint size_t;

typedef size_t rsize_t;

typedef int errno_t;

typedef struct localeinfo_struct localeinfo_struct, *Plocaleinfo_struct;

struct localeinfo_struct {
    pthreadlocinfo locinfo;
    pthreadmbcinfo mbcinfo;
};

typedef int intptr_t;

typedef struct localeinfo_struct *_locale_t;




int FUN_30001000(char *param_1);
void FUN_30001060(undefined4 param_1);
int FUN_30001100(undefined4 param_1);
int FUN_30001170(undefined4 param_1,int *param_2,int param_3);
void FUN_300011f0(char *param_1,int param_2,char *param_3,int *param_4);
void FUN_30001260(void);
void FUN_300012f0(undefined4 param_1,undefined4 param_2,int param_3,uint *param_4);
undefined4 FUN_300015e0(undefined4 param_1,int *param_2);
void FUN_300017b0(int *param_1,int param_2,int param_3);
void FUN_30001a80(int param_1,undefined4 param_2,undefined4 param_3,int param_4);
undefined4 FUN_30002230(int param_1,int *param_2);
int FUN_300022d0(undefined4 param_1,int *param_2);
int FUN_30002310(int param_1,int param_2,uint param_3,int param_4,int param_5,int param_6,int param_7,int param_8);
int FUN_30002410(int param_1,undefined4 param_2,short *param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
undefined4 FUN_30002550(int param_1,int param_2,int param_3,undefined4 param_4,undefined4 param_5);
int FUN_300025f0(int param_1,int param_2);
void FUN_30002620(int param_1,int param_2,undefined4 param_3,int param_4);
uint FUN_30002690(int param_1,int param_2,int param_3);
undefined4 FUN_30002720(int param_1,int param_2,undefined4 param_3);
void FUN_30002770(int param_1,int param_2,undefined4 param_3);
void FUN_300027a0(int param_1,int param_2,undefined4 param_3);
int FUN_300027d0(int param_1,int param_2,int param_3,int param_4);
undefined4 FUN_30002860(int param_1,int param_2);
void FUN_30002890(int *param_1);
uint FUN_30002a40(int param_1,int param_2,int param_3,undefined4 param_4);
void FUN_30002b30(void);
void FUN_30002b50(undefined4 param_1);
undefined4 __fastcall FUN_30002be0(int param_1);
int * FUN_30002d70(int param_1);
void __thiscall FUN_30002df0(undefined4 param_1,int param_2);
void FUN_30002fa0(undefined4 param_1,int param_2);
void FUN_30003140(undefined4 param_1);
void FUN_300031d0(undefined4 param_1,int param_2);
undefined * FUN_30003510(int param_1,int param_2);
void FUN_30003540(void);
float10 FUN_30003570(float *param_1,int param_2,int param_3);
undefined ** FUN_300035d0(undefined *param_1);
undefined * FUN_30003640(undefined *param_1);
undefined * FUN_30003670(undefined *param_1);
byte FUN_300036a0(int param_1,int param_2,int param_3);
undefined4 FUN_300036e0(undefined4 param_1);
undefined4 FUN_30003730(undefined4 param_1);
undefined4 __cdecl getPNSStr(undefined4 param_1);
void FUN_300037d0(int param_1,float *param_2,undefined4 *param_3,float param_4);
undefined4 FUN_30003af0(float *param_1,int *param_2);
undefined4 FUN_30003bd0(float param_1,float *param_2,float *param_3,float *param_4,float *param_5);
void FUN_30003cc0(float param_1,int *param_2,float *param_3,float *param_4);
void FUN_30003f90(int param_1);
void FUN_30004120(undefined4 *param_1,int param_2,float *param_3);
void FUN_300043c0(undefined4 param_1,undefined4 param_2,int param_3);
void FUN_300043f0(int param_1,undefined4 *param_2,undefined4 param_3,int param_4);
undefined4 FUN_30004740(undefined4 param_1);
undefined4 FUN_300047b0(int param_1);
int FUN_30004820(int param_1);
undefined * FUN_30004850(undefined4 param_1);
void FUN_300048a0(undefined4 param_1,undefined4 *param_2);
undefined * FUN_30004910(undefined4 param_1);
undefined * FUN_30004960(undefined4 param_1,undefined1 *param_2,undefined4 *param_3);
void FUN_30004a00(int param_1,undefined4 param_2);
float10 FUN_30004a50(undefined4 param_1);
void FUN_30004b80(void);
undefined * FUN_30004c90(int param_1,undefined4 *param_2);
void FUN_30004cd0(undefined4 param_1,int param_2);
void FUN_30004d00(int param_1,int param_2);
void FUN_30004d40(float *param_1,float *param_2);
void FUN_30004dc0(undefined4 param_1,undefined4 *param_2);
void FUN_30004e40(undefined4 param_1,undefined4 param_2,undefined4 param_3);
undefined4 FUN_30004ec0(undefined4 param_1);
int FUN_30004f10(char *param_1,char *param_2,int param_3,int param_4);
void FUN_30004f70(char *param_1,float *param_2,float param_3,undefined4 param_4);
undefined4 FUN_30005260(int param_1);
undefined4 FUN_300052a0(int param_1);
void FUN_300052c0(float *param_1,float *param_2);
undefined * FUN_300053d0(void);
undefined4 FUN_30005440(int param_1,int param_2,int param_3,float *param_4);
byte FUN_300054a0(uint param_1);
undefined4 FUN_30005520(int *param_1);
size_t FUN_30005950(char *param_1,size_t param_2,char *param_3,va_list param_4);
void FUN_30005980(int param_1);
undefined4 FUN_30005a60(char *param_1,char *param_2);
void FUN_30005ad0(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4);
void FUN_30005b00(undefined4 *param_1,int param_2,float *param_3,int param_4,undefined4 param_5);
void FUN_30006590(undefined4 param_1,undefined4 param_2);
void FUN_30006620(undefined4 param_1,float *param_2);
undefined4 FUN_300066e0(undefined4 param_1,int param_2);
undefined4 FUN_30006730(undefined4 param_1,int param_2);
void FUN_30006780(undefined4 param_1,int *param_2);
void FUN_30006840(undefined4 param_1);
void FUN_30006860(undefined4 param_1,undefined4 param_2);
undefined4 FUN_30006880(undefined4 param_1);
undefined4 FUN_300068c0(void);
undefined4 FUN_300068d0(undefined4 param_1);
undefined4 FUN_30006910(undefined4 param_1);
undefined4 FUN_30006960(undefined4 param_1);
undefined4 FUN_300069b0(int param_1);
undefined4 FUN_30006a00(undefined4 param_1);
undefined4 FUN_30006a60(int param_1);
void FUN_30006a90(int param_1);
void FUN_30006ad0(uint param_1);
void FUN_30006b10(uint param_1);
void FUN_30006b50(float *param_1,float *param_2,float *param_3,float param_4);
void FUN_30006be0(int *param_1,float *param_2,float *param_3,float *param_4,int param_5,int param_6,code *param_7,undefined4 param_8,uint param_9);
void FUN_30006e80(int *param_1,undefined4 param_2,float *param_3,float *param_4);
void FUN_30006fb0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30006fd0(void);
void __fastcall FUN_300071c0(undefined4 param_1,float *param_2,float param_3,float param_4);
float10 __fastcall FUN_30007280(int param_1);
void FUN_30007440(void);
undefined4 FUN_30007590(void);
undefined4 FUN_30007640(void);
undefined4 FUN_30007710(void);
void FUN_30007840(void);
void __fastcall FUN_30007be0(undefined4 param_1);
void FUN_30007c30(void);
void FUN_30007e60(void);
void FUN_30007f70(void);
void FUN_30008150(void);
void __fastcall FUN_30008520(undefined4 param_1);
void FUN_300085b0(void);
void FUN_300087b0(void);
void FUN_300087c0(void);
undefined4 FUN_30008b10(void);
void FUN_30008ca0(void);
void FUN_30008d90(void);
void FUN_30009030(void);
void FUN_300091a0(void);
void FUN_300093e0(void);
void FUN_30009900(void);
void FUN_300099a0(void);
void FUN_30009b70(void);
void FUN_30009ec0(void);
void FUN_30009f70(void);
void FUN_30009fc0(void);
void FUN_3000a0d0(int param_1,int param_2);
undefined4 FUN_3000a1b0(int param_1);
bool FUN_3000a260(undefined4 param_1);
void FUN_3000a290(void);
void FUN_3000a3b0(void);
undefined4 FUN_3000a770(void);
undefined4 FUN_3000aa60(void);
undefined4 FUN_3000aac0(int param_1);
void FUN_3000aaf0(void);
undefined4 FUN_3000ad10(void);
int __fastcall FUN_3000af10(int param_1);
undefined4 FUN_3000af20(void);
bool FUN_3000af70(void);
void FUN_3000afc0(void);
void FUN_3000b070(int param_1,int param_2,int param_3);
void FUN_3000b3c0(int param_1,int param_2,int param_3,undefined4 param_4,undefined4 param_5);
void FUN_3000be40(void);
void FUN_3000c150(void);
void FUN_3000c420(void);
void FUN_3000c610(void);
undefined * FUN_3000c680(int param_1,int param_2);
char * FUN_3000c6b0(undefined4 param_1);
char * FUN_3000c700(undefined4 param_1);
char * FUN_3000c750(undefined4 param_1);
undefined4 FUN_3000c7a0(uint param_1);
void FUN_3000c7e0(int param_1,int param_2,int param_3);
void FUN_3000cba0(int param_1);
void FUN_3000cda0(void);
void FUN_3000cfc0(void);
void FUN_3000ebe0(void);
void FUN_3000f070(int *param_1);
uint FUN_3000f760(int *param_1);
void FUN_3000f890(int param_1);
void FUN_30010020(undefined4 param_1);
void FUN_30010520(void);
undefined4 FUN_30010530(void);
int FUN_30010540(int param_1);
undefined * FUN_30010560(int param_1);
undefined4 FUN_30010580(int param_1);
undefined4 FUN_300105d0(undefined4 *param_1);
void FUN_30010610(undefined4 param_1);
void __fastcall FUN_300106a0(undefined4 param_1);
void FUN_30010cb0(undefined4 param_1);
undefined4 FUN_30010e10(int param_1);
undefined4 FUN_30010e40(int param_1);
undefined4 FUN_30010e60(int param_1);
void FUN_30010e80(void);
void FUN_30011db0(undefined4 param_1,float *param_2,float *param_3);
void __fastcall FUN_30012330(float *param_1,float *param_2);
void FUN_300123a0(int *param_1,int *param_2);
float10 FUN_300123e0(void);
float10 FUN_30012480(void);
void FUN_30012520(undefined4 param_1);
void FUN_300125b0(undefined4 param_1,float param_2);
void __fastcall FUN_30012870(undefined4 param_1);
void __thiscall FUN_300129e0(undefined4 param_1,int param_2,int param_3);
void FUN_300139c0(void);
void __fastcall FUN_30013a80(int param_1,undefined4 param_2,undefined4 param_3,int param_4);
void FUN_30013ca0(uint param_1,int param_2);
void FUN_30013fe0(int param_1);
undefined4 FUN_300140b0(void);
bool __fastcall FUN_300140e0(int param_1);
undefined4 FUN_300140f0(float *param_1,float *param_2,float param_3);
undefined4 FUN_300148b0(float *param_1,float *param_2);
undefined4 FUN_30014ab0(float *param_1);
void FUN_30014f30(void);
undefined4 FUN_30015000(void);
void FUN_300151f0(float *param_1);
void FUN_30015280(undefined4 *param_1);
void FUN_30015320(void);
void FUN_30015820(void);
undefined4 FUN_30015960(void);
undefined4 FUN_30015a20(undefined4 param_1,int param_2);
void __fastcall FUN_30015b80(int param_1);
void __fastcall FUN_30015d90(undefined4 param_1);
undefined4 FUN_30015e70(undefined4 param_1,undefined4 param_2,undefined4 *param_3);
void FUN_30015f10(undefined4 *param_1);
void FUN_30015f90(undefined4 param_1,int param_2);
int FUN_30016340(int param_1,int param_2);
void FUN_300163f0(int param_1);
void FUN_30016470(void);
void FUN_30016580(float *param_1,float *param_2);
int FUN_300165d0(int param_1);
undefined4 __fastcall FUN_30016610(int *param_1,int param_2,int param_3);
undefined4 __fastcall FUN_300166f0(int *param_1,float *param_2,int param_3);
void FUN_300167d0(void);
void FUN_300168f0(int param_1);
void FUN_30016970(int *param_1,int *param_2,undefined4 param_3);
void FUN_30016b80(int param_1,int param_2);
void FUN_30016bf0(float param_1,float param_2,float param_3,float param_4,int param_5);
undefined4 FUN_300176b0(int param_1);
void FUN_30017710(int param_1,int param_2,int param_3,int param_4,undefined4 param_5,int param_6);
undefined4 FUN_30017b30(void);
void FUN_30017c80(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30017d10(int param_1,float param_2,float param_3,float param_4,float param_5,byte param_6,int param_7,int param_8,int param_9,int param_10);
void FUN_30018d40(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,int param_7);
void FUN_30019320(float param_1,float param_2,float param_3,float param_4,undefined4 param_5,float param_6,int param_7,undefined4 param_8,float param_9);
void FUN_30019a70(void);
void FUN_30019f90(void);
undefined4 FUN_3001a200(undefined1 *param_1);
void FUN_3001a3c0(void);
void FUN_3001a5e0(void);
void FUN_3001a6a0(void);
void FUN_3001a780(void);
void FUN_3001a9f0(void);
void FUN_3001aa40(void);
void FUN_3001aa90(void);
void FUN_3001aaf0(void);
void FUN_3001ac90(void);
void FUN_3001af40(void);
void FUN_3001af60(void);
void FUN_3001b040(void);
void FUN_3001b080(void);
void FUN_3001b0a0(void);
void FUN_3001b280(void);
void FUN_3001b2c0(void);
undefined4 FUN_3001b4d0(void);
void FUN_3001b5e0(void);
void FUN_3001b8b0(void);
void FUN_3001ba00(void);
void FUN_3001bbc0(int param_1);
void FUN_3001be60(void);
void FUN_3001be90(void);
void FUN_3001bfb0(void);
void FUN_3001c0b0(void);
void FUN_3001c5e0(int param_1);
undefined * FUN_3001cb00(int param_1);
void FUN_3001cb40(void);
void FUN_3001cb90(void);
void FUN_3001cbf0(void);
void FUN_3001cc50(void);
undefined4 FUN_3001cca0(undefined4 param_1);
undefined4 FUN_3001cd30(int param_1);
uint FUN_3001cd70(int param_1);
undefined4 FUN_3001ce10(int param_1);
void FUN_3001ce50(int param_1,undefined4 param_2);
void FUN_3001cea0(int param_1,undefined4 *param_2);
void FUN_3001cf40(undefined4 param_1);
void FUN_3001d190(int param_1,int param_2);
void FUN_3001d220(int param_1);
void FUN_3001d9e0(int param_1);
void FUN_3001ddb0(float param_1);
int FUN_3001df00(int param_1,int param_2,int param_3);
void FUN_3001e110(void);
void FUN_3001e120(float *param_1,int param_2);
void FUN_3001e2e0(float *param_1,int param_2,int param_3,int param_4,undefined4 param_5,int param_6);
char * FUN_3001e4d0(int param_1);
char FUN_3001e4f0(void);
char FUN_3001e530(int param_1);
void FUN_3001e670(void);
void FUN_3001e680(void);
void FUN_3001e6f0(int param_1);
void FUN_3001e810(int param_1);
undefined4 FUN_3001e9d0(void);
void FUN_3001eb80(int param_1);
void FUN_3001ec60(int param_1);
void FUN_3001edc0(float param_1);
void FUN_3001f150(int param_1,int param_2);
int * FUN_3001f2a0(void);
void FUN_3001f380(int param_1);
void FUN_3001f620(int param_1);
void FUN_3001f8f0(int param_1);
void FUN_30020390(float param_1);
void FUN_300205f0(int param_1);
void FUN_300206d0(int param_1);
void FUN_300207b0(float param_1);
void FUN_300209d0(undefined4 param_1);
void FUN_300209e0(char *param_1,undefined4 param_2,int param_3);
void FUN_30020aa0(byte *param_1,undefined4 param_2,int param_3,int param_4);
void FUN_30020b80(undefined4 param_1,undefined4 param_2,float param_3,float param_4,float param_5,float param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,undefined4 param_10,undefined4 param_11);
void FUN_30020c10(float param_1,float param_2,float param_3,float param_4,undefined4 *param_5,byte *param_6,float param_7,int param_8,int param_9,int param_10);
void FUN_30020ef0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8);
void FUN_30020f50(int param_1,int param_2,int param_3,uint param_4,int param_5,int param_6,int param_7,int param_8);
float10 FUN_300210f0(float param_1,int param_2);
float10 FUN_30021170(float param_1,int param_2);
float10 FUN_30021320(float param_1,int param_2);
float10 FUN_30021520(float param_1);
void FUN_300217e0(float param_1,int param_2);
void FUN_30021a30(void);
void FUN_30021a60(undefined4 *param_1);
void FUN_30021b30(undefined4 param_1);
float10 FUN_30021c20(float param_1);
void FUN_30022300(void);
char * FUN_300226d0(int param_1);
void FUN_30022760(void);
void FUN_30022800(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_300228c0(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4);
void FUN_300229a0(void);
void FUN_30022b30(void);
void FUN_300230b0(void);
void FUN_300240d0(void);
void FUN_300242b0(void);
void FUN_300246a0(void);
void FUN_300247f0(int param_1);
void FUN_30024910(undefined4 *param_1);
void FUN_30024a20(void);
void FUN_30024be0(void);
int FUN_30024ec0(void);
void FUN_30024f90(void);
void FUN_300257a0(void);
void FUN_30025860(void);
int FUN_30025980(int param_1);
void FUN_30025a10(undefined4 param_1);
void FUN_30025a30(void);
undefined4 FUN_30025be0(void);
void FUN_30025f60(void);
void FUN_300261d0(void);
void FUN_300262a0(void);
void FUN_30026390(void);
void FUN_30026590(void);
void FUN_300265a0(void);
void FUN_300265e0(undefined4 param_1,undefined4 param_2);
void FUN_300266b0(void);
void FUN_30026a00(void);
void FUN_30026a40(float param_1,float param_2,float param_3,float param_4,float *param_5,float *param_6,undefined4 param_7,float param_8,int param_9);
void FUN_30026c30(void);
int FUN_30026fc0(int *param_1,undefined4 *param_2);
void FUN_300271c0(void);
void FUN_30027300(void);
void FUN_30027400(void);
void FUN_300275f0(void);
int FUN_30027860(void);
undefined4 FUN_300279c0(float param_1,int param_2);
undefined4 FUN_30027bb0(int param_1,int param_2);
void FUN_30027c10(void);
void FUN_30027e70(void);
void FUN_30028060(void);
void FUN_30028070(void);
void FUN_300280b0(undefined4 param_1);
void FUN_30028130(void);
void FUN_30028300(void);
void FUN_30028470(void);
void FUN_30028720(undefined4 param_1);
void FUN_30028910(void);
void FUN_30028b30(float param_1);
void FUN_30028bb0(void);
void FUN_30028cc0(void);
void FUN_30028e50(void);
void FUN_30028fa0(undefined4 *param_1);
void FUN_30029230(int param_1,int param_2,undefined4 param_3,int param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9);
void FUN_300292c0(float param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,undefined4 param_10);
float10 FUN_30029340(float param_1,undefined1 *param_2);
void FUN_30029720(void);
void FUN_300298e0(void);
void FUN_30029a50(void);
void FUN_3002a3f0(void);
void FUN_3002a840(void);
void FUN_3002ac70(void);
void FUN_3002b910(void);
int FUN_3002bda0(int param_1);
void FUN_3002bf10(undefined4 param_1);
void FUN_3002bf20(float *param_1,float *param_2,float *param_3,float *param_4);
void FUN_3002bf90(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
void FUN_3002c010(float param_1,float param_2,float param_3,float param_4,float *param_5,float *param_6,undefined4 *param_7,float param_8,uint param_9);
void FUN_3002c2d0(float param_1,undefined4 param_2,float param_3,undefined4 param_4,float param_5);
void FUN_3002c390(undefined4 param_1,float param_2,undefined4 param_3,float param_4,float param_5);
void FUN_3002c450(float param_1,undefined4 param_2,float param_3,undefined4 param_4,float param_5);
void FUN_3002c510(undefined4 param_1,float param_2,undefined4 param_3,float param_4,float param_5);
void FUN_3002c5d0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void FUN_3002c640(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,int param_5,undefined4 param_6);
void FUN_3002c6c0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9);
void FUN_3002c730(float *param_1,float *param_2,float *param_3,float *param_4);
void FUN_3002c780(undefined4 param_1,undefined4 param_2,float param_3,float param_4,undefined4 param_5);
void FUN_3002c850(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
void FUN_3002c8d0(undefined4 param_1,undefined4 param_2,float param_3,float param_4,undefined4 param_5);
void FUN_3002c9a0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void FUN_3002ca10(int param_1,int param_2,int param_3,int param_4,char param_5);
void FUN_3002cb00(int param_1,int param_2,int param_3,int param_4,char param_5);
void FUN_3002cbf0(int param_1,int param_2,char *param_3,undefined4 *param_4,int param_5,int param_6,int param_7,undefined4 param_8,int param_9);
void FUN_3002cdb0(int param_1,int param_2,char *param_3,undefined4 *param_4,int param_5,int param_6,int param_7,int param_8,int param_9);
void FUN_3002cf60(void);
void FUN_3002cf80(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3002cfc0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
int FUN_3002d000(char *param_1);
void FUN_3002d030(int param_1,int param_2,int param_3,int param_4);
void FUN_3002d0d0(void);
undefined * FUN_3002d170(int param_1,int param_2);
void FUN_3002d1d0(undefined4 *param_1);
void FUN_3002d260(float *param_1,float *param_2,undefined4 param_3);
int FUN_3002d510(undefined4 *param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,int param_9,int param_10,undefined4 param_11,undefined4 param_12);
void FUN_3002d710(undefined4 *param_1,float *param_2,int param_3,int param_4,int param_5,float param_6);
void FUN_3002d910(float *param_1,int param_2);
void FUN_3002dbf0(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,int param_5,float param_6,undefined4 param_7);
int FUN_3002deb0(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,float *param_5,int param_6);
void FUN_3002df90(undefined4 param_1,float param_2);
void FUN_3002e1f0(void);
undefined4 * FUN_3002e240(void);
int FUN_3002e280(void);
void FUN_3002e2e0(float param_1);
undefined4 FUN_3002e3c0(int param_1,undefined4 param_2);
void FUN_3002e4d0(int param_1,int param_2);
void FUN_3002e7e0(void);
void FUN_3002ec00(int param_1,float *param_2,int param_3,undefined4 param_4,int param_5);
void FUN_3002eef0(int param_1,float *param_2,float *param_3);
void FUN_3002f580(int param_1,int param_2,undefined4 param_3,undefined4 param_4,float *param_5);
void FUN_3002f680(int param_1,int param_2,undefined4 param_3);
void FUN_3002f760(undefined4 *param_1);
void FUN_3002f7e0(undefined4 *param_1);
uint FUN_3002fac0(void);
void __fastcall FUN_30030510(undefined4 param_1);
undefined4 FUN_300305a0(int param_1,int param_2,undefined4 param_3);
void FUN_300306a0(void);
void FUN_30030f60(void);
void FUN_30031240(void);
void __fastcall FUN_300312d0(int param_1,int param_2);
void FUN_300313f0(void);
void __fastcall FUN_30031cd0(int param_1);
void FUN_30031e50(void);
void FUN_30032100(void);
void FUN_30032570(int param_1);
void FUN_30032720(int param_1);
void FUN_30032890(void);
void FUN_30032c80(int param_1,int param_2);
void FUN_30032e90(void);
void FUN_30032ee0(void);
void FUN_30033040(void);
undefined4 FUN_300331e0(int param_1,int param_2,int param_3);
void FUN_30033710(int param_1,undefined4 *param_2);
void FUN_30033760(int param_1,undefined4 *param_2);
void FUN_300337b0(float *param_1,int param_2,undefined4 param_3,undefined4 param_4,float *param_5,float *param_6);
void FUN_30033ad0(int *param_1);
void FUN_30033d80(int param_1);
void FUN_30033db0(int param_1);
undefined4 FUN_30033f90(int param_1);
void FUN_30034010(int param_1,void *param_2,void *param_3,void *param_4,void *param_5,void *param_6,float *param_7,int param_8,int param_9);
void FUN_300343e0(int param_1);
void FUN_30034680(void);
void __fastcall FUN_300348e0(int param_1);
void __fastcall FUN_300351a0(int param_1);
void FUN_30035360(void);
void FUN_300353a0(undefined4 *param_1,float *param_2,int param_3,int param_4,float param_5,int param_6,int param_7,float param_8,float param_9);
void FUN_30035b50(undefined4 *param_1,float *param_2,int param_3,int param_4,int param_5,int param_6,int param_7);
void FUN_300362f0(int param_1,undefined4 *param_2,float *param_3);
void FUN_300366b0(int param_1,undefined4 *param_2,undefined4 *param_3);
void FUN_30036850(int param_1,float *param_2,int param_3,int param_4);
void FUN_30036980(int param_1,undefined4 *param_2);
void __fastcall FUN_300369e0(undefined4 *param_1);
undefined4 FUN_30036a00(int param_1);
void FUN_30036a20(int param_1);
void FUN_30036a40(undefined4 *param_1);
void FUN_30036b10(void);
void __fastcall FUN_30036c30(float *param_1,float *param_2,int param_3,int param_4,int param_5);
void FUN_30037060(int param_1,undefined4 param_2,undefined4 param_3,int param_4);
void FUN_300371b0(int param_1,undefined4 param_2,undefined4 param_3,int param_4);
void FUN_30037330(int param_1,float *param_2,float *param_3);
void FUN_30037690(int *param_1,undefined4 param_2);
void FUN_30039680(int param_1);
undefined4 FUN_30039780(int param_1);
uint FUN_300397b0(undefined4 param_1);
undefined4 * FUN_300397d0(int param_1);
undefined4 FUN_30039840(undefined4 param_1);
int FUN_30039890(int param_1);
void FUN_30039900(undefined4 param_1);
undefined4 FUN_30039fe0(int param_1,int param_2);
void FUN_3003a060(void);
uint FUN_3003a140(undefined4 param_1,undefined4 param_2);
void FUN_3003a210(void);
void FUN_3003a250(void);
int FUN_3003a690(int param_1);
void FUN_3003a770(float param_1,int param_2);
int FUN_3003a850(void);
int FUN_3003a8c0(void);
int FUN_3003a920(int param_1,int *param_2);
int FUN_3003a9f0(int param_1,int *param_2);
void FUN_3003aab0(float param_1,int *param_2);
void FUN_3003ac60(float param_1,int *param_2);
void FUN_3003b340(void);
void FUN_3003b350(void);
undefined4 FUN_3003b360(uint param_1,int param_2);
void FUN_3003bb60(undefined4 param_1,int param_2);
void FUN_3003bb80(float *param_1,float *param_2,float param_3,float *param_4);
void FUN_3003bbd0(int param_1,float param_2);
void FUN_3003bc20(void);
int * FUN_3003bcb0(int param_1);
void FUN_3003bd60(int *param_1);
void FUN_3003be00(int param_1,int param_2);
void FUN_3003be90(int param_1,int param_2,float *param_3);
void FUN_3003bf00(int param_1);
void FUN_3003c210(int param_1,float param_2,float param_3);
void FUN_3003c7a0(int param_1);
void FUN_3003cfd0(void);
void FUN_3003d070(void);
void FUN_3003d1a0(void);
void FUN_3003d2d0(int *param_1,undefined4 *param_2,int *param_3,float param_4,int param_5,int param_6);
void FUN_3003da10(char *param_1);
void FUN_3003da50(void);
void FUN_3003da90(int *param_1);
void FUN_3003db00(int *param_1);
void FUN_3003e1d0(void);
void FUN_3003ec00(void);
void FUN_3003f190(void);
void FUN_3003f570(void);
void FUN_3003f8a0(undefined4 *param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,int param_6,undefined4 param_7,int param_8);
void FUN_3003fcf0(float param_1,float param_2,float param_3,float param_4,int param_5,int param_6);
void __thiscall FUN_30040040(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,float param_6,undefined4 param_7,undefined4 param_8,float param_9);
undefined4 FUN_30040130(int param_1);
byte FUN_30040150(int param_1);
undefined4 FUN_300401d0(int param_1);
bool FUN_300401f0(int param_1);
undefined4 FUN_30040210(int param_1);
bool FUN_300402d0(int param_1);
void FUN_300402e0(int param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4);
void FUN_30040340(void);
void FUN_300403d0(undefined4 param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,undefined4 *param_5,undefined4 *param_6,undefined4 *param_7,undefined4 *param_8);
undefined4 FUN_30040660(void);
void FUN_30040670(void);
undefined4 FUN_30040690(void);
void FUN_300406a0(void);
void FUN_300406c0(void);
void FUN_300406e0(int param_1,undefined4 param_2);
uint FUN_30040720(int param_1);
uint FUN_30040780(uint param_1);
undefined4 FUN_30040820(int param_1);
undefined4 FUN_30040850(undefined4 param_1);
uint FUN_300408d0(int param_1);
void FUN_30040a40(int param_1);
void FUN_30040bb0(int param_1);
void FUN_30040dd0(float param_1);
undefined4 FUN_300411a0(void);
void FUN_30041700(int param_1);
undefined4 FUN_30041b60(int param_1,int param_2);
void FUN_30041bb0(float *param_1,float param_2,float param_3,undefined4 param_4,int param_5);
int FUN_30041f40(int param_1);
undefined4 FUN_30042180(int param_1);
void FUN_30042220(float param_1);
uint FUN_300427b0(int param_1);
void FUN_30042830(void);
void FUN_300428a0(void);
undefined4 FUN_30042930(void);
void FUN_30042a40(void);
int FUN_30042a50(int param_1,int param_2,int param_3);
void FUN_30042b70(int param_1);
undefined4 FUN_30042b90(void);
void FUN_30042bd0(int param_1);
void FUN_30042d40(void);
void FUN_30042d60(void);
void FUN_30043150(void);
void FUN_30043170(undefined4 param_1);
void FUN_30043390(void);
void FUN_30043590(void);
void FUN_300436e0(int param_1);
float * FUN_300442c0(float *param_1);
char * FUN_30044370(undefined4 param_1);
void FUN_300443c0(void);
void FUN_30044730(void);
void FUN_30044790(int *param_1);
undefined * FUN_300447d0(int param_1,int param_2);
undefined4 * FUN_30044820(void);
void FUN_30044890(int param_1);
void FUN_300449a0(int param_1,int param_2);
void FUN_30044a70(int param_1,int param_2);
void FUN_30044c20(int param_1,int *param_2);
void FUN_30044e50(int param_1);
void FUN_30044ec0(int param_1);
void FUN_30045ab0(int param_1);
void FUN_30045b30(int param_1);
void FUN_30045ce0(float param_1);
void FUN_30045e10(int param_1);
void FUN_30045fb0(int param_1);
void FUN_30046190(int param_1);
void FUN_30046280(int param_1);
void FUN_30046350(void);
void FUN_30046490(void);
void FUN_30046570(void);
void FUN_30046650(void);
void FUN_30046710(void);
void FUN_300468e0(void);
void FUN_30046a10(void);
void FUN_30046af0(void);
undefined4 FUN_30046bb0(void);
void FUN_30046bd0(undefined4 param_1);
void FUN_30046cd0(undefined4 param_1);
void FUN_30046d30(undefined4 param_1,undefined4 param_2);
void FUN_30046d90(undefined4 param_1);
undefined * FUN_30046df0(undefined4 param_1);
void FUN_30046e10(void);
void FUN_30046e90(void);
void FUN_300471c0(void);
void FUN_30047210(void);
undefined * FUN_300481f0(int param_1);
void FUN_30048260(void);
undefined4 * FUN_300483f0(int *param_1);
undefined4 FUN_30048540(void);
void FUN_300486f0(void);
void FUN_30048940(void);
undefined4 FUN_30048a50(undefined4 param_1);
void FUN_30048aa0(void);
void FUN_30048bf0(void);
undefined4 FUN_30048c40(undefined4 param_1);
void FUN_30048c60(void);
undefined4 FUN_300494c0(int param_1);
void FUN_300494d0(char *param_1,int param_2);
void FUN_30049650(void);
void FUN_300496b0(undefined4 param_1);
void FUN_30049760(void);
void FUN_30049860(void);
void FUN_30049ab0(void);
void FUN_30049c20(void);
void FUN_3004b1d0(void);
void FUN_3004b210(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,int param_5);
undefined4 vmMain(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void FUN_3004b940(void);
void FUN_3004b990(int *param_1);
void FUN_3004b9d0(undefined4 param_1,int param_2,undefined4 *param_3,float param_4,float param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,int param_10);
void FUN_3004bb10(void);
void FUN_3004bce0(undefined4 param_1,undefined4 param_2,int param_3);
void FUN_3004c0b0(undefined4 param_1,undefined4 param_2,int param_3);
void FUN_3004c4e0(void);
void FUN_3004c5f0(void);
int __fastcall FUN_3004c750(int param_1);
void FUN_3004c7c0(float *param_1,undefined4 param_2,int param_3,undefined4 *param_4);
void FUN_3004c9c0(float *param_1);
void FUN_3004cea0(undefined4 *param_1);
void FUN_3004cf80(undefined4 *param_1);
void FUN_3004d030(int param_1,int param_2);
void FUN_3004d180(int param_1,int param_2);
void FUN_3004d320(int param_1,int param_2);
void FUN_3004d420(int param_1);
void FUN_3004d560(int param_1);
void FUN_3004d600(void);
float10 FUN_3004db40(float param_1,int param_2);
void FUN_3004dda0(int param_1,int param_2);
void FUN_3004de40(void);
char FUN_3004e230(int param_1);
void FUN_3004e270(int param_1,float param_2,int *param_3,undefined4 param_4,float param_5);
void FUN_3004e6b0(void);
void FUN_3004e7e0(void);
void FUN_3004e8b0(void);
void FUN_3004e9a0(float param_1);
void FUN_3004eb80(void);
void FUN_3004ec50(void);
void FUN_3004ee10(void);
void FUN_3004ee30(void);
void FUN_3004ef20(void);
void FUN_3004f090(void);
void FUN_3004f2b0(void);
void FUN_3004f360(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void __fastcall FUN_3004f3e0(undefined4 param_1);
void __fastcall FUN_3004f530(undefined4 param_1);
void FUN_3004f6c0(void);
undefined * FUN_3004f780(int param_1);
void FUN_3004f830(int param_1);
void FUN_3004fc90(void);
void __thiscall FUN_3004fd50(undefined4 param_1,int param_2);
void FUN_3004fe10(int param_1,char *param_2);
void FUN_3004fe40(void);
void FUN_30050000(void);
int FUN_300502b0(int param_1,int param_2,float param_3,int param_4);
undefined4 FUN_300506f0(void);
bool FUN_300508a0(char param_1);
char * FUN_300508b0(char *param_1,char *param_2);
undefined4 FUN_30050920(void);
undefined4 FUN_300509e0(undefined4 param_1);
char * FUN_30050a20(int param_1);
long FUN_30050a50(char *param_1);
void FUN_30050aa0(int param_1);
void FUN_30050b90(void);
undefined1 * FUN_30050d50(undefined4 param_1,char *param_2,char *param_3);
undefined4 FUN_30050e60(int param_1,uint param_2,int param_3);
undefined4 FUN_30050f10(int param_1);
undefined4 FUN_30051050(void);
void FUN_300510a0(int param_1,int param_2,char *param_3);
void FUN_30051220(undefined4 param_1,undefined4 param_2,undefined4 param_3);
bool FUN_30051340(void);
bool FUN_300514d0(void);
undefined4 FUN_30051540(void);
void FUN_30051570(void);
void FUN_300515d0(uint param_1);
void FUN_30051600(undefined4 param_1);
bool FUN_30051630(uint param_1,int param_2);
byte FUN_30051680(void);
void FUN_300516c0(void);
void FUN_300517c0(int param_1,float *param_2);
void FUN_30053760(void);
void FUN_30053a60(int param_1,int param_2);
void FUN_30053c30(int param_1,int param_2);
void FUN_30054090(undefined4 *param_1,undefined4 *param_2,int param_3);
void FUN_30054160(undefined4 *param_1,undefined4 *param_2,int param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
void FUN_30054230(undefined4 param_1,undefined4 *param_2,undefined4 *param_3,int param_4,int param_5,int param_6,int param_7);
void FUN_300543a0(undefined4 param_1,undefined4 *param_2,undefined4 *param_3,int param_4,int param_5,int param_6,int param_7);
void FUN_30054510(int param_1,int param_2);
void FUN_30054560(int param_1,undefined4 *param_2,int param_3,int param_4,int param_5,int param_6,undefined4 param_7,float param_8);
void FUN_300546c0(undefined4 param_1,undefined4 param_2);
int FUN_30054700(int param_1,undefined4 *param_2,float *param_3,int param_4,undefined4 param_5);
void FUN_30054830(int param_1,int param_2);
void FUN_300549b0(int param_1);
int FUN_30054a20(int param_1,undefined4 *param_2,undefined4 param_3);
void FUN_30054bf0(undefined4 *param_1,undefined4 *param_2,int param_3,float param_4,float param_5,float param_6);
int FUN_30054e00(int param_1,float *param_2,float *param_3);
undefined4 FUN_30055160(int *param_1);
bool FUN_300551c0(undefined4 param_1,uint param_2);
void FUN_300551f0(int param_1);
void FUN_30055260(undefined4 param_1,int param_2);
void __thiscall FUN_300552e0(undefined4 param_1,uint param_2);
void FUN_30055370(undefined4 param_1,int param_2,undefined4 *param_3,int param_4);
void __fastcall FUN_300554d0(undefined4 param_1,undefined4 param_2);
void FUN_30055510(int param_1,undefined4 param_2,int param_3,uint param_4);
void FUN_300556c0(undefined4 param_1,int *param_2,int param_3,int *param_4,int param_5);
void FUN_30055bd0(undefined4 param_1,undefined4 param_2,undefined4 *param_3,undefined4 param_4);
void FUN_30055c20(int param_1,int param_2,uint param_3);
void FUN_30055d20(undefined4 param_1,int *param_2,int param_3,int param_4);
void FUN_30055e70(void);
void __thiscall FUN_30056030(float param_1,float param_2,float param_3,float param_4,float param_5);
void __fastcall FUN_30056210(int param_1);
void FUN_300562d0(int param_1,int param_2,int param_3,int param_4);
void FUN_30056660(undefined *param_1,undefined4 *param_2,undefined4 *param_3,undefined4 param_4);
void FUN_30056b40(void);
undefined4 FUN_30056cb0(float *param_1,float *param_2,float *param_3);
void FUN_30056df0(undefined4 param_1,undefined4 *param_2);
void FUN_30056fb0(int param_1,undefined4 param_2,int param_3);
void FUN_30057020(undefined4 param_1,float *param_2);
void FUN_30057160(int *param_1,undefined4 param_2,int param_3);
void FUN_30057240(undefined4 param_1,int param_2);
void FUN_30057420(void);
void FUN_30057730(float *param_1);
void FUN_30057b30(void);
void FUN_30057d50(undefined4 *param_1,undefined4 param_2,int param_3,int *param_4,undefined4 *param_5);
void FUN_30058000(int param_1,int *param_2);
void FUN_30058180(undefined4 *param_1);
void FUN_30058300(undefined4 *param_1,undefined4 *param_2,int param_3);
undefined4 FUN_300584c0(int param_1,undefined4 param_2,float *param_3);
undefined4 FUN_300585e0(int param_1,undefined4 param_2,float *param_3);
void FUN_30058700(int param_1,int param_2,uint param_3);
void FUN_30058750(undefined4 param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4);
void FUN_300587a0(undefined4 param_1,undefined4 *param_2,int param_3,undefined4 param_4,undefined4 param_5);
void FUN_300588e0(undefined4 param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,undefined4 *param_5,undefined4 param_6,undefined4 param_7);
void FUN_30058920(void);
void FUN_30058950(int param_1,char *param_2);
void FUN_30059140(uint *param_1);
void FUN_3005a330(void);
void FUN_3005a3d0(int param_1,int param_2,float param_3);
char * FUN_3005a770(undefined4 param_1);
void FUN_3005a7b0(int param_1);
void FUN_3005a9b0(int param_1,int param_2);
void FUN_3005aa40(int param_1,int param_2);
void FUN_3005ad80(undefined4 *param_1,undefined4 *param_2);
undefined * FUN_3005af90(int param_1,int param_2,int param_3);
void FUN_3005b050(void);
void FUN_3005b070(void);
void FUN_3005b0b0(void);
void FUN_3005b1a0(void);
void FUN_3005b1f0(int *param_1,int param_2);
undefined * FUN_3005b210(void);
undefined * FUN_3005b240(void);
void FUN_3005b2d0(int param_1,char *param_2,int param_3,int *param_4);
void FUN_3005b440(int *param_1);
void FUN_3005b480(int param_1,undefined4 param_2,int param_3,int *param_4);
void FUN_3005b560(void);
void FUN_3005bad0(void);
undefined4 FUN_3005bc40(int param_1);
void FUN_3005c030(int param_1);
undefined4 FUN_3005c180(int param_1);
void FUN_3005c1d0(char *param_1,undefined4 param_2,char *param_3,undefined4 *param_4,undefined4 param_5);
void FUN_3005c360(void);
void FUN_3005c490(void);
void __fastcall FUN_3005c570(undefined4 param_1,int *param_2,undefined4 param_3,undefined4 param_4,int param_5,undefined4 param_6,int param_7);
void FUN_3005c7e0(float *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
void FUN_3005c880(float *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
void FUN_3005c920(float *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
uint FUN_3005c9a0(undefined4 param_1,int param_2);
void FUN_3005ca20(int param_1);
void FUN_3005cba0(void);
undefined4 FUN_3005ce10(int param_1,int *param_2);
void FUN_3005d5b0(void);
void FUN_3005e2e0(void);
void FUN_3005e340(undefined4 param_1,float param_2,undefined4 param_3,float param_4,float param_5);
undefined4 FUN_3005e3b0(float param_1,float param_2,undefined4 param_3,int param_4);
int FUN_3005e4f0(int param_1,int param_2,int param_3);
void FUN_3005ebd0(int param_1,float param_2,int *param_3,undefined4 param_4,float param_5,int param_6);
void FUN_3005f290(int param_1,int param_2,int *param_3,float param_4,int param_5);
int FUN_3005f980(int param_1);
int FUN_3005fbb0(int param_1,int param_2,int param_3,float param_4,int param_5,int param_6);
undefined4 FUN_300604e0(void);
void FUN_30060650(void);
void FUN_30060830(void);
void FUN_300608f0(char *param_1);
void FUN_30060ca0(int param_1);
void FUN_30060de0(undefined4 param_1);
void FUN_30060f10(int param_1);
void FUN_300610d0(void);
void FUN_300610f0(void);
void FUN_30061570(undefined4 param_1);
void FUN_30061610(void);
void FUN_30061810(void);
void FUN_30061990(int param_1);
void FUN_30061ad0(char *param_1);
void FUN_30061c70(undefined4 param_1,int param_2,int param_3);
void FUN_30062150(void);
undefined4 FUN_300621d0(int param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4,int *param_5);
undefined * FUN_30062260(int param_1);
void FUN_30062280(int *param_1);
void FUN_300623e0(void);
void FUN_30062440(undefined4 *param_1);
void FUN_30062470(int param_1,undefined4 param_2,int param_3,undefined4 param_4,undefined4 param_5,undefined4 *param_6,undefined4 param_7);
void FUN_300626d0(int param_1);
void FUN_300627a0(void);
void FUN_300627d0(char *param_1);
void FUN_30062980(void);
void FUN_30062af0(void);
void FUN_30062b20(void);
void FUN_30062bb0(void);
void FUN_30063050(code *param_1);
void FUN_300635c0(int param_1,code *param_2);
void FUN_300637d0(int param_1,code *param_2);
void FUN_300639c0(void);
void FUN_30063b00(undefined4 param_1);
void FUN_30063b90(void);
void FUN_30063c70(void);
void FUN_30063d10(void);
void FUN_30063f00(void);
void FUN_30064120(void);
void FUN_30064350(void);
void FUN_300643d0(void);
void FUN_30066550(int param_1);
void FUN_30066590(void);
void FUN_30066670(undefined4 *param_1);
void FUN_300667e0(int param_1);
void FUN_30066980(void);
void FUN_30066b90(uint *param_1);
uint * FUN_30066ca0(void);
void FUN_30066d60(void);
uint FUN_30066ee0(void);
int FUN_30066f30(char *param_1);
undefined4 FUN_30066fe0(int param_1,undefined4 param_2,undefined4 param_3);
void FUN_300670c0(undefined4 param_1);
void FUN_30067110(void);
undefined4 FUN_30067210(char *param_1,undefined4 param_2,undefined4 param_3,int param_4);
void FUN_300672b0(undefined4 param_1);
void FUN_300676b0(void);
void FUN_300678f0(void);
void FUN_30067950(void);
void FUN_30067d30(undefined4 *param_1,undefined4 *param_2,float *param_3);
void FUN_30067ea0(void);
void FUN_30067ef0(void);
void FUN_30068aa0(int param_1);
void FUN_30068b60(int param_1);
void FUN_30068cf0(char *param_1,int param_2);
void FUN_30068d20(int param_1);
void FUN_30069260(int param_1);
undefined4 FUN_30069710(int param_1,int param_2);
undefined4 FUN_300697c0(int param_1,int param_2);
void FUN_30069860(int param_1);
void FUN_300698d0(int param_1);
void FUN_30069940(int param_1);
void FUN_300699d0(int param_1);
void FUN_30069bd0(void);
void FUN_3006a110(int param_1,int param_2);
void FUN_3006a380(int param_1,int param_2);
void FUN_3006a770(undefined4 param_1);
void FUN_3006a7a0(undefined4 param_1);
void FUN_3006a7c0(undefined4 param_1);
void FUN_3006a7e0(int param_1);
void FUN_3006a870(void);
undefined4 FUN_3006a980(byte *param_1,undefined4 param_2,undefined4 *param_3);
undefined4 FUN_3006aa10(undefined4 param_1,undefined4 param_2,float *param_3);
undefined4 FUN_3006aa40(undefined4 param_1,undefined4 param_2,undefined4 *param_3);
undefined4 FUN_3006aa70(undefined4 param_1,undefined4 param_2,int param_3);
undefined4 FUN_3006aab0(undefined4 param_1,undefined4 param_2,int param_3);
undefined * FUN_3006aaf0(void);
void FUN_3006ab40(void);
void FUN_3006abe0(void);
void FUN_3006ad30(void);
void FUN_3006af40(void);
void FUN_3006b100(void);
undefined * FUN_3006b170(char *param_1);
void FUN_3006b1e0(void);
void FUN_3006b330(void);
void FUN_3006b730(void);
void FUN_3006b790(void);
void dllEntry(undefined4 param_1);
void FUN_3006b800(undefined4 param_1);
void FUN_3006b820(undefined4 param_1);
void FUN_3006b840(void);
void FUN_3006b850(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3006b870(undefined4 param_1);
void FUN_3006b890(undefined4 param_1,undefined4 param_2);
void FUN_3006b8b0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006b8d0(void);
void FUN_3006b8e0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006b900(undefined4 param_1,undefined4 param_2);
void FUN_3006b920(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006b940(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006b960(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006b980(undefined4 param_1);
void FUN_3006b9a0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3006b9c0(undefined4 param_1);
void FUN_3006b9e0(undefined4 param_1);
void FUN_3006ba00(undefined4 param_1);
void FUN_3006ba20(undefined4 param_1);
void FUN_3006ba40(undefined4 param_1);
void FUN_3006ba60(void);
void FUN_3006ba70(void);
void FUN_3006ba80(undefined4 param_1);
void FUN_3006baa0(undefined4 param_1,undefined4 param_2);
void FUN_3006bac0(undefined4 param_1,undefined4 param_2);
void FUN_3006bae0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3006bb00(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
void FUN_3006bb30(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9);
void FUN_3006bb70(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
void FUN_3006bba0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9);
void FUN_3006bbe0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
void FUN_3006bc10(void);
void FUN_3006bc20(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3006bc50(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
void FUN_3006bc80(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
void FUN_3006bcb0(undefined4 param_1,undefined4 param_2);
void FUN_3006bcd0(void);
void FUN_3006bce0(undefined4 param_1);
void FUN_3006bd00(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
void FUN_3006bd30(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void FUN_3006bd60(undefined4 param_1);
void FUN_3006bd80(undefined4 param_1,undefined4 param_2);
void FUN_3006bda0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3006bdc0(undefined4 param_1);
void FUN_3006bde0(void);
void FUN_3006bdf0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006be10(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006be30(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006be50(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
void FUN_3006be80(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006bea0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006bec0(void);
void FUN_3006bed0(int param_1);
void FUN_3006bf30(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006bf50(undefined4 param_1);
void FUN_3006bf70(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3006bf90(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8);
void FUN_3006bfd0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
void FUN_3006c000(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
void FUN_3006c030(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void FUN_3006c060(undefined4 param_1);
void FUN_3006c080(void);
void FUN_3006c090(void);
void FUN_3006c0a0(undefined4 param_1);
void FUN_3006c0c0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9);
void FUN_3006c100(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,undefined4 param_10);
void FUN_3006c170(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006c190(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3006c1b0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006c1d0(undefined4 param_1);
void FUN_3006c1f0(undefined4 param_1);
void FUN_3006c210(undefined4 param_1,undefined4 param_2);
void FUN_3006c230(undefined4 param_1,undefined4 param_2);
void FUN_3006c250(undefined4 param_1);
void FUN_3006c270(void);
void FUN_3006c280(undefined4 param_1,undefined4 param_2);
void FUN_3006c2a0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3006c2c0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006c2e0(void);
void FUN_3006c2f0(void);
void FUN_3006c300(void);
void FUN_3006c330(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006c350(undefined4 param_1);
void FUN_3006c370(undefined4 param_1);
void FUN_3006c390(undefined4 param_1);
void FUN_3006c3b0(undefined4 param_1,undefined4 param_2);
void FUN_3006c3d0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006c400(undefined4 param_1);
void FUN_3006c420(undefined4 param_1);
void FUN_3006c440(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void FUN_3006c4b0(undefined4 param_1);
void FUN_3006c4d0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
void FUN_3006c500(undefined4 param_1,undefined4 param_2);
void FUN_3006c520(undefined4 param_1);
void FUN_3006c580(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3006c5a0(undefined4 param_1);
void FUN_3006c5c0(undefined4 param_1);
void FUN_3006c5e0(undefined4 param_1);
void FUN_3006c600(undefined4 param_1);
void FUN_3006c620(undefined4 param_1);
void FUN_3006c640(undefined4 param_1,undefined4 param_2,int param_3);
void FUN_3006c690(undefined4 param_1);
void FUN_3006c6b0(undefined4 param_1);
void FUN_3006c6d0(undefined4 param_1,undefined4 param_2);
void FUN_3006c6f0(undefined4 param_1,undefined4 param_2);
void FUN_3006c710(void);
void FUN_3006c7a0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
undefined4 FUN_3006c7e0(undefined4 param_1);
void FUN_3006c810(void);
int * FUN_3006c890(int param_1);
int FUN_3006c950(uint param_1,int param_2,undefined4 param_3,int param_4,int param_5,undefined4 *param_6,int param_7,float param_8,float param_9,undefined4 param_10,undefined4 param_11,undefined4 param_12,undefined4 *param_13,undefined4 *param_14,float param_15,float param_16);
int FUN_3006caf0(uint param_1,int param_2,undefined4 param_3,undefined4 *param_4,int param_5,float param_6,float param_7,undefined4 param_8,undefined4 param_9);
int FUN_3006cc10(uint param_1,int param_2,undefined4 param_3,undefined4 *param_4,int param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8);
void FUN_3006cd20(int param_1);
void FUN_3006cd70(int param_1,int param_2,int param_3);
void FUN_3006dea0(int *param_1);
void __fastcall FUN_3006df40(undefined4 param_1);
void FUN_3006e110(void);
void FUN_3006e1f0(void);
void FUN_3006e2b0(void);
void FUN_3006e420(undefined4 param_1,undefined4 param_2,int param_3);
void FUN_3006e490(void);
void FUN_3006e550(void);
void FUN_3006e8a0(void);
void FUN_3006e8f0(void);
void FUN_3006ec90(void);
void FUN_3006ed80(void);
void FUN_3006f4b0(float param_1,int param_2);
void FUN_3006f510(void);
void FUN_3006f560(void);
void FUN_3006f5a0(void);
bool FUN_3006f6c0(void);
void FUN_3006fac0(void);
void FUN_3006faf0(void);
void FUN_3006fda0(void);
char * FUN_30070400(undefined4 param_1,undefined4 param_2);
void FUN_30070430(void);
void FUN_300705f0(undefined4 param_1);
void FUN_300706a0(int param_1);
void FUN_30070950(void);
undefined4 FUN_30070be0(float *param_1);
undefined4 FUN_30070c30(float *param_1,float param_2);
void FUN_30070c80(void);
void FUN_30070ef0(int param_1,int param_2,int param_3);
void FUN_30071740(void);
void FUN_30071760(uint param_1);
void FUN_300717a0(void);
void FUN_300717d0(undefined4 param_1,float *param_2,float param_3);
void FUN_30071850(int param_1);
void FUN_30071bc0(int param_1);
void FUN_300720e0(int param_1);
void FUN_30072330(float *param_1);
void FUN_300723a0(undefined4 *param_1);
void FUN_30072770(int param_1);
void FUN_30072cd0(int param_1);
void FUN_30072e50(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3,int param_4,undefined4 param_5);
void FUN_30072f40(undefined4 param_1,float *param_2,float *param_3,int param_4,undefined4 param_5);
void __thiscall FUN_30073150(undefined4 param_1,int param_2);
void FUN_300733c0(undefined4 param_1);
void __fastcall FUN_30073450(int param_1,undefined4 param_2,int param_3);
void __fastcall FUN_30073770(undefined4 param_1);
void __fastcall FUN_300738a0(undefined4 param_1,int param_2,int param_3);
void __fastcall FUN_30073c20(undefined4 param_1);
void __fastcall FUN_30073ce0(undefined4 param_1,int param_2);
void FUN_30074e60(void);
void __fastcall FUN_30074f00(int param_1);
void FUN_30075080(uint param_1,int param_2);
void FUN_30075370(int param_1);
undefined4 FUN_30075400(int param_1,int param_2,int param_3,int param_4,int param_5);
void FUN_300754b0(int param_1);
void FUN_30075510(undefined4 param_1,undefined4 *param_2);
void __fastcall FUN_30075550(undefined4 *param_1,undefined4 param_2);
void FUN_300756e0(float *param_1);
void FUN_30075760(void);
void FUN_30075bd0(void);
void FUN_30075c00(void);
undefined4 FUN_30075c60(void);
bool FUN_30075ca0(int param_1);
undefined4 FUN_30075cf0(int param_1,int *param_2,int *param_3);
int __fastcall FUN_30075d90(int param_1);
void FUN_30075dd0(void);
undefined4 __fastcall FUN_30075e10(int param_1);
void FUN_30075e60(void);
int __thiscall FUN_30075e90(int param_1,int param_2,int param_3);
undefined4 __fastcall FUN_30075ee0(int param_1);
void FUN_30075f50(int param_1,int param_2);
void FUN_30076000(int param_1,int param_2);
void FUN_300760b0(int param_1,int param_2);
void FUN_300762e0(int param_1);
void FUN_300765e0(int param_1);
void FUN_30076ae0(void);
void FUN_30076cf0(int param_1);
void FUN_30077040(int *param_1);
void FUN_300771f0(int param_1);
void FUN_30077310(undefined4 param_1);
void FUN_30077450(int *param_1);
uint FUN_30077850(int *param_1,int param_2);
void FUN_30077c30(undefined4 *param_1,float *param_2,int param_3,int param_4,int param_5,float param_6);
void FUN_30077e70(float *param_1,float *param_2,int param_3,undefined4 param_4,int param_5,float param_6);
void FUN_30078020(undefined4 *param_1,float *param_2,int param_3,int param_4,int param_5,float param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,undefined4 param_10);
void FUN_30078180(undefined4 *param_1,float *param_2,int param_3,int param_4,int param_5);
void FUN_30078460(undefined4 param_1,undefined4 *param_2,undefined4 param_3,int param_4,int param_5);
void FUN_30078510(int param_1,int param_2,float *param_3,float *param_4,uint param_5);
void FUN_300799a0(undefined4 param_1,undefined4 param_2,float *param_3,float *param_4);
void FUN_30079ac0(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
void FUN_30079b20(int param_1,float *param_2,float *param_3);
void FUN_30079d50(float *param_1,float *param_2);
void FUN_30079f90(float *param_1,float *param_2,int param_3);
undefined4 FUN_3007a0d0(int param_1,float *param_2);
void FUN_3007a580(float *param_1,int param_2);
void FUN_3007a5d0(float *param_1,int param_2,undefined4 param_3,int param_4,int param_5,int param_6,float param_7);
void FUN_3007b3d0(int *param_1,int param_2,int *param_3);
void FUN_3007ce20(int param_1);
void FUN_3007d6a0(void);
void FUN_3007d6f0(undefined4 *param_1,int param_2,uint param_3);
undefined * FUN_3007d7e0(undefined4 param_1,undefined4 param_2);
void FUN_3007d840(void);
void FUN_3007d8b0(void);
void FUN_3007d9b0(int param_1);
void FUN_3007db40(void);
void FUN_3007dc50(int param_1);
void __fastcall FUN_3007dcc0(undefined4 param_1);
void FUN_3007dd30(undefined *param_1);
void FUN_3007ddf0(void);
void FUN_3007de40(void);
int __fastcall FUN_3007e2d0(undefined4 param_1,int param_2);
undefined ** FUN_3007e340(char *param_1,int param_2);
void FUN_3007e3b0(char *param_1);
void FUN_3007e450(undefined4 *param_1);
void __fastcall FUN_3007e480(int param_1,int param_2);
void __fastcall FUN_3007e4c0(int param_1,undefined4 *param_2);
void __fastcall FUN_3007e500(int param_1,int param_2);
void __thiscall FUN_3007e520(void *param_1,int param_2);
void FUN_3007e540(void);
void FUN_3007ebf0(int param_1,undefined4 param_2,uint param_3);
void FUN_3007ec90(undefined4 param_1,int param_2);
void FUN_3007ed20(void);
void FUN_3007ee40(void);
void FUN_3007eee0(undefined4 param_1);
void FUN_3007efa0(void);
undefined4 FUN_3007f260(void);
undefined4 FUN_3007f350(void);
void FUN_3007f3a0(undefined4 param_1);
void FUN_3007f4a0(void);
void FUN_3007f4d0(void);
undefined4 FUN_3007f590(char *param_1);
bool FUN_3007f5d0(char *param_1);
void FUN_3007f640(void);
void FUN_3007f760(void);
void FUN_3007f940(int *param_1);
float10 __thiscall FUN_3007f950(undefined4 param_1,undefined4 param_2);
float10 __thiscall FUN_3007f980(undefined4 param_1,undefined4 param_2);
void FUN_3007f9a0(uint param_1,undefined4 *param_2);
void FUN_3007fa00(float *param_1,float *param_2);
void FUN_3007fb20(undefined4 *param_1);
void FUN_3007fb50(undefined4 *param_1,undefined4 *param_2);
void FUN_3007fb90(float *param_1,float *param_2,float *param_3);
float10 FUN_3007fc30(float param_1);
float10 FUN_3007fc80(float param_1);
int FUN_3007fca0(float param_1);
float10 FUN_3007fcb0(float param_1,float param_2,float param_3);
void FUN_3007fd30(float param_1,float param_2);
void FUN_3007fdb0(undefined4 *param_1,undefined4 *param_2,float *param_3);
float10 FUN_3007fe10(void);
uint FUN_3007fe40(uint param_1);
float10 FUN_3007fe70(undefined4 param_1);
void FUN_3007feb0(float param_1,float param_2);
undefined4 FUN_3007fed0(float *param_1,float *param_2);
void FUN_3007ff10(float *param_1);
void FUN_3007ff80(float *param_1);
float10 FUN_3007ffe0(float *param_1,float *param_2);
void FUN_30080060(float *param_1,float *param_2,float *param_3);
float10 FUN_300800a0(void);
float10 FUN_300800d0(float *param_1);
void FUN_30080100(float *param_1,float *param_2);
float10 FUN_30080140(float *param_1,float *param_2);
void FUN_30080190(float *param_1);
void FUN_300801b0(float *param_1,float *param_2,float *param_3);
void FUN_300802a0(undefined4 param_1,float *param_2,float *param_3,float *param_4);
void FUN_30080410(undefined4 param_1,int param_2);
void FUN_30080490(float *param_1,float *param_2,float *param_3,undefined4 param_4);
void FUN_30080520(float *param_1,float *param_2,float *param_3,float *param_4);
void FUN_300805c0(float *param_1,float *param_2);
void FUN_30080600(float *param_1,float *param_2,float *param_3);
void FUN_300807f0(int param_1,float param_2);
void FUN_30080870(undefined4 param_1,int param_2);
void FUN_300808c0(int param_1,int param_2);
void FUN_30080940(int param_1,float *param_2);
char * FUN_30080a50(char *param_1);
void FUN_30080a80(char *param_1,char *param_2);
bool FUN_30080ab0(int param_1,int param_2);
void FUN_30080af0(int param_1,int param_2);
void FUN_30080b30(int param_1,int param_2);
void FUN_30080b70(undefined4 *param_1);
void FUN_30080b90(undefined4 *param_1);
undefined4 FUN_30080bb0(void);
void FUN_30080bc0(void);
undefined1 * FUN_30080c00(int *param_1);
bool FUN_30080dd0(int param_1);
void FUN_30080df0(char *param_1,char *param_2,int param_3);
int FUN_30080e40(char *param_1,char *param_2,int param_3);
int FUN_30080ea0(char *param_1,char *param_2,int param_3);
undefined4 FUN_30080ee0(int param_1,int param_2);
void FUN_30080f10(char *param_1);
void FUN_30080f40(char *param_1);
void FUN_30080f70(char *param_1,uint param_2,undefined4 param_3);
void FUN_30080fc0(char *param_1);
void FUN_30081010(char param_1);
char * FUN_300810d0(char *param_1);
void FUN_30081120(undefined4 param_1,undefined4 param_2,undefined4 param_3);
undefined * FUN_30081160(undefined4 param_1);
void FUN_300811f0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30081220(char *param_1,int param_2);
undefined4 FUN_30081350(undefined4 *param_1,char *param_2,char *param_3);
void FUN_300813d0(byte *param_1,char *param_2,uint param_3);
int FUN_300814e0(char *param_1);
undefined4 FUN_300815c0(undefined4 param_1);
void FUN_30081600(float *param_1,float param_2,float param_3);
undefined4 FUN_30081640(byte *param_1);
void FUN_30081670(byte *param_1);
void FUN_30081770(char *param_1);
void FUN_30081830(char *param_1,undefined4 param_2);
void FUN_30081870(undefined4 param_1);
void FUN_300818a0(undefined4 param_1);
void FUN_300818b0(undefined4 *param_1);
void FUN_300818f0(uint *param_1);
void FUN_30081de0(int param_1);
undefined4 FUN_30081eb0(int param_1);
void FUN_30081ee0(int param_1,undefined1 *param_2,int param_3);
void FUN_30081f50(int param_1);
void FUN_30082000(int param_1);
undefined * FUN_30082120(int param_1);
void FUN_30082180(void);
uint FUN_30082190(void);
undefined * FUN_300821d0(byte *param_1);
void FUN_300822d0(float *param_1,float *param_2,float *param_3,float param_4);
undefined4 FUN_300823f0(undefined4 param_1,float *param_2);
undefined4 FUN_30082420(undefined4 param_1,int param_2);
undefined4 FUN_30082470(undefined4 param_1,undefined4 *param_2);
undefined4 FUN_300824a0(undefined4 param_1,int param_2);
undefined4 FUN_30082500(undefined4 param_1,undefined4 *param_2);
void FUN_30082530(undefined4 param_1,undefined1 *param_2);
void FUN_300825a0(undefined4 param_1,undefined4 *param_2);
void FUN_30082700(undefined4 param_1);
void FUN_30082710(undefined4 *param_1,undefined4 param_2);
void FUN_30082770(void *param_1);
void FUN_300827b0(uint *param_1,float *param_2,float param_3,int *param_4,int param_5,int param_6,float param_7);
void FUN_30082840(float *param_1,undefined4 param_2,undefined4 param_3);
void FUN_30082d00(float *param_1,float param_2,float param_3);
void FUN_30082db0(int param_1);
int FUN_30082de0(int param_1,int param_2);
undefined4 FUN_30082ea0(int param_1,int param_2,int param_3);
void FUN_30082fa0(int param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083070(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083090(int param_1,undefined4 param_2,undefined4 param_3);
undefined4 FUN_300830d0(int param_1,int param_2);
void FUN_30083180(int param_1,undefined4 param_2,undefined4 param_3);
void FUN_300832a0(undefined4 param_1,undefined4 param_2,int param_3);
undefined * FUN_30083300(undefined4 param_1);
void FUN_30083350(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_300833a0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_300833f0(undefined4 param_1,undefined4 param_2,int param_3);
void FUN_30083450(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083490(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_300834d0(undefined4 param_1,undefined4 param_2,float param_3,float param_4,float param_5,float param_6,float param_7,float param_8,float param_9,float param_10,undefined4 param_11,float param_12);
void FUN_300835d0(int param_1,undefined4 param_2,undefined4 param_3);
void FUN_300836c0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7);
void FUN_30083730(int param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083810(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083850(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083890(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_300838c0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083a70(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083ac0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083b10(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083b60(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30083be0(undefined4 param_1);
void FUN_30083cb0(undefined4 param_1);
void FUN_30083ff0(int param_1,int *param_2,char *param_3);
void FUN_30084140(int param_1,uint param_2);
void FUN_300842e0(int param_1,int param_2);
uint FUN_300843f0(int param_1);
void FUN_30084450(int param_1);
void FUN_30084530(int param_1);
float10 FUN_30084640(float *param_1);
void FUN_30084710(int param_1);
void FUN_30084760(int param_1,int param_2);
void FUN_30084780(int param_1);
void FUN_30084890(void);
void FUN_300848c0(int param_1);
void FUN_30084920(void);
void FUN_30084950(int param_1);
undefined * __fastcall FUN_300849e0(int param_1);
void FUN_30084a60(float *param_1,float *param_2,float *param_3);
void FUN_30084a80(int param_1,int *param_2,int *param_3,undefined1 *param_4);
void FUN_30084c90(int param_1,undefined4 *param_2);
void FUN_30084eb0(int param_1);
void FUN_30085100(int param_1);
void FUN_300852a0(int param_1);
void FUN_300854c0(float *param_1);
void FUN_300857b0(float *param_1);
void FUN_30085aa0(int param_1);
void FUN_30085c80(int param_1);
void FUN_30085e20(void);
undefined * FUN_30085e60(undefined4 param_1);
void FUN_30085f30(float *param_1);
void FUN_300860f0(int param_1);
void FUN_30086360(float *param_1,float *param_2,float *param_3,float *param_4);
void FUN_300863e0(float *param_1);
void FUN_30086730(float *param_1);
void FUN_30087120(undefined4 *param_1);
void FUN_30087560(int param_1);
undefined * FUN_30087a10(void);
undefined * FUN_30087a50(undefined4 param_1,int param_2);
void FUN_30087b10(void *param_1);
void FUN_30087b50(undefined4 *param_1,int param_2);
void FUN_30087d60(int param_1);
undefined4 FUN_30087e40(int param_1);
uint FUN_30087e90(char *param_1);
void FUN_30087ee0(int param_1,undefined4 *param_2);
undefined4 * FUN_30087f00(int param_1,undefined4 param_2);
undefined4 FUN_30087f70(int param_1,undefined4 param_2);
undefined4 FUN_300880f0(int param_1,undefined4 param_2);
undefined4 FUN_30088140(int param_1,undefined4 param_2);
bool FUN_300881a0(int param_1,undefined4 param_2);
bool FUN_300881d0(int param_1,undefined4 param_2);
bool FUN_30088200(int param_1,undefined4 param_2);
undefined4 FUN_30088370(int param_1);
bool FUN_30088460(int param_1,undefined4 param_2);
bool FUN_30088490(int param_1,undefined4 param_2);
undefined4 FUN_300884d0(int param_1,undefined4 param_2);
undefined4 FUN_300885c0(int param_1,undefined4 param_2);
bool FUN_300887c0(int param_1,undefined4 param_2);
bool FUN_30088840(int param_1,undefined4 param_2);
undefined4 FUN_300889c0(int param_1,undefined4 param_2);
undefined4 FUN_30088e50(int param_1,undefined4 param_2,int param_3);
undefined4 FUN_30089110(int param_1,undefined4 param_2);
undefined4 FUN_30089150(int param_1,undefined4 param_2);
undefined4 FUN_30089190(int param_1,undefined4 param_2);
void FUN_300891f0(void);
void FUN_30089240(undefined4 param_1,undefined4 param_2);
void FUN_30089350(int param_1);
bool FUN_30089490(int param_1,undefined4 param_2);
undefined4 FUN_30089650(int param_1,undefined4 param_2);
undefined4 FUN_300896a0(int param_1,undefined4 param_2);
undefined4 FUN_300897a0(int param_1,undefined4 param_2);
void FUN_300899a0(void);
void FUN_300899f0(void);
void FUN_30089b60(void);
undefined4 FUN_30089b70(undefined4 param_1,int param_2);
undefined4 FUN_30089bd0(float param_1,float param_2,float param_3,float param_4,float param_5,float param_6);
void FUN_30089c20(undefined4 *param_1);
void FUN_30089c70(undefined4 *param_1);
undefined4 FUN_30089ca0(int *param_1);
void FUN_30089ce0(int *param_1);
void FUN_30089d10(float param_1,undefined4 param_2);
void FUN_30089eb0(int param_1);
void FUN_30089ed0(int param_1);
undefined4 FUN_30089fb0(void);
void FUN_30089fc0(undefined4 param_1);
void FUN_3008a0d0(void);
int FUN_3008a130(int param_1);
void FUN_3008a1b0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3008a2f0(undefined4 param_1,undefined4 param_2,int param_3);
void FUN_3008a380(undefined4 param_1);
void __fastcall FUN_3008a390(int param_1);
void FUN_3008a3d0(undefined4 param_1);
void FUN_3008a490(void);
void FUN_3008a4c0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3008a510(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3008a560(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3008a680(int param_1,undefined4 param_2,undefined4 param_3);
void FUN_3008b0a0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_3008b130(int param_1,undefined4 param_2,undefined4 param_3);
void FUN_3008b1e0(int param_1,undefined4 param_2,undefined4 param_3);
void FUN_3008b230(int param_1,undefined4 param_2,undefined4 param_3);
void FUN_3008b340(int param_1);
void FUN_3008b5d0(int param_1,uint param_2);
undefined4 FUN_3008b7c0(undefined4 param_1,int param_2,int *param_3);
undefined4 FUN_3008b8c0(int param_1);
void FUN_3008b950(void);
undefined4 FUN_3008b980(void);
undefined4 FUN_3008b9e0(void);
void FUN_3008baf0(undefined4 *param_1);
void thunk_FUN_3008f880(int param_1);
void FUN_3008bb70(int param_1,undefined1 param_2);
void FUN_3008bbe0(int *param_1);
void __fastcall FUN_3008bd00(int param_1);
undefined4 FUN_3008bd50(int param_1,int param_2);
undefined4 FUN_3008bfa0(int param_1);
void FUN_3008c150(int param_1,undefined4 *param_2);
void FUN_3008c360(int *param_1,int param_2);
void FUN_3008c410(int param_1,char param_2);
void FUN_3008c5f0(int param_1,undefined4 *param_2,int *param_3,int param_4,int param_5);
void FUN_3008c7b0(int param_1,undefined4 *param_2,int *param_3,int param_4,int param_5);
void FUN_3008c9e0(int *param_1);
void FUN_3008cbc0(uint *param_1);
void FUN_3008d450(int *param_1);
void FUN_3008d4c0(int param_1,int param_2,int *param_3);
undefined1 FUN_3008d5e0(void);
undefined4 __thiscall FUN_3008d610(int *param_1,uint param_2);
undefined4 FUN_3008d6c0(void);
undefined4 __thiscall FUN_3008d6f0(int param_1,undefined4 param_2,short *param_3,int *param_4,undefined4 *param_5);
uint FUN_3008d850(char param_1);
undefined1 FUN_3008d8d0(int *param_1,int param_2);
void FUN_3008da30(int *param_1);
int * __fastcall FUN_3008daf0(undefined4 param_1,int param_2,int *param_3,int param_4);
undefined4 FUN_3008db90(int param_1,int param_2);
void FUN_3008dc50(int *param_1,undefined4 *param_2,int param_3);
void FUN_3008df20(int *param_1);
void FUN_3008e1a0(int param_1);
void FUN_3008e200(int *param_1);
void FUN_3008e3b0(int *param_1,char param_2);
void FUN_3008e440(undefined1 param_1);
void FUN_3008e480(undefined4 param_1);
void FUN_3008e4a0(void);
int __fastcall FUN_3008e4d0(int *param_1);
void __thiscall FUN_3008e5b0(int *param_1,int param_2,char param_3);
void FUN_3008e6b0(void);
void __fastcall FUN_3008e6e0(int *param_1);
void FUN_3008e790(void);
void FUN_3008e860(void);
void FUN_3008e8f0(void);
void FUN_3008e980(undefined4 param_1,undefined4 param_2,undefined1 *param_3,uint param_4);
void FUN_3008e9d0(int param_1);
void FUN_3008eb00(int param_1);
void FUN_3008ebc0(int param_1);
void FUN_3008ec50(int param_1);
void FUN_3008ecc0(void);
void FUN_3008eea0(void);
void FUN_3008f2a0(void);
uint FUN_3008f390(void);
void FUN_3008f560(int *param_1);
void FUN_3008f700(int param_1);
void FUN_3008f7a0(int param_1,char param_2);
void FUN_3008f850(int param_1);
void FUN_3008f880(int param_1);
void FUN_3008f8b0(int param_1);
void FUN_3008f8d0(int param_1);
void FUN_3008f8f0(int *param_1,int param_2,int param_3,int param_4,char param_5);
void FUN_3008faa0(undefined4 param_1,undefined4 param_2,undefined4 param_3);
int FUN_3008fae0(int param_1);
void FUN_3008fb20(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void __thiscall FUN_3008fb50(undefined4 *param_1,undefined4 *param_2);
void FUN_3008fba0(void);
void FUN_3008fc00(int *param_1,int param_2);
void FUN_3008fef0(int *param_1);
void FUN_3008ff70(int *param_1);
void FUN_30090070(void);
void __thiscall FUN_300900b0(int param_1,uint param_2);
void FUN_30090170(void);
int __fastcall FUN_30090190(int param_1);
void __fastcall FUN_300901c0(char *param_1);
void FUN_300901f0(void);
void FUN_30090250(char param_1);
undefined4 FUN_30090400(int param_1,int *param_2);
void FUN_300908a0(int param_1);
void FUN_30090970(int *param_1,char param_2);
void FUN_30090b40(int param_1);
void FUN_30090bd0(void);
void FUN_30090c00(int param_1,int param_2,uint *param_3,uint param_4,undefined4 param_5,uint *param_6,uint param_7);
void FUN_30090d90(int param_1,int param_2,uint *param_3,uint param_4,undefined4 param_5,uint *param_6,uint param_7);
void FUN_30090fc0(void);
void FUN_300910e0(int *param_1,char param_2);
void __thiscall FUN_30091190(int param_1,int param_2,int param_3);
void FUN_300911e0(int param_1,int *param_2,int param_3,int param_4,int param_5);
void FUN_30091270(int *param_1,int param_2,int *param_3,int param_4);
void FUN_30091430(int param_1,int param_2,undefined4 *param_3,int param_4);
void FUN_300915a0(int param_1,int param_2,int param_3,int param_4);
void FUN_30091820(int param_1,int param_2,undefined4 *param_3,int param_4);
void FUN_300919a0(int *param_1);
void FUN_30091bc0(int *param_1,int *param_2,int *param_3);
void FUN_30091e60(undefined4 *param_1);
void FUN_300921e0(undefined4 param_1);
int FUN_30092200(int *param_1,int param_2,uint param_3);
undefined4 * FUN_30092330(int *param_1,int param_2,uint param_3);
int FUN_300923d0(int *param_1,undefined4 param_2,uint param_3,uint param_4);
int FUN_30092480(int *param_1,undefined4 param_2,int param_3,uint param_4);
void FUN_30092530(int *param_1,int param_2,undefined1 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void FUN_300925a0(int *param_1,int param_2,undefined1 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void FUN_30092610(int param_1);
void FUN_300927b0(undefined4 param_1,char param_2);
void FUN_30092850(undefined4 param_1,char param_2);
void FUN_30092b80(int *param_1,int param_2);
void FUN_30092cd0(int *param_1);
void FUN_30092df0(undefined4 param_1,size_t param_2);
void FUN_30092e00(undefined4 param_1,void *param_2);
undefined4 FUN_30092e10(undefined4 param_1,undefined4 param_2,undefined4 param_3);
void FUN_30092e20(int *param_1);
int FUN_30092e40(int param_1,int param_2);
int FUN_30092e50(int param_1,int param_2);
void FUN_30092e70(int param_1,int param_2,int param_3,int param_4,int param_5,size_t param_6);
void FUN_30092ec0(int *param_1,char param_2);
void FUN_30092f40(int *param_1,undefined4 param_2,uint param_3);
void FUN_30093010(void *param_1,size_t param_2);
void FUN_30093030(int param_1,uint param_2,int param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
undefined1 * FUN_30093150(void);
void FUN_300931a0(void);
void FUN_30093220(void);
void FUN_30093240(undefined4 param_1);
char * FUN_300932f0(uint param_1);
void FUN_30093510(int *param_1);
void FUN_30093550(void);
void FUN_30093580(int param_1);
void FUN_300937f0(void);
void FUN_30093900(void);
void thunk_FUN_300937f0(void);
void thunk_FUN_30093900(void);
int __WSAFDIsSet(SOCKET param_1,fd_set *param_2);
BOOL GetModuleInformation(HANDLE hProcess,HMODULE hModule,LPMODULEINFO lpmodinfo,DWORD cb);
DWORD GetModuleFileNameExA(HANDLE hProcess,HMODULE hModule,LPSTR lpFilename,DWORD nSize);
BOOL EnumProcessModules(HANDLE hProcess,HMODULE *lphModule,DWORD cb,LPDWORD lpcbNeeded);
void __fastcall __security_check_cookie(int param_1);
void * __cdecl _memset(void *_Dst,int _Val,size_t _Size);
uint * FUN_30093a10(uint *param_1,char *param_2);
long __cdecl _atol(char *_Str);
void FUN_30093aac(char *param_1);
int __cdecl _rand(void);
void FUN_30093ad9(void);
void __cfltcvt_init(void);
void __cdecl __fpmath(int param_1);
ulonglong __fastcall FUN_30093b60(undefined4 param_1,undefined4 param_2);
_LocaleUpdate * __thiscall _LocaleUpdate::_LocaleUpdate(_LocaleUpdate *this,localeinfo_struct *param_1);
int __cdecl __tolower_l(int _C,_locale_t _Locale);
int __cdecl _tolower(int _C);
void FUN_30093de0(void);
uint FUN_30093dfd(uint param_1,uint param_2);
float10 FUN_30093ea0(double param_1,undefined2 param_2);
void FUN_30094010(void);
void FUN_3009405f(void);
ushort FUN_30094068(int param_1);
int __cdecl __vsnprintf_l(char *_DstBuf,size_t _MaxCount,char *_Format,_locale_t _Locale,va_list _ArgList);
int __cdecl __vsnprintf(char *_Dest,size_t _Count,char *_Format,va_list _Args);
void * __cdecl _memcpy(void *_Dst,void *_Src,size_t _Size);
void FUN_30094580(void);
void FUN_300945cf(void);
ushort FUN_300945d8(int param_1);
double __cdecl __atof_l(char *_String,_locale_t _Locale);
double __cdecl _atof(char *_String);
void __alloca_probe(void);
uint * FUN_30094770(uint *param_1,char param_2);
void __cdecl swap(void);
void __cdecl shortsort(undefined1 *param_1,int param_2,code *param_3);
void __cdecl _qsort(void *_Base,size_t _NumOfElements,size_t _SizeOfElements,_PtFuncCompare *_PtFuncCompare);
void FUN_30094b4a(void);
char * __cdecl _strncpy(char *_Dest,char *_Source,size_t _Count);
void FUN_30094cd0(void);
void FUN_30094d2d(undefined4 param_1,undefined4 param_2,int param_3);
undefined4 __cdecl vscan_fn(code *param_1,int param_2,undefined4 param_3,undefined4 param_4);
int __cdecl FID_conflict:_sscanf(char *_Src,char *_Format,...);
void FUN_30094ff0(void);
void FUN_3009503f(void);
ushort FUN_30095048(int param_1);
float10 FUN_300950e0(double param_1,undefined2 param_2);
int __cdecl __get_printf_count_output(void);
char * __cdecl _strtok(char *_Str,char *_Delim);
int __cdecl _sprintf(char *_Dest,char *_Format,...);
void __cdecl _free(void *_Memory);
void FUN_300953a7(void);
size_t __cdecl __fread_nolock_s(void *_DstBuf,size_t _DstSize,size_t _ElementSize,size_t _Count,FILE *_File);
size_t __cdecl _fread_s(void *_DstBuf,size_t _DstSize,size_t _ElementSize,size_t _Count,FILE *_File);
void FUN_30095675(void);
size_t __cdecl _fread(void *_DstBuf,size_t _ElementSize,size_t _Count,FILE *_File);
int __cdecl __fclose_nolock(FILE *_File);
int __cdecl _fclose(FILE *_File);
void FUN_30095787(void);
undefined4 _V6_HeapAlloc(uint param_1);
void FUN_300957d5(void);
void * __cdecl _malloc(size_t _Size);
void __cdecl _rewind(FILE *_File);
void FUN_30095964(void);
long __cdecl __ftell_nolock(FILE *_File);
long __cdecl _ftell(FILE *_File);
void FUN_30095b6f(void);
int __cdecl __fseek_nolock(FILE *_File,long _Offset,int _Origin);
int __cdecl _fseek(FILE *_File,long _Offset,int _Origin);
void FUN_30095c7e(void);
FILE * __cdecl __fsopen(char *_Filename,char *_Mode,int _ShFlag);
void FUN_30095d42(void);
FILE * __cdecl _fopen(char *_Filename,char *_Mode);
int __cdecl __toupper_l(int _C,_locale_t _Locale);
int __cdecl _toupper(int _C);
char * __cdecl _strrchr(char *_Str,int _Ch);
void * __cdecl _memchr(void *_Buf,int _Val,size_t _MaxCount);
clock_t __cdecl _clock(void);
undefined4 ___inittime(void);
int __cdecl __flush(FILE *_File);
int __cdecl __fflush_nolock(FILE *_File);
int __cdecl flsall(int param_1);
void FUN_3009615d(void);
void FUN_3009618c(void);
int __cdecl _fflush(FILE *_File);
void FUN_300961de(void);
void FUN_300961e8(void);
undefined ** FUN_300961f1(void);
void __cdecl __lock_file(FILE *_File);
void __cdecl __lock_file2(int _Index,void *_File);
void __cdecl __unlock_file(FILE *_File);
void __cdecl __unlock_file2(int _Index,void *_File);
char * __cdecl _strncat(char *_Dest,char *_Source,size_t _Count);
errno_t __cdecl __localtime64_s(tm *_Tm,__time64_t *_Time);
tm * __cdecl __localtime64(__time64_t *_Time);
__time64_t __cdecl __time64(__time64_t *_Time);
void __cdecl __freea(void *_Memory);
void __cdecl _store_str(char *param_1,char **param_2,uint *param_3);
void __cdecl _store_num(int param_1,int param_2,char **param_3,uint *param_4,uint param_5);
int __cdecl _expandtime(localeinfo_struct *param_1,char param_2,tm *param_3,char **param_4,uint *param_5,__lc_time_data *param_6,uint param_7);
int __cdecl _store_winword(localeinfo_struct *param_1,int param_2,tm *param_3,char **param_4,uint *param_5,__lc_time_data *param_6);
int __Strftime_l(char *param_1,uint param_2,char *param_3,int param_4,tm *param_5,localeinfo_struct *param_6);
size_t __cdecl _strftime(char *_Buf,size_t _SizeInBytes,char *_Format,tm *_Tm);
void * __cdecl _memmove(void *_Dst,void *_Src,size_t _Size);
void * __cdecl _calloc(size_t _Count,size_t _Size);
ulong __cdecl strtoxl(localeinfo_struct *param_1,char *param_2,char **param_3,int param_4,int param_5);
long __cdecl _strtol(char *_Str,char **_EndPtr,int _Radix);
char * __cdecl __getenv_helper_nolock(char *param_1);
char * __cdecl _getenv(char *_VarName);
void FUN_300979e0(void);
undefined4 __CRT_INIT@12(undefined4 param_1,int param_2,int param_3);
int __fastcall ___DllMainCRTStartup(undefined4 param_1,int param_2,undefined4 param_3);
void entry(undefined4 param_1,int param_2);
void __cdecl ___report_gsfailure(void);
void __cdecl fastzero_I(undefined1 (*param_1) [16],uint param_2);
undefined1 * __VEC_memzero(undefined1 *param_1,undefined4 param_2,uint param_3);
undefined4 FUN_30097e23(void);
undefined4 __get_sse2_info(void);
int __encode_pointer(int param_1);
void __encoded_null(void);
int __decode_pointer(int param_1);
LPVOID ___set_flsgetvalue(void);
void __cdecl __mtterm(void);
void __cdecl __initptd(_ptiddata _Ptd,pthreadlocinfo _Locale);
void FUN_3009811e(void);
void FUN_30098127(void);
_ptiddata __cdecl __getptd_noexit(void);
_ptiddata __cdecl __getptd(void);
void __freefls@4(void *param_1);
void FUN_300982dd(void);
void FUN_300982e9(void);
void __cdecl __freeptd(_ptiddata _Ptd);
int __cdecl __mtinit(void);
void __cdecl __forcdecpt_l(char *_Buf,_locale_t _Locale);
void __cdecl __cropzeros_l(char *_Buf,_locale_t _Locale);
int __cdecl __positive(double *arg);
void __cdecl __fassign_l(int flag,char *argument,char *number,_locale_t param_4);
void __cdecl __fassign(int flag,char *argument,char *number);
void __shift(void);
void __cdecl __forcdecpt(char *_Buf);
void __cdecl __cropzeros(char *_Buf);
int __cftoe2_l(uint param_1,int param_2,int param_3,int *param_4,char param_5,localeinfo_struct *param_6);
void __cftoe_l(double *param_1,undefined1 *param_2,int param_3,int param_4,undefined4 param_5,undefined4 param_6);
errno_t __cdecl __cftoe(double *_Value,char *_Buf,size_t _SizeInBytes,int _Dec,int _Caps);
int __cftoa_l(double *param_1,undefined1 *param_2,uint param_3,size_t param_4,int param_5,localeinfo_struct *param_6);
undefined4 __thiscall __cftof2_l(undefined1 *param_1,int param_2,size_t param_3,char param_4,localeinfo_struct *param_5);
void __cftof_l(double *param_1,undefined1 *param_2,int param_3,int param_4,undefined4 param_5);
void __cftog_l(double *param_1,undefined1 *param_2,int param_3,int param_4,undefined4 param_5,undefined4 param_6);
errno_t __cdecl __cfltcvt_l(double *arg,char *buffer,size_t sizeInBytes,int format,int precision,int caps,_locale_t plocinfo);
errno_t __cdecl __cfltcvt(double *arg,char *buffer,size_t sizeInBytes,int format,int precision,int caps);
void __initp_misc_cfltcvt_tab(void);
void __setdefaultprecision(void);
undefined4 __ms_p5_test_fdiv(void);
void __ms_p5_mp_test_fdiv(void);
int __cdecl CPtoLCID(int param_1);
void __cdecl setSBCS(threadmbcinfostruct *param_1);
void __cdecl setSBUpLow(threadmbcinfostruct *param_1);
pthreadmbcinfo __cdecl ___updatetmbcinfo(void);
void FUN_3009933c(void);
int __cdecl getSystemCP(int param_1);
void __setmbcp_nolock(undefined4 param_1,int param_2);
int __cdecl __setmbcp(int _CodePage);
void FUN_30099707(void);
undefined4 ___initmbctable(void);
void ___freetlocinfo(void *param_1);
void ___addlocaleref(LONG *param_1);
LONG * ___removelocaleref(LONG *param_1);
int * __updatetlocinfoEx_nolock(void);
pthreadlocinfo __cdecl ___updatetlocinfo(void);
void FUN_30099a77(void);
int __cdecl __crtLCMapStringA_stat(localeinfo_struct *param_1,ulong param_2,ulong param_3,char *param_4,int param_5,char *param_6,int param_7,int param_8,int param_9);
int __cdecl ___crtLCMapStringA(_locale_t _Plocinfo,LPCWSTR _LocaleName,DWORD _DwMapFlag,LPCSTR _LpSrcStr,int _CchSrc,LPSTR _LpDestStr,int _CchDest,int _Code_page,BOOL _BError);
int __cdecl __get_errno_from_oserr(ulong param_1);
int * __cdecl __errno(void);
ulong * __cdecl ___doserrno(void);
void __cdecl __dosmaperr(ulong param_1);
int __cdecl __isleadbyte_l(int _C,_locale_t _Locale);
int __cdecl _isleadbyte(int _C);
int __cdecl __isctype_l(int _C,int _Type,_locale_t _Locale);
void __fastcall __trandisp1(undefined4 param_1,int param_2);
void __fastcall __trandisp2(undefined4 param_1,int param_2);
void FUN_3009a1c3(void);
float10 __fastcall FUN_3009a1d0(undefined4 param_1,undefined4 param_2,undefined2 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8);
float10 __fastcall __startOneArgErrorHandling(undefined4 param_1,undefined4 param_2,undefined2 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
void FUN_3009a245(void);
undefined4 FUN_3009a25c(void);
uint __fastcall __fload_withFB(undefined4 param_1,int param_2);
uint FUN_3009a2b8(undefined4 param_1,uint param_2);
void __math_exit(void);
void ___libm_error_support(double *param_1,undefined8 *param_2,double *param_3,int param_4);
void FUN_3009a760(void);
float10 FUN_3009a77e(void);
int __cdecl __flsbuf(int _Ch,FILE *_File);
void __cdecl write_char(void);
void __cdecl write_multi_char(undefined4 param_1,int param_2);
void __cdecl write_string(int param_1);
int __cdecl __output_l(FILE *_File,char *_Format,_locale_t _Locale,va_list _ArgList);
void FUN_3009b6ae(undefined4 param_1);
void __cdecl __invoke_watson(wchar_t *param_1,wchar_t *param_2,wchar_t *param_3,uint param_4,uintptr_t param_5);
void __invalid_parameter(wchar_t *param_1,wchar_t *param_2,wchar_t *param_3,uint param_4,uintptr_t param_5);
void FUN_3009b80b(undefined4 *param_1,undefined4 *param_2,uint param_3);
undefined4 * __VEC_memcpy(undefined4 *param_1,undefined4 *param_2,uint param_3);
void FUN_3009b980(void);
float10 FUN_3009b99e(void);
size_t __cdecl _strlen(char *_Str);
FLT __cdecl __fltin2(FLT _Flt,char *_Str,_locale_t _Locale);
uint __cdecl ___strgtold12_l(_LDBL12 *pld12,char **p_end_ptr,char *str,int mult12,int scale,int decpt,int implicit_E,_locale_t _Locale);
unkbyte10 FUN_3009c3ec(void);
void __cintrindisp2(void);
void __cintrindisp1(void);
void __ctrandisp2(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
void FUN_3009c4f3(void);
void FUN_3009c4fa(void);
void __ctrandisp1(undefined4 param_1,undefined4 param_2);
float10 __fload(uint param_1,int param_2);
void FUN_3009c6b0(void);
float10 FUN_3009c6c9(double param_1,int param_2,uint param_3);
undefined4 __d_inttype(double param_1);
undefined4 FUN_3009d47d(int param_1,int param_2,int param_3,int param_4,double *param_5);
undefined4 ___check_float_string(size_t param_1,void *param_2,undefined4 *param_3);
uint __hextodec(byte param_1);
uint __fastcall __inc(undefined4 param_1,FILE *param_2);
void __un_inc(int param_1,FILE *param_2);
uint __whiteout(void);
int __cdecl __input_l(FILE *_File,uchar *param_2,_locale_t _Locale,va_list _ArgList);
void FUN_3009e650(void);
float10 FUN_3009e66e(void);
void __SEH_prolog4(undefined4 param_1,int param_2);
void __SEH_epilog4(void);
undefined4 __except_handler4(int *param_1,int param_2,undefined4 param_3);
int __cdecl __heap_init(void);
void __cdecl __heap_term(void);
int __cdecl __mtinitlocks(void);
void __cdecl __mtdeletelocks(void);
void FUN_3009ecc2(int param_1);
int __cdecl __mtinitlocknum(int _LockNum);
void FUN_3009ed93(void);
void __cdecl __lock(int _File);
uint ___sbh_find_block(int param_1);
void ___sbh_free_block(uint *param_1,int param_2);
undefined4 * ___sbh_alloc_new_region(void);
int ___sbh_alloc_new_group(int param_1);
undefined4 ___sbh_resize_block(uint *param_1,int param_2,int param_3);
int * ___sbh_alloc_block(uint *param_1);
int __cdecl __filbuf(FILE *_File);
int __cdecl __read_nolock(int _FileHandle,void *_DstBuf,uint _MaxCharCount);
int __cdecl __read(int _FileHandle,void *_DstBuf,uint _MaxCharCount);
void FUN_300a0073(void);
int __cdecl __fileno(FILE *_File);
errno_t __cdecl _memcpy_s(void *_Dst,rsize_t _DstSize,void *_Src,rsize_t _MaxCount);
int __cdecl __close_nolock(int _FileHandle);
int __cdecl __close(int _FileHandle);
void FUN_300a028b(void);
void __cdecl __freebuf(FILE *_File);
void __crt_waiting_on_module_handle(LPCWSTR param_1);
void __cdecl __amsg_exit(int param_1);
void __cdecl ___crtCorExitProcess(int param_1);
void __cdecl ___crtExitProcess(int param_1);
void FUN_300a0362(void);
void FUN_300a036b(void);
void __initterm(undefined4 *param_1);
void __initterm_e(undefined4 *param_1,undefined4 *param_2);
int __cdecl __cinit(int param_1);
void __cdecl doexit(int param_1,int param_2,int param_3);
void FUN_300a0551(void);
void __cdecl __exit(int _Code);
void __cdecl __cexit(void);
void __cdecl __init_pointers(void);
void __cdecl __NMSG_WRITE(int param_1);
void __cdecl __FF_MSGBANNER(void);
void FUN_300a07bd(undefined4 param_1);
int __cdecl __callnewh(size_t _Size);
long __cdecl __lseek_nolock(int _FileHandle,long _Offset,int _Origin);
long __cdecl __lseek(int _FileHandle,long _Offset,int _Origin);
void FUN_300a093b(void);
int __cdecl __ioinit(void);
void __cdecl __ioterm(void);
FILE * __cdecl __openfile(char *_Filename,char *_Mode,int _ShFlag,FILE *_File);
FILE * __cdecl __getstream(void);
void FUN_300a0fdf(void);
void __local_unwind4(uint *param_1,int param_2,uint param_3);
void FUN_300a10be(int param_1);
void __fastcall _EH4_CallFilterFunc(code *param_1);
void __fastcall _EH4_TransferToHandler(code *UNRECOVERED_JUMPTABLE);
void __fastcall _EH4_GlobalUnwind(PVOID param_1);
void __fastcall _EH4_LocalUnwind(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4);
undefined8 __aulldiv(uint param_1,uint param_2,uint param_3,uint param_4);
longlong __allmul(uint param_1,int param_2,uint param_3,int param_4);
int __cdecl __write_nolock(int _FileHandle,void *_Buf,uint _MaxCharCount);
int __cdecl __write(int _FileHandle,void *_Buf,uint _MaxCharCount);
void FUN_300a19e9(void);
int __cdecl __commit(int _FileHandle);
void FUN_300a1aca(void);
void * __cdecl __malloc_crt(size_t _Size);
void * __cdecl __calloc_crt(size_t _Count,size_t _Size);
void * __cdecl __realloc_crt(void *_Ptr,size_t _NewSize);
void * __cdecl __recalloc_crt(void *_Ptr,size_t _Count,size_t _Size);
int FUN_300a1c05(void);
void FUN_300a1c9b(void);
void __cdecl FID_conflict:__set_dstbias(long _Value);
void __cdecl FID_conflict:__set_dstbias(long _Value);
void __cdecl FID_conflict:__set_dstbias(long _Value);
void __tzset_nolock(void);
void FUN_300a1f83(void);
int __cdecl cvtdate(int param_1,int param_2,uint param_3,int param_4,int param_5,int param_6,int param_7,int param_8,int param_9);
bool __isindst_nolock(void);
void __cdecl ___tzset(void);
void FUN_300a2432(void);
int __cdecl __isindst(tm *_Time);
void FUN_300a2473(void);
errno_t __cdecl __gmtime64_s(tm *_Tm,__time64_t *_Time);
errno_t __cdecl __get_daylight(int *_Daylight);
errno_t __cdecl __get_dstbias(long *_Daylight_savings_bias);
errno_t __cdecl __get_timezone(long *_Timezone);
undefined4 * FUN_300a276a(void);
undefined4 * FUN_300a2770(void);
undefined4 * FUN_300a2776(void);
undefined ** FUN_300a277c(void);
undefined8 __alldiv(uint param_1,uint param_2,uint param_3,uint param_4);
undefined8 __allrem(uint param_1,uint param_2,uint param_3,uint param_4);
tm * __cdecl ___getgmtimebuf(void);
errno_t __cdecl _strcpy_s(char *_Dst,rsize_t _SizeInBytes,char *_Src);
int __cdecl ___ascii_stricmp(char *_Str1,char *_Str2);
uint __alloca_probe_16(void);
uint __alloca_probe_8(void);
void * __calloc_impl(uint param_1,uint param_2,undefined4 *param_3);
void FUN_300a2af8(void);
int __cdecl __mbsnbicoll_l(uchar *_Str1,uchar *_Str2,size_t _MaxCount,_locale_t _Locale);
int __cdecl __mbsnbicoll(uchar *_Str1,uchar *_Str2,size_t _MaxCount);
int __cdecl ___wtomb_environ(void);
size_t __cdecl _strnlen(char *_Str,size_t _MaxCount);
int __cdecl __setenvp(void);
void __cdecl parse_cmdline(undefined4 *param_1,byte *param_2,int *param_3);
int __cdecl __setargv(void);
LPVOID __cdecl ___crtGetEnvironmentStringsA(void);
void __RTC_Initialize(void);
int __cdecl __XcptFilter(ulong _ExceptionNum,_EXCEPTION_POINTERS *_ExceptionPtr);
int __cdecl ___CppXcptFilter(ulong _ExceptionNum,_EXCEPTION_POINTERS *_ExceptionPtr);
undefined4 FUN_300a330c(void);
void __cdecl ___security_init_cookie(void);
void FUN_300a33a8(void);
undefined8 __aulldvrm(uint param_1,uint param_2,uint param_3,uint param_4);
int __cdecl __isdigit_l(int _C,_locale_t _Locale);
int __cdecl _isdigit(int _C);
int __cdecl __isxdigit_l(int _C,_locale_t _Locale);
int __cdecl _isxdigit(int _C);
int __cdecl __isspace_l(int _C,_locale_t _Locale);
int __cdecl _isspace(int _C);
int __cdecl FID_conflict:__atoflt_l(_CRT_FLOAT *_Result,char *_Str,_locale_t _Locale);
int __cdecl FID_conflict:__atoflt_l(_CRT_FLOAT *_Result,char *_Str,_locale_t _Locale);
errno_t __cdecl __fptostr(char *_Buf,size_t _SizeInBytes,int _Digits,STRFLT _PtFlt);
void ___dtold(uint *param_1,uint *param_2);
STRFLT __cdecl __fltout2(_CRT_DOUBLE _Dbl,STRFLT _Flt,char *_ResultStr,size_t _SizeInBytes);
undefined8 __alldvrm(uint param_1,uint param_2,uint param_3,uint param_4);
ulonglong __fastcall __aullshr(byte param_1,uint param_2);
errno_t __cdecl __controlfp_s(uint *_CurrentState,uint _NewValue,uint _Mask);
int __cdecl __crtGetStringTypeA_stat(localeinfo_struct *param_1,ulong param_2,char *param_3,int param_4,ushort *param_5,int param_6,int param_7,int param_8);
BOOL __cdecl ___crtGetStringTypeA(_locale_t _Plocinfo,DWORD _DWInfoType,LPCSTR _LpSrcStr,int _CchSrc,LPWORD _LpCharType,int _Code_page,BOOL _BError);
void ___free_lc_time(undefined4 *param_1);
void ___free_lconv_num(undefined4 *param_1);
void ___free_lconv_mon(int param_1);
UINT __cdecl ____lc_codepage_func(void);
errno_t __cdecl _strcat_s(char *_Dst,rsize_t _SizeInBytes,char *_Src);
size_t __cdecl _strcspn(char *_Str,char *_Control);
errno_t __cdecl _strncpy_s(char *_Dst,rsize_t _SizeInBytes,char *_Src,rsize_t _MaxCount);
int __cdecl _strcmp(char *_Str1,char *_Str2);
int __cdecl _strncmp(char *_Str1,char *_Str2,size_t _MaxCount);
char * __cdecl _strpbrk(char *_Str,char *_Control);
void ___ansicp(LCID param_1);
void ___convertcp(UINT param_1,UINT param_2,char *param_3,uint *param_4,LPSTR param_5,int param_6);
void __87except(int param_1,int *param_2,ushort *param_3);
undefined4 FUN_300a457c(void);
void __raise_exc_ex(uint *param_1,uint *param_2,uint param_3,int param_4,uint *param_5,uint *param_6,int param_7);
void __raise_exc(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6);
bool __handle_exc(uint param_1,double *param_2,uint param_3);
void __set_errno_from_matherr(int param_1);
char __errcode(byte param_1);
float10 __umatherr(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9);
float10 __handle_qnan1(undefined4 param_1,double param_2,undefined4 param_3);
void __except1(undefined4 param_1,undefined4 param_2,undefined8 param_3,undefined8 param_4,undefined4 param_5);
float10 __frnd(double param_1);
float10 __set_exp(undefined8 param_1,short param_2);
undefined4 __sptype(int param_1,uint param_2);
void FUN_300a4d1f(uint param_1,uint param_2,int *param_3);
int __statfp(void);
int __clrfp(void);
int __ctrlfp(void);
void FUN_300a4e22(void);
void ___set_fpsr_sse2(uint param_1);
longlong __cdecl __lseeki64_nolock(int _FileHandle,longlong _Offset,int _Origin);
longlong __cdecl __lseeki64(int _FileHandle,longlong _Offset,int _Origin);
void FUN_300a5080(void);
void __cdecl __getbuf(FILE *_File);
int __cdecl __isatty(int _FileHandle);
errno_t __cdecl __wctomb_s_l(int *_SizeConverted,char *_MbCh,size_t _SizeInBytes,wchar_t _WCh,_locale_t _Locale);
errno_t __cdecl _wctomb_s(int *_SizeConverted,char *_MbCh,rsize_t _SizeInBytes,wchar_t _WCh);
INTRNCVT_STATUS __cdecl __ld12tod(_LDBL12 *_Ifp,_CRT_DOUBLE *_D);
INTRNCVT_STATUS __cdecl __ld12tof(_LDBL12 *_Ifp,_CRT_FLOAT *_F);
void ___mtold12(char *param_1,int param_2,uint *param_3);
int __cdecl __fpclass(double _X);
int __cdecl __ungetc_nolock(int _Ch,FILE *_File);
int __cdecl __mbtowc_l(wchar_t *_DstCh,char *_SrcCh,size_t _SrcSizeInBytes,_locale_t _Locale);
int __cdecl _mbtowc(wchar_t *_DstCh,char *_SrcCh,size_t _SrcSizeInBytes);
BOOL __cdecl __ValidateImageBase(PBYTE pImageBase);
PIMAGE_SECTION_HEADER __cdecl __FindPESection(PBYTE pImageBase,DWORD_PTR rva);
BOOL __cdecl __IsNonwritableInCurrentImage(PBYTE pTarget);
void FUN_300a70bd(undefined4 param_1);
BOOL ___crtInitCritSecAndSpinCount(LPCRITICAL_SECTION param_1,DWORD param_2);
int __cdecl __set_osfhnd(int param_1,intptr_t param_2);
int __cdecl __free_osfhnd(int param_1);
intptr_t __cdecl __get_osfhandle(int _FileHandle);
int __cdecl ___lock_fhandle(int _Filehandle);
void FUN_300a7341(void);
void __cdecl __unlock_fhandle(int _Filehandle);
int __cdecl __alloc_osfhnd(void);
void FUN_300a7444(void);
void FUN_300a7502(void);
undefined4 __onexit_nolock(undefined4 param_1);
_onexit_t __cdecl __onexit(_onexit_t _Func);
void FUN_300a762c(void);
int __cdecl _atexit(_func_4879 *param_1);
void __initp_eh_hooks(void);
void __initp_misc_winsig(undefined4 param_1);
uint __cdecl siglookup(uint param_1);
_PHNDLR __cdecl ___get_sigabrt(void);
int __cdecl _raise(int _SigNum);
void FUN_300a7869(void);
void FUN_300a78a5(undefined4 param_1);
void FUN_300a78b4(undefined4 param_1);
int __cdecl ___crtMessageBoxA(LPCSTR _LpText,LPCSTR _LpCaption,UINT _UType);
int __cdecl __set_error_mode(int _Mode);
int __tsopen_nolock(undefined4 *param_1,LPCSTR param_2,uint param_3,int param_4,byte param_5);
errno_t __cdecl __sopen_helper(char *_Filename,int _OFlag,int _ShFlag,int _PMode,int *_PFileHandle,int _BSecure);
void FUN_300a8234(void);
errno_t __cdecl __sopen_s(int *_FileHandle,char *_Filename,int _OpenFlag,int _ShareFlag,int _PermissionMode);
int __cdecl __mbsnbicmp_l(uchar *_Str1,uchar *_Str2,size_t _MaxCount,_locale_t _Locale);
int __cdecl __mbsnbicmp(uchar *_Str1,uchar *_Str2,size_t _MaxCount);
int __cdecl __mbsnbcmp_l(uchar *_Str1,uchar *_Str2,size_t _MaxCount,_locale_t _Locale);
int __cdecl __mbsnbcmp(uchar *_Str1,uchar *_Str2,size_t _MaxCount);
void __global_unwind2(PVOID param_1);
void __local_unwind2(int param_1,uint param_2);
void __NLG_Notify(ulong param_1);
void FUN_300a8780(void);
wint_t __cdecl __putwch_nolock(wchar_t _WCh);
void * __cdecl _realloc(void *_Memory,size_t _NewSize);
void FUN_300a898c(void);
void * __cdecl __recalloc(void *_Memory,size_t _Count,size_t _Size);
int __cdecl strncnt(char *param_1,int param_2);
int __cdecl __crtCompareStringA_stat(localeinfo_struct *param_1,ulong param_2,ulong param_3,char *param_4,int param_5,char *param_6,int param_7,int param_8);
int __cdecl ___crtCompareStringA(_locale_t _Plocinfo,LPCWSTR _LocaleName,DWORD _DwCmpFlags,LPCSTR _LpString1,int _CchCount1,LPCSTR _LpString2,int _CchCount2,int _Code_page);
int __cdecl __strnicoll_l(char *_Str1,char *_Str2,size_t _MaxCount,_locale_t _Locale);
int __cdecl findenv(uchar *param_1);
undefined4 * __cdecl copy_environ(void);
int __cdecl ___crtsetenv(char **_POption,int _Primary);
int __cdecl x_ismbbtype_l(localeinfo_struct *param_1,uint param_2,int param_3,int param_4);
int __cdecl __ismbblead(uint _C);
void __cdecl$I10_OUTPUT(int param_1,uint param_2,ushort param_3,int param_4,byte param_5,short *param_6);
uint __hw_cw(void);
uint __fastcall ___hw_cw_sse2(undefined4 param_1,uint param_2);
uint __cdecl __control87(uint _NewValue,uint _Mask);
int __cdecl __strnicmp_l(char *_Str1,char *_Str2,size_t _MaxCount,_locale_t _Locale);
int __cdecl __strnicmp(char *_Str1,char *_Str2,size_t _MaxCount);
size_t __cdecl __msize(void *_Memory);
void FUN_300aa255(void);
void __cdecl _abort(void);
int __cdecl __chsize_nolock(int _FileHandle,longlong _Size);
int __cdecl __setmode_nolock(int _FileHandle,int _Mode);
errno_t __cdecl __get_fmode(int *_PMode);
void __cdecl ___initconout(void);
char * __cdecl __strdup(char *_Src);
uchar * __cdecl __mbschr_l(uchar *_Str,uint _Ch,_locale_t _Locale);
uchar * __cdecl __mbschr(uchar *_Str,uint _Ch);
int __cdecl ___ascii_strnicmp(char *_Str1,char *_Str2,size_t _MaxCount);
void GetAdaptersInfo(void);
void RtlUnwind(PVOID TargetFrame,PVOID TargetIp,PEXCEPTION_RECORD ExceptionRecord,PVOID ReturnValue);

