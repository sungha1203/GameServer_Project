#include "pch.h"
#include "UserManager.h"

bool UserManager::Login(const std::string& requestedNickname, std::string& assignedNickname, std::string& reason)
{
	std::string nickname = Trim(requestedNickname);

	std::lock_guard<std::mutex> lock(userLock);

	if (nickname.empty())
		nickname = CreateGuestNickname();

	if (nicknames.find(nickname) != nicknames.end())
	{
		reason = "Duplicate nickname";
		return false;
	}

	nicknames.insert(nickname);
	assignedNickname = nickname;
	return true;
}

void UserManager::Logout(const std::string& nickname)
{
	if (nickname.empty())
		return;

	std::lock_guard<std::mutex> lock(userLock);
	nicknames.erase(nickname);
}

int UserManager::GetUserCount() const
{
	std::lock_guard<std::mutex> lock(userLock);
	return static_cast<int>(nicknames.size());
}

std::string UserManager::CreateGuestNickname()
{
	std::string nickname;

	do
	{
		const int id = guestId.fetch_add(1);
		nickname = "Guest" + std::to_string(id);
	} while (nicknames.find(nickname) != nicknames.end());

	return nickname;
}

std::string UserManager::Trim(const std::string& value) const
{
	const size_t begin = value.find_first_not_of(" \t\r\n");
	if (begin == std::string::npos)
		return "";

	const size_t end = value.find_last_not_of(" \t\r\n");
	return value.substr(begin, end - begin + 1);
}
