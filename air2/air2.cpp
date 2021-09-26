
#include "pch.h"

#include <Windows.h>
#pragma comment(linker,"/EXPORT:?air_roi_create@@YAHPAPAXABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z=air2Org.?air_roi_create@@YAHPAPAXABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z")
#pragma comment(linker,"/EXPORT:?air_roi_destroy@@YAHPAX@Z=air2Org.?air_roi_destroy@@YAHPAX@Z")
#pragma comment(linker,"/EXPORT:?air_roi_detect@@YAHPAXPBEHHHW4air_pixel_format@@PAEHH@Z=air2Org.?air_roi_detect@@YAHPAXPBEHHHW4air_pixel_format@@PAEHH@Z")
#pragma comment(linker,"/EXPORT:air_dl_segment_create=air2Org.air_dl_segment_create")
#pragma comment(linker,"/EXPORT:air_dl_segment_destroy=air2Org.air_dl_segment_destroy")
#pragma comment(linker,"/EXPORT:air_dl_segment_detect=air2Org.air_dl_segment_detect")
#pragma comment(linker,"/EXPORT:air_dl_segment_get_insize=air2Org.air_dl_segment_get_insize")


// 创建成员指针偏移
const DWORD g_dwCreateMemberOffset = 0x1E1523;
const DWORD g_dwOrginCallOffset = 0x36BB;
// 邀请call偏移
const DWORD g_dwInviteCallOffset = 0x1974c0;
// 释放成员指针偏移
const DWORD g_dwDeleteMemberOffset = 0x196380;


#define HOOK_LEN 5
VOID SetHook(DWORD_PTR dwHookAddr, LPVOID dwPfnAddr, BYTE* btBackCode)
{
    // 准备HOOK
    BYTE btJmp[HOOK_LEN] = { 0xE9 ,0,0,0,0 };
    *(DWORD*)&btJmp[1] = (DWORD)dwPfnAddr - dwHookAddr - HOOK_LEN;
    // 获取自身进程句柄
    HANDLE hProcess = GetCurrentProcess();
    // 备份数据
    if (!ReadProcessMemory(hProcess, (LPVOID)dwHookAddr, btBackCode, HOOK_LEN, NULL))
    {
        MessageBox(NULL, "HOOK失败", NULL, NULL);
        return;
    }
    // 开始hook
    if (!WriteProcessMemory(hProcess, (LPVOID)dwHookAddr, btJmp, HOOK_LEN, NULL))
    {
        MessageBox(NULL, "HOOK失败", NULL, NULL);
        return;
    }
}

VOID UnSetHook(DWORD_PTR dwHookAddr, BYTE* btBackCode)
{
    // 获取自身进程句柄
    HANDLE hProcess = GetCurrentProcess();
    // 卸载hook
    if (!WriteProcessMemory(hProcess, (LPVOID)dwHookAddr, btBackCode, HOOK_LEN, NULL))
    {
        MessageBox(NULL, "卸载HOOK失败", NULL, NULL);
        return;
    }
}

//DWORD WINAPI ThreadProc(LPVOID lpParameter)
//{
//    while (true)
//    {
//
//        if (GetAsyncKeyState(VK_F2) & 0x8000)
//        {
//            MessageBoxA(NULL, "开始邀请自己", NULL, NULL);
//            DWORD dwClassroomHandle = (DWORD)GetModuleHandle("classroom.dll");
//            DWORD dwInvateCallAddr = dwClassroomHandle + g_dwInviteCallOffset;
//            DWORD dwTestAddr = dwClassroomHandle + 0x542FB4;
//            DWORD dwStuAddr = 0x0ED7F680;
//            //__asm
//            //{
//            //    pushad
//            //    pushf
//            //    mov ecx, dwStuAddr
//            //    call dwInvateCallAddr
//            //    popf
//            //    popad
//            //}
//        }
//        if (GetAsyncKeyState(VK_F3) & 0x8000)
//        {
//            MessageBoxA(NULL, "开始邀请张三", NULL, NULL);
//            DWORD dwClassroomHandle = (DWORD)GetModuleHandle("classroom.dll");
//            DWORD dwInvateCallAddr = dwClassroomHandle + g_dwInviteCallOffset;
//            DWORD dwStuAddr = 0x0ED7F7B0;
//            __asm
//            {
//                pushad
//                pushf
//                mov ecx, dwStuAddr
//                call dwInvateCallAddr
//                popf
//                popad
//            }
//        }
//    }
//}

// 邀请原数据
DWORD g_dwCreateMemberOrgCallAddr = NULL;
DWORD g_dwCreateMemberAddr = NULL;
BYTE g_btCreateMemberCode[5]{};
DWORD g_dwCreateMemberCallback = NULL;
DWORD g_dwInvateCallAddr = NULL;
DWORD g_dwMemberPoint = NULL;
void AutoInviteMember()
{
    __asm
    {
        mov ecx, g_dwMemberPoint
        call g_dwInvateCallAddr
    }
    return;
}

_declspec(naked) void HookCreateMemberFunc()
{
    __asm 
    {
        pushad
        pushf
        mov eax, dword ptr ss:[ebp - 0x48]
        mov [g_dwMemberPoint], eax
    }
    AutoInviteMember();
    __asm
    {
        popf
        popad
        call g_dwCreateMemberOrgCallAddr
        jmp g_dwCreateMemberCallback
    }
}
//
//// 原数据
//DWORD m_dwDeleteMemberReturn[5] = { 0 };
//_declspec(naked) void HookDeleteMemberFunc()
//{
//    MessageBox(NULL, "有人离开了", NULL, NULL);
//}

// 入口函数
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        // 初始化数据
        DWORD dwClassroomHandle = (DWORD)GetModuleHandle("classroom.dll");
        g_dwInvateCallAddr = dwClassroomHandle + g_dwInviteCallOffset;
        g_dwCreateMemberAddr = dwClassroomHandle + g_dwCreateMemberOffset;
        g_dwCreateMemberCallback = g_dwCreateMemberAddr + HOOK_LEN;
        g_dwCreateMemberOrgCallAddr = dwClassroomHandle + g_dwOrginCallOffset;

        // HOOK加入成员生成指针部分
        SetHook(g_dwCreateMemberAddr, HookCreateMemberFunc, g_btCreateMemberCode);

        //HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, NULL, NULL);
        //if (hThread != NULL)
        //{
        //    CloseHandle(hThread);
        //}
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        UnSetHook(g_dwCreateMemberAddr, g_btCreateMemberCode);
    }   

    return TRUE;
}