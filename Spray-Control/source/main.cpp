#include <iostream>
#include <thread>

#include "..\header\memory.h"

namespace offset
{
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEA98C;
	constexpr ::std::ptrdiff_t dwClientState = 0x59F19C;
	constexpr ::std::ptrdiff_t dwClientState_GetLocalPlayer = 0x180;
	constexpr ::std::ptrdiff_t dwClientState_IsHLTV = 0x4D48;
	constexpr ::std::ptrdiff_t dwClientState_Map = 0x28C;
	constexpr ::std::ptrdiff_t dwClientState_MapDirectory = 0x188;
	constexpr ::std::ptrdiff_t dwClientState_MaxPlayer = 0x388;
	constexpr ::std::ptrdiff_t dwClientState_PlayerInfo = 0x52C0;
	constexpr ::std::ptrdiff_t dwClientState_State = 0x108;
	constexpr ::std::ptrdiff_t dwClientState_ViewAngles = 0x4D90;

	constexpr ::std::ptrdiff_t m_aimPunchAngle = 0x303C;
	constexpr ::std::ptrdiff_t m_aimPunchAngleVel = 0x3048;
	constexpr ::std::ptrdiff_t m_iShotsFired = 0x103E0;
}

struct Vector2
{
	float x = { }, y = { };
};

void SetConsoleColor(WORD color)
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, color);
}

void LogDebug(const std::string& message)
{
	std::cout << "\033[32m[Info]\033[0m " << message << std::endl;
	SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset color to default
}

void SetConsoleTitle(const std::string& title)
{
	SetConsoleTitleA(title.c_str());
}

int main()
{
	SetConsoleTitle("SprayControl by @soevielofficial");

	const auto memory = Memory{ "csgo.exe" };

	const auto client = memory.GetModuleAddress("client.dll");
	const auto engine = memory.GetModuleAddress("engine.dll");

	auto oldPunch = Vector2{ };

	LogDebug("Successfully wrote value to memory.");

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		const auto& localPlayer = memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
		const auto& shotFired = memory.Read<std::int32_t>(client + offset::m_iShotsFired);

		if (shotFired)
		{
			const auto& clientState = memory.Read<std::uintptr_t>(engine + offset::dwClientState);
			const auto& viewAngles = memory.Read<Vector2>(clientState + offset::dwClientState_ViewAngles);
			const auto& aimPunch = memory.Read<Vector2>(localPlayer + offset::m_aimPunchAngle);

			auto newAngles = Vector2
			{
				viewAngles.x + oldPunch.x - aimPunch.x * 2.f,
				viewAngles.y + oldPunch.y - aimPunch.y * 2.f
			};

			if (newAngles.x > 89.f)
				newAngles.x = 89.f;

			if (newAngles.x < -89.f)
				newAngles.x = -89.f;
			
			while (newAngles.y > 180.f)
				newAngles.y -= 360.f;

			while (newAngles.y < -180.f)
				newAngles.y += 360.f;

			memory.Write<Vector2>(clientState + offset::dwClientState_ViewAngles, newAngles);

			oldPunch.x = aimPunch.x * 2.f;
			oldPunch.y = aimPunch.y * 2.f;

		}
		else
		{
			oldPunch.x = oldPunch.y = 0.f;
		}
	}

	return 0;	
}