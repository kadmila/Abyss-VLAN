#include "../../include/platform.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include <Winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <netioapi.h>
#include <combaseapi.h>

#include "../../include/defaults.h"

#include "../../third_party/wintun/include/wintun.h"

static WINTUN_CREATE_ADAPTER_FUNC *WintunCreateAdapter;
static WINTUN_CLOSE_ADAPTER_FUNC *WintunCloseAdapter;
static WINTUN_OPEN_ADAPTER_FUNC *WintunOpenAdapter;
static WINTUN_GET_ADAPTER_LUID_FUNC *WintunGetAdapterLUID;
static WINTUN_GET_RUNNING_DRIVER_VERSION_FUNC *WintunGetRunningDriverVersion;
static WINTUN_DELETE_DRIVER_FUNC *WintunDeleteDriver;
static WINTUN_SET_LOGGER_FUNC *WintunSetLogger;
static WINTUN_START_SESSION_FUNC *WintunStartSession;
static WINTUN_END_SESSION_FUNC *WintunEndSession;
static WINTUN_GET_READ_WAIT_EVENT_FUNC *WintunGetReadWaitEvent;
static WINTUN_RECEIVE_PACKET_FUNC *WintunReceivePacket;
static WINTUN_RELEASE_RECEIVE_PACKET_FUNC *WintunReleaseReceivePacket;
static WINTUN_ALLOCATE_SEND_PACKET_FUNC *WintunAllocateSendPacket;
static WINTUN_SEND_PACKET_FUNC *WintunSendPacket;

static HMODULE wintun_module;
static int is_stderr_redirected = 1;

static void CALLBACK
WintunLogger(_In_ WINTUN_LOGGER_LEVEL Level, _In_ DWORD64 Timestamp, _In_z_ const WCHAR *LogLine)
{
    SYSTEMTIME SystemTime;
    FileTimeToSystemTime((FILETIME *)&Timestamp, &SystemTime);
    const WCHAR *LevelMarker;
    const WCHAR *ColorCode;
    switch (Level)
    {
    case WINTUN_LOG_INFO:
        LevelMarker = L"INFO";
        ColorCode = L"\x1b[32m";
        break;
    case WINTUN_LOG_WARN:
        LevelMarker = L"WARN";
        ColorCode = L"\x1b[33m";
        break;
    case WINTUN_LOG_ERR:
        LevelMarker = L"ERR!";
        ColorCode = L"\x1b[31m";
        break;
    default:
        return;
    }
    if (is_stderr_redirected)
    {
        fwprintf(
            stderr,
            L"%04u-%02u-%02u %02u:%02u:%02u.%04u [%ls] %ls\n",
            SystemTime.wYear,
            SystemTime.wMonth,
            SystemTime.wDay,
            SystemTime.wHour,
            SystemTime.wMinute,
            SystemTime.wSecond,
            SystemTime.wMilliseconds,
            LevelMarker,
            LogLine);
    }
    else
    {
        fwprintf(
            stderr,
            L"%04u-%02u-%02u %02u:%02u:%02u.%04u %ls[%ls]\033[0m %ls\n",
            SystemTime.wYear,
            SystemTime.wMonth,
            SystemTime.wDay,
            SystemTime.wHour,
            SystemTime.wMinute,
            SystemTime.wSecond,
            SystemTime.wMilliseconds,
            ColorCode,
            LevelMarker,
            LogLine);
    }
}

static DWORD64 Now(VOID)
{
    FILETIME Timestamp;
    GetSystemTimeAsFileTime(&Timestamp);

    ULARGE_INTEGER t;
    t.LowPart  = Timestamp.dwLowDateTime;
    t.HighPart = Timestamp.dwHighDateTime;
    return t.QuadPart;
}

static DWORD
LogError(_In_z_ const WCHAR *Prefix, _In_ DWORD Error)
{
    WCHAR *SystemMessage = NULL, *FormattedMessage = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_MAX_WIDTH_MASK,
        NULL,
        HRESULT_FROM_SETUPAPI(Error),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (void *)&SystemMessage,
        0,
        NULL);
    FormatMessageW(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY |
            FORMAT_MESSAGE_MAX_WIDTH_MASK,
        SystemMessage ? L"%1: %3(Code 0x%2!08X!)" : L"%1: Code 0x%2!08X!",
        0,
        0,
        (void *)&FormattedMessage,
        0,
        (va_list *)(DWORD_PTR[]){ (DWORD_PTR)Prefix, (DWORD_PTR)Error, (DWORD_PTR)SystemMessage });
    if (FormattedMessage)
    {
        WintunLogger(WINTUN_LOG_ERR, Now(), FormattedMessage);
    }
    LocalFree(FormattedMessage);
    LocalFree(SystemMessage);
    return Error;
}

static DWORD
LogLastError(_In_z_ const WCHAR *Prefix)
{
    DWORD LastError = GetLastError();
    LogError(Prefix, LastError);
    SetLastError(LastError);
    return LastError;
}

static void
Log(_In_ WINTUN_LOGGER_LEVEL Level, _In_z_ const WCHAR *Format, ...)
{
    WCHAR LogLine[0x200];
    va_list args;
    va_start(args, Format);
    _vsnwprintf_s(LogLine, _countof(LogLine), _TRUNCATE, Format, args);
    va_end(args);
    WintunLogger(Level, Now(), LogLine);
}

///// Interface implementation

int
avlan_init(void)
{
    wintun_module = LoadLibraryExW(L"./wintun.dll", NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!wintun_module)
    {
        return 1;
    }

#define X(Name) ((*(FARPROC *)&Name = GetProcAddress(wintun_module, #Name)) == NULL)
    if (X(WintunCreateAdapter) || X(WintunCloseAdapter) || X(WintunOpenAdapter) || X(WintunGetAdapterLUID) ||
        X(WintunGetRunningDriverVersion) || X(WintunDeleteDriver) || X(WintunSetLogger) || X(WintunStartSession) ||
        X(WintunEndSession) || X(WintunGetReadWaitEvent) || X(WintunReceivePacket) || X(WintunReleaseReceivePacket) ||
        X(WintunAllocateSendPacket) || X(WintunSendPacket))
#undef X
    {
        FreeLibrary(wintun_module);
        wintun_module = NULL;
        return 1;
    }
    
    is_stderr_redirected = _isatty(_fileno(stderr)) == 0;
    WintunSetLogger(WintunLogger);

    return 0;
}

void 
avlan_cleanup(void)
{
    if (wintun_module)
    {
        FreeLibrary(wintun_module);
        wintun_module = NULL;
    }
}

struct avlan_adapter {
    WINTUN_ADAPTER_HANDLE wintun_adapter;
    WINTUN_SESSION_HANDLE wintun_session;
};

struct avlan_adapter*
avlan_create_adapter(const char *adapter_name, const char *cidr)
{
    wchar_t adapter_name_w[ADAPTER_NAME_MAX_LENGTH];
    struct avlan_adapter *adapter;

    if (MultiByteToWideChar(CP_UTF8, 0, adapter_name, -1, adapter_name_w, ADAPTER_NAME_MAX_LENGTH) == 0) 
    {
        return NULL;
    }
    
    adapter = malloc(sizeof(struct avlan_adapter));
    if (!adapter)
    {
        return NULL;
    }

    GUID adapter_guid;
    if (CoCreateGuid(&adapter_guid) != S_OK)
    {
        goto fail;
    }

    adapter->wintun_adapter = WintunCreateAdapter(adapter_name_w, L"AbyssVLAN", &adapter_guid);
    if (!adapter->wintun_adapter)
    {
        goto fail;
    }

    MIB_UNICASTIPADDRESS_ROW address_row;

    char ip[16]; // "255.255.255.255"
    const char *slash = strchr(cidr, '/');
    if (slash == NULL)
    {
        goto fail;
    }

    size_t len = (size_t)(slash - cidr);
    if (len >= sizeof(ip))
    {
        goto fail;
    }

    memcpy(ip, cidr, len);
    ip[len] = '\0';

    int prefix = atoi(slash + 1);
    if (prefix < 0 || prefix > 32)
    {
        goto fail;
    }

    InitializeUnicastIpAddressEntry(&address_row);
    WintunGetAdapterLUID(adapter->wintun_adapter, &address_row.InterfaceLuid);

    address_row.Address.si_family = AF_INET;
    if (InetPtonA(AF_INET, ip, &address_row.Address.Ipv4.sin_addr) != 1)
    {
        goto fail;
    }

    address_row.OnLinkPrefixLength = (UINT8)prefix;
    address_row.DadState = IpDadStatePreferred; // state no ip address conflict with other adapters in this network
    NTSTATUS status = CreateUnicastIpAddressEntry(&address_row);
    if (status != ERROR_SUCCESS && status != ERROR_OBJECT_ALREADY_EXISTS)
    {
        SetLastError(status);
        goto fail;
    }

    adapter->wintun_session = WintunStartSession(adapter->wintun_adapter, 0x400000);
    if (!adapter->wintun_session)
    {
        goto fail;
    }

    return adapter;

fail:
    free(adapter);
    return NULL;
}

int 
avlan_adapter_poll(struct avlan_adapter *adapter, uint8_t *buffer, uint32_t buffer_size, uint32_t *bytes_received)
{
    *bytes_received = 0;

    DWORD packet_size;
    BYTE *packet = WintunReceivePacket(adapter->wintun_session, &packet_size); //non-blocking
    if (!packet) {
        DWORD last_error = GetLastError();
        if (last_error == ERROR_NO_MORE_ITEMS) {
            return 0;
        } else {
            SetLastError(last_error);
            return 1;
        }
    }

    if (packet_size > buffer_size)
    {
        goto fail;
    }
    
    memcpy(buffer, packet, packet_size);
    *bytes_received = packet_size;

    WintunReleaseReceivePacket(adapter->wintun_session, packet);
    return 0;

fail:
    WintunReleaseReceivePacket(adapter->wintun_session, packet);
    return 1;
}

int 
avlan_adapter_wait(struct avlan_adapter *adapter, uint32_t timeout_ms)
{
    return WaitForSingleObject(WintunGetReadWaitEvent(adapter->wintun_session), timeout_ms) == WAIT_OBJECT_0 ? 0 : 1;
}

int 
avlan_adapter_write(struct avlan_adapter *adapter, const uint8_t *buffer, uint32_t length)
{
    BYTE *packet = WintunAllocateSendPacket(adapter->wintun_session, (DWORD)length);
    if (!packet)
    {
        return 1;
    }

    memcpy(packet, buffer, length);
    WintunSendPacket(adapter->wintun_session, packet);
    return 0;
}

void 
avlan_close_adapter(struct avlan_adapter *adapter) {
    if (!adapter)
    {
        return;
    }

    WintunEndSession(adapter->wintun_session);
    WintunCloseAdapter(adapter->wintun_adapter);

    free(adapter);
}
