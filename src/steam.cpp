#include <string>
#if defined(_WIN32)
# include <Shlwapi.h>
# include <windows.h>
#else
# include <dlfcn.h>
# include <sys/stat.h>
# include <unistd.h>
# include <cstring>
# include <cstdio>
# include <cwchar>
#endif

#include "../deps/osw/Steamworks.h"
#include "../deps/osw/ISteamUser017.h"
#include "steam.h"

std::wstring Steam::GetSteamInstallFolder()
{
#if defined(_WIN32)
	wchar_t installFolder[MAX_PATH] = { 0 };
	unsigned long bufferLength = sizeof(installFolder);
	unsigned long type = REG_SZ;

	if (SHGetValue(HKEY_CURRENT_USER, TEXT("Software\\Valve\\Steam"), TEXT("SteamPath"), &type, installFolder, &bufferLength) != ERROR_SUCCESS)
	{
		MessageBox(0, L"Could not find Steam Install directory in:\n HKEY_CURRENT_USER\\Software\\Valve\\Steam\\SteamPath)\n", L"leysourceengineclient - GetSteamInstallFolder", MB_OK);
	}

	return installFolder;
#else
	// Try common Linux Steam install locations
	const char* home = getenv("HOME");
	std::string candidates[] = {
		std::string(home ? home : "") + "/.steam/steam",
		std::string(home ? home : "") + "/.local/share/Steam",
		"/usr/lib/steam",
		"/usr/lib64/steam"
	};

	struct stat st;
	for (auto &p : candidates)
	{
		if (p.empty())
			continue;
		if (stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
		{
			// convert to wstring
			std::wstring ws(p.begin(), p.end());
			return ws;
		}
	}

	return L"";
#endif
}

ISteamUser017* Steam::GetSteamUser()
{
	return this->steamUser;
}

ISteamUser017* temporaryHack = 0;

int Steam::Initiate()
{
#if defined(_WIN32)
	SetDllDirectory(GetSteamInstallFolder().c_str());

	HMODULE steam = LoadLibrary(L"steam");
	HMODULE steamapi = LoadLibrary(L"steam_api");
	HMODULE steamclientdll = LoadLibrary(L"steamclient");

	if (!steamclientdll)
	{
		return 1;
	}

	CreateInterfaceFn fnApiInterface = (CreateInterfaceFn)GetProcAddress(steamclientdll, "CreateInterface");

	if (!fnApiInterface)
	{
		return 2;
	}

	if (!(this->steamClient = (ISteamClient017*)fnApiInterface(STEAMCLIENT_INTERFACE_VERSION_017, NULL)))
	{
		return 3;
	}

	if (!(this->steamPipeHandle = this->steamClient->CreateSteamPipe()))
	{
		return 4;
	}

	if (!(this->steamUserHandle = this->steamClient->ConnectToGlobalUser(this->steamPipeHandle)))
	{
		return 5;
	}

	if (!(this->steamUser = (ISteamUser017*)this->steamClient->GetISteamUser(this->steamUserHandle, this->steamPipeHandle, STEAMUSER_INTERFACE_VERSION_017)))
	{
		return 6;
	}

	temporaryHack = this->steamUser;

	return 0;
#else
	// Try to dlopen common steamclient library names and lookup CreateInterface
	const char* names[] = {"steamclient.so", "libsteamclient.so", "libsteam.so", "libsteam_api.so", NULL};
	void* handle = NULL;
	for (const char** n = names; *n != NULL; ++n)
	{
		handle = dlopen(*n, RTLD_NOW | RTLD_GLOBAL);
		if (handle)
			break;
	}

	if (!handle)
	{
		fprintf(stderr, "Could not load steamclient library (dlopen failed)\n");
		return 1;
	}

	void* sym = dlsym(handle, "CreateInterface");
	if (!sym)
	{
		fprintf(stderr, "CreateInterface not found in steamclient library\n");
		dlclose(handle);
		return 2;
	}

	CreateInterfaceFn fnApiInterface = reinterpret_cast<CreateInterfaceFn>(sym);

	// Try multiple SteamClient interface versions (newest -> oldest)
	const char* client_versions[] = {
		STEAMCLIENT_INTERFACE_VERSION_017,
		"SteamClient016",
		"SteamClient015",
		"SteamClient014",
		"SteamClient013",
		"SteamClient012",
		"SteamClient011",
		"SteamClient010",
		"SteamClient009",
		"SteamClient008",
		"SteamClient007",
		"SteamClient006",
		NULL
	};

	ISteamClient017* client = NULL;
	for (const char** ver = client_versions; *ver != NULL; ++ver)
	{
		fprintf(stderr, "Attempting CreateInterface('%s')...\n", *ver);
		int retcode = 0;
		void* iface = fnApiInterface(*ver, &retcode);
		fprintf(stderr, " -> returned %p (retcode=%d)\n", iface, retcode);
		if (iface)
		{
			client = reinterpret_cast<ISteamClient017*>(iface);
			fprintf(stderr, "Selected interface %s -> %p\n", *ver, client);
			break;
		}
	}

	if (!(this->steamClient = client))
	{
		dlclose(handle);
		return 3;
	}

	if (!(this->steamPipeHandle = this->steamClient->CreateSteamPipe()))
	{
		dlclose(handle);
		return 4;
	}

	if (!(this->steamUserHandle = this->steamClient->ConnectToGlobalUser(this->steamPipeHandle)))
	{
		dlclose(handle);
		return 5;
	}

	if (!(this->steamUser = reinterpret_cast<ISteamUser017*>(this->steamClient->GetISteamUser(this->steamUserHandle, this->steamPipeHandle, STEAMUSER_INTERFACE_VERSION_017))))
	{
		dlclose(handle);
		return 6;
	}

	temporaryHack = this->steamUser;

	return 0;
#endif
}
