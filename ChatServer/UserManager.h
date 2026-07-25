#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_set>

class UserManager
{
public:
	bool Login(const std::string& requestedNickname, std::string& assignedNickname, std::string& reason);
	void Logout(const std::string& nickname);
	int GetUserCount() const;

private:
	std::string CreateGuestNickname();
	std::string Trim(const std::string& value) const;

private:
	mutable std::mutex userLock;
	std::unordered_set<std::string> nicknames;
	std::atomic<int> guestId{ 1 };
};
