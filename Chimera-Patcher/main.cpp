#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "scanner.hpp"
#define WIN32_LEAN_AND_MEAN  
#include <windows.h>

#define OLDSRV "69 00 6D 00 2E 00 6D 00 79 00 73 00 70 00 61 00 63 00 65 00 2E 00 61 00 6B 00 61 00 64 00 6E 00 73 00 2E 00 6E 00 65 00 74"
#define OLDAD "64 00 65 00 49 00 4D 00 2E 00 6D 00 79 00 73 00 70 00 61 00 63 00 65 00 2E 00 63 00 6F 00 6D 00 2F"
#define NEWSRV "6D 00 79 00 69 00 6D 00 2E 00 6D 00 79 00 73 00 70 00 61 00 63 00 65 00 2E 00 63 00 6F 00 6D 00"
#define NEWAD "? ? ? ? ? ? ? ? 2E 00 6F 00 70 00 74 00 2E 00 66 00 69 00 6D 00 73 00 65 00 72 00 76 00 65 00 2E 00 63 00 6F 00 6D 00"
#define ERR "45 00 52 00 52 00 4F 00 52 00 3A 00 20 00 43 00 61 00 6E 00 6E 00 6F 00 74 00 20 00"
#define GHBN "C6 84 24 B0 00 00 00 ? 8B 4C 24 24"

int main()
{
	std::string path = "";
    std::string serverurl = "mms.phantom-im.xyz";
    std::string adsrvurl = "";
    DWORD64 erroraddr;
    int version = 0;
	std::cout << "\u001b[34m";
	std::cout << R"(

   ______  __        _                                      _____  ____    ____  
 .' ___  |[  |      (_)                                    |_   _||_   \  /   _| 
/ .'   \_| | |--.   __   _ .--..--.  .---.  _ .--.  ,--.     | |    |   \/   |   
| |        | .-. | [  | [ `.-. .-. |/ /__\\[ `/'`\]`'_\ :    | |    | |\  /| |   
\ `.___.'\ | | | |  | |  | | | | | || \__., | |    // | |,  _| |_  _| |_\/_| |_  
 `.____ .'[___]|__][___][___||__||__]'.__.'[___]   \'-;__/ |_____||_____||_____| 
                                                                                 

)" << std::endl;
	std::cout << "Searching for existing MySpaceIM installations...\n";
	path = "C:\\Program Files (x86)\\MySpace\\IM\\MySpaceIM.exe";
	if (!std::filesystem::exists(path)) {
        path = "C:\\Program Files\\MySpace\\IM\\MySpaceIM.exe";
        if (!std::filesystem::exists(path)) {
            std::cout << "Couldn't find any existing installations. Please manually enter the path to your MySpaceIM executable\n: ";
            std::cin >> path;
            if (!std::filesystem::exists(path)) {
                std::cout << "MySpaceIM executable doesn't exist! Exiting in 10 seconds...";
                Sleep(10000);
                return 1;
            }
        }
	}
	std::cout << "Found existing myspace installation, trying to detect version...\n";
	std::ifstream exe = std::ifstream(path, std::ios_base::binary | std::ios_base::in);
	std::stringstream tmp;
	tmp << exe.rdbuf();
    std::string buf = tmp.str();
    std::vector<unsigned char> file(buf.begin(), buf.end());
	exe.close();
    DWORD  verHandle = 0;
    UINT   size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD  verSize = GetFileVersionInfoSizeA(path.c_str(), &verHandle);
    if (verSize == NULL) {
        std::cout << "Can't read version info from file! Closing in 10 seconds...\n";
        Sleep(10000);
        return 1;
    }
    LPSTR verData = new char[verSize];
    if (GetFileVersionInfoA(path.c_str(), verHandle, verSize, verData)) {
        if (VerQueryValueA(verData, "\\", (VOID FAR * FAR*) & lpBuffer, &size)) {
            if (size) {
                VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
                if (verInfo->dwSignature == 0xfeef04bd)
                    version = (verInfo->dwFileVersionLS >> 16) & 0xffff;
            }
        }
    }
    if (version == 0) {
        std::cout << "Can't read version info from file! Closing in 10 seconds...\n";
        Sleep(10000);
        return 1;
    }
    delete[] verData;
    std::string selfhost;
    retry:
    std::cout << "Do you want to patch MySpaceIM for self host(answering with no will use the publicly hosted instance) [yes/no]\n: ";
    std::cin >> selfhost;
    if (selfhost == "yes") {
        std::cout << "Enter the hostname of your selfhosted instance\n: ";
        std::cin >> selfhost;
        if (selfhost.length() > 21 && version < 700 || selfhost.length() > 16 && version > 700) {
            std::cout << "The character limit for the hostname is 21 characters for versions before 700 and 16 characters for versions after 700.\n";
            goto retry;
        }
        serverurl = selfhost;
    }
    else if(selfhost != "no") {
        std::cout << "Invalid input.\n";
        goto retry;
    }

    if (version > 700) 
        adsrvurl = serverurl + "/adopt/";
    else 
        adsrvurl = serverurl + "/html.ng/";
    pattern_batch main;
    if (version > 200 && version < 700) {
        std::cout << "First generation MySpaceIM detected. Version: " << version << std::endl;
        std::cout << "Scanning for patches...\n";
        std::string srv = OLDSRV;
        if (version == 697) 
            srv = NEWSRV;
        main.add("SRVURL", srv.c_str(), [&](ptr_manage mg) {
            std::cout << "SRV Location in binary: " << std::hex << mg.as<std::uintptr_t>() << std::endl;
            std::uintptr_t index = mg.as<std::uintptr_t>();
            for (int i = 0, j = 0; i < serverurl.size(); i++, j+=2) {
                file[index + j] = serverurl[i];
                file[index + j + 1] = 0x00;
            }
            file[index + serverurl.size() * 2] = 0x00;
            file[index + serverurl.size() * 2 + 1] = 0x00;
        });
        main.add("ADSRVURL", OLDAD, [&](ptr_manage mg) {
            std::cout << "AdSRVLocation in binary: " << std::hex << mg.as<std::uintptr_t>() << std::endl;
            std::uintptr_t index = mg.as<std::uintptr_t>();
            for (int i = 0, j = 0; i < adsrvurl.size(); i++, j+=2) {
                file[index + j] = adsrvurl[i];
                file[index + j + 1] = 0x00;
            }
            file[index + adsrvurl.size() * 2] = 0x00;
            file[index + adsrvurl.size() * 2 + 1] = 0x00;
        });
        bool found = main.run(file);
        if (!found) {
            std::cout << "Failed to apply some patches. Closing in 10 seconds\n";
            Sleep(10000);
            return 1;
        }
    }
    else if (version > 700 && version < 900) {
        std::cout << "Second generation MySpaceIM detected. Version: " << version << std::endl;
        std::cout << "Scanning for patches...\n";
        main.add("SRVURL", NEWSRV, [&](ptr_manage mg) {
            std::cout << "SRV Location in binary: " << std::hex << mg.as<std::uintptr_t>() << std::endl;
            std::uintptr_t index = mg.as<std::uintptr_t>();
            for (int i = 0, j = 0; i < serverurl.size(); i++, j += 2) {
                file[index + j] = serverurl[i];
                file[index + j + 1] = 0x00;
            }
            file[index + serverurl.size() * 2] = 0x00;
            file[index + serverurl.size() * 2 + 1] = 0x00;
        });
        main.add("ADSRVURL", NEWAD, [&](ptr_manage mg) {
            std::cout << "AdSRVLocation in binary: " << std::hex << mg.as<std::uintptr_t>() << std::endl;
            std::uintptr_t index = mg.as<std::uintptr_t>();
            for (int i = 0, j = 0; i < adsrvurl.size(); i++, j += 2) {
                file[index + j] = adsrvurl[i];
                file[index + j + 1] = 0x00;
            }
            file[index + adsrvurl.size() * 2] = 0x00;
            file[index + adsrvurl.size() * 2 + 1] = 0x00;
        });
        main.add("ERR", ERR, [&](ptr_manage mg) {
            std::cout << "Error Location in binary: " << std::hex << mg.as<std::uintptr_t>() << std::endl;
            std::uintptr_t index = mg.as<std::uintptr_t>();
            for (int i = 0;i < serverurl.size(); i++) {
                file[index + i] = serverurl[i];
            }
            file[index + serverurl.size()] = 0x00;
            erroraddr = index + 0x400000;
        });
        main.add("GHBN", GHBN, [&](ptr_manage mg) {
            std::cout << "GHBN Location in binary: " << std::hex << mg.as<std::uintptr_t>() << std::endl;
            std::uintptr_t index = mg.as<std::uintptr_t>();
            file[index] = 0xB9;
            for (int i = 0; i < 4; i++) {
                int byte = (erroraddr >> (i * 8)) & 0xFF;
                file[index + i + 1] = byte;
            }
            for (int i = 0; i < 7; i++) {
                file[index + i + 5] = 0x90;
            }
        });
        bool found = main.run(file);
        if (!found) {
            std::cout << "Failed to apply some patches. Closing in 10 seconds\n";
            Sleep(10000);
            return 1;
        }
    }
    std::ofstream o = std::ofstream(path, std::ios_base::binary | std::ios_base::out);
    o.write((const char*)file.data(), file.size());
    o.close();
    std::cout << "Patching done. Closing in 10 seconds\n";
    Sleep(10000);
	return 0;
}
