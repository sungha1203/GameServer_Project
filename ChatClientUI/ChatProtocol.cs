namespace ChatClientUI;

internal static class ChatProtocol
{
    public const ushort C_LOGIN_REQ = 1001;
    public const ushort S_LOGIN_RES = 1002;

    public const ushort C_ROOM_LIST_REQ = 1051;
    public const ushort S_ROOM_LIST_RES = 1052;

    public const ushort C_ROOM_ENTER_REQ = 1101;
    public const ushort S_ROOM_ENTER_RES = 1102;
    public const ushort S_ROOM_USER_JOIN = 1103;
    public const ushort S_ROOM_USER_LEAVE = 1104;
    public const ushort C_ROOM_LEAVE_REQ = 1105;
    public const ushort S_ROOM_LEAVE_RES = 1106;

    public const ushort C_CHAT_REQ = 1201;
    public const ushort S_CHAT = 1202;

    public const ushort S_ERROR = 9001;
}
