#pragma once

#include <memory>

class ChatRoomManager;
class UserManager;

extern std::unique_ptr<ChatRoomManager> GRoomManager;
extern std::unique_ptr<UserManager> GUserManager;

constexpr int CHAT_ROOM_COUNT = 10;
