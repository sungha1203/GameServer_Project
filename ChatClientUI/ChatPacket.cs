namespace ChatClientUI;

internal readonly record struct ChatPacket(ushort Id, string Payload);
