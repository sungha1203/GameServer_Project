using System.Buffers.Binary;
using System.IO;
using System.Net.Sockets;
using System.Text;

namespace ChatClientUI;

internal sealed class ChatTcpClient : IAsyncDisposable
{
    private TcpClient? tcpClient;
    private NetworkStream? stream;
    private CancellationTokenSource? receiveCts;

    public event Action<ChatPacket>? PacketReceived;
    public event Action<string>? Disconnected;

    public bool IsConnected => tcpClient?.Connected == true;

    public async Task ConnectAsync(string ip, int port)
    {
        await DisposeAsync();

        tcpClient = new TcpClient();
        await tcpClient.ConnectAsync(ip, port);
        stream = tcpClient.GetStream();
        receiveCts = new CancellationTokenSource();
        _ = Task.Run(() => ReceiveLoopAsync(receiveCts.Token));
    }

    public async Task SendAsync(ushort packetId, string payload)
    {
        if (stream == null)
            throw new InvalidOperationException("Not connected.");

        byte[] payloadBytes = Encoding.UTF8.GetBytes(payload);
        int packetSize = 4 + payloadBytes.Length;
        byte[] packet = new byte[packetSize];

        BinaryPrimitives.WriteUInt16LittleEndian(packet.AsSpan(0, 2), (ushort)packetSize);
        BinaryPrimitives.WriteUInt16LittleEndian(packet.AsSpan(2, 2), packetId);
        payloadBytes.CopyTo(packet.AsSpan(4));

        await stream.WriteAsync(packet);
    }

    private async Task ReceiveLoopAsync(CancellationToken token)
    {
        byte[] headerBuffer = new byte[4];

        try
        {
            while (!token.IsCancellationRequested && stream != null)
            {
                await ReadExactAsync(headerBuffer, token);

                ushort size = BinaryPrimitives.ReadUInt16LittleEndian(headerBuffer.AsSpan(0, 2));
                ushort id = BinaryPrimitives.ReadUInt16LittleEndian(headerBuffer.AsSpan(2, 2));

                if (size < 4)
                    throw new InvalidDataException($"Invalid packet size: {size}");

                byte[] payloadBuffer = new byte[size - 4];
                if (payloadBuffer.Length > 0)
                    await ReadExactAsync(payloadBuffer, token);

                string payload = Encoding.UTF8.GetString(payloadBuffer);
                PacketReceived?.Invoke(new ChatPacket(id, payload));
            }
        }
        catch (OperationCanceledException)
        {
        }
        catch (Exception ex)
        {
            Disconnected?.Invoke(ex.Message);
        }
    }

    private async Task ReadExactAsync(byte[] buffer, CancellationToken token)
    {
        int offset = 0;
        while (offset < buffer.Length)
        {
            if (stream == null)
                throw new IOException("Disconnected.");

            int read = await stream.ReadAsync(buffer.AsMemory(offset, buffer.Length - offset), token);
            if (read == 0)
                throw new IOException("Disconnected.");

            offset += read;
        }
    }

    public async ValueTask DisposeAsync()
    {
        receiveCts?.Cancel();
        receiveCts?.Dispose();
        receiveCts = null;

        stream?.Dispose();
        stream = null;

        tcpClient?.Close();
        tcpClient = null;

        await Task.CompletedTask;
    }
}
