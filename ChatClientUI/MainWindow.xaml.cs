using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;

namespace ChatClientUI;

public partial class MainWindow : Window
{
    private const string ServerIp = "172.30.1.48";
    private const int ServerPort = 7778;

    private readonly ChatTcpClient client = new();
    private int currentRoomId;
    private string nickname = "";

    public ObservableCollection<RoomItem> Rooms { get; } = new();
    public ObservableCollection<ChatLine> ChatLines { get; } = new();

    public MainWindow()
    {
        InitializeComponent();
        DataContext = this;

        client.PacketReceived += OnPacketReceived;
        client.Disconnected += OnDisconnected;

        for (int roomId = 1; roomId <= 10; roomId++)
            Rooms.Add(new RoomItem(roomId, 0));
    }

    private async void LoginButton_Click(object sender, RoutedEventArgs e)
    {
        await LoginAsync();
    }

    private async void NicknameTextBox_KeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.Enter)
        {
            e.Handled = true;
            await LoginAsync();
        }
    }

    private async Task LoginAsync()
    {
        try
        {
            LoginButton.IsEnabled = false;
            LoginStatusTextBlock.Text = "Connecting...";

            if (!client.IsConnected)
                await client.ConnectAsync(ServerIp, ServerPort);

            LoginStatusTextBlock.Text = "Logging in...";
            await client.SendAsync(ChatProtocol.C_LOGIN_REQ, NicknameTextBox.Text.Trim());
        }
        catch (Exception ex)
        {
            LoginStatusTextBlock.Text = $"Login failed: {ex.Message}";
            LoginButton.IsEnabled = true;
        }
    }

    private async void RefreshRoomsButton_Click(object sender, RoutedEventArgs e)
    {
        await SendSafeAsync(ChatProtocol.C_ROOM_LIST_REQ, "");
    }

    private async void RoomListBox_MouseDoubleClick(object sender, MouseButtonEventArgs e)
    {
        await EnterSelectedRoomAsync();
    }

    private async Task EnterSelectedRoomAsync()
    {
        if (RoomListBox.SelectedItem is not RoomItem room)
            return;

        RoomListStatusTextBlock.Text = $"Entering room {room.RoomId}...";
        await SendSafeAsync(ChatProtocol.C_ROOM_ENTER_REQ, room.RoomId.ToString());
    }

    private async void SendButton_Click(object sender, RoutedEventArgs e)
    {
        await SendChatAsync();
    }

    private async void MessageTextBox_KeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.Enter)
        {
            e.Handled = true;
            await SendChatAsync();
        }
    }

    private async Task SendChatAsync()
    {
        string message = MessageTextBox.Text.Trim();
        if (message.Length == 0)
            return;

        await SendSafeAsync(ChatProtocol.C_CHAT_REQ, message);
        MessageTextBox.Clear();
    }

    private async void LeaveButton_Click(object sender, RoutedEventArgs e)
    {
        await SendSafeAsync(ChatProtocol.C_ROOM_LEAVE_REQ, "");
    }

    private async Task SendSafeAsync(ushort packetId, string payload)
    {
        try
        {
            if (!client.IsConnected)
            {
                ShowLoginView("Disconnected.");
                return;
            }

            await client.SendAsync(packetId, payload);
        }
        catch (Exception ex)
        {
            AddSystemLine($"Send failed: {ex.Message}");
        }
    }

    private void OnPacketReceived(ChatPacket packet)
    {
        Dispatcher.Invoke(() => HandlePacket(packet));
    }

    private void HandlePacket(ChatPacket packet)
    {
        switch (packet.Id)
        {
        case ChatProtocol.S_LOGIN_RES:
            HandleLoginRes(packet.Payload);
            break;
        case ChatProtocol.S_ROOM_LIST_RES:
            HandleRoomList(packet.Payload);
            break;
        case ChatProtocol.S_ROOM_ENTER_RES:
            HandleRoomEnterRes(packet.Payload);
            break;
        case ChatProtocol.S_ROOM_USER_JOIN:
            HandleRoomUserJoin(packet.Payload);
            break;
        case ChatProtocol.S_ROOM_LEAVE_RES:
            HandleRoomLeaveRes(packet.Payload);
            break;
        case ChatProtocol.S_ROOM_USER_LEAVE:
            HandleRoomUserLeave(packet.Payload);
            break;
        case ChatProtocol.S_CHAT:
            HandleChat(packet.Payload);
            break;
        case ChatProtocol.S_ERROR:
            AddSystemLine(packet.Payload);
            break;
        }

        ScrollChatToEnd();
    }

    private void HandleLoginRes(string payload)
    {
        string[] parts = payload.Split('|', 2);
        if (parts.Length >= 2 && parts[0] == "OK")
        {
            nickname = parts[1];
            LoginButton.IsEnabled = true;
            ShowRoomListView();
            RoomListStatusTextBlock.Text = $"{nickname}, choose a room.";
            return;
        }

        LoginButton.IsEnabled = true;
        LoginStatusTextBlock.Text = $"Login failed: {payload}";
    }

    private void HandleRoomList(string payload)
    {
        foreach (string item in payload.Split('|', StringSplitOptions.RemoveEmptyEntries))
        {
            string[] parts = item.Split(':', 2);
            if (parts.Length != 2)
                continue;

            if (int.TryParse(parts[0], out int roomId) && int.TryParse(parts[1], out int userCount))
                UpdateRoomCount(roomId, userCount);
        }
    }

    private void HandleRoomEnterRes(string payload)
    {
        string[] parts = payload.Split('|');
        if (parts.Length >= 3 && parts[0] == "OK" && int.TryParse(parts[1], out int roomId))
        {
            currentRoomId = roomId;
            ChatLines.Clear();
            CurrentRoomTextBlock.Text = $"Room {roomId}";
            ChatStatusTextBlock.Text = $"{nickname}";

            if (int.TryParse(parts[2], out int userCount))
                UpdateRoomCount(roomId, userCount);

            ShowChatRoomView();
            MessageTextBox.Focus();
            return;
        }

        RoomListStatusTextBlock.Text = $"Room enter failed: {payload}";
    }

    private void HandleRoomUserJoin(string payload)
    {
        string[] parts = payload.Split('|');
        if (parts.Length < 3)
            return;

        AddSystemLine($"{parts[0]} joined room {parts[1]}.");

        if (int.TryParse(parts[1], out int roomId) && int.TryParse(parts[2], out int userCount))
            UpdateRoomCount(roomId, userCount);
    }

    private void HandleRoomLeaveRes(string payload)
    {
        string[] parts = payload.Split('|');
        if (parts.Length >= 2 && parts[0] == "OK")
        {
            currentRoomId = 0;
            ChatLines.Clear();
            ShowRoomListView();
            RoomListStatusTextBlock.Text = "You are not in a room.";
            _ = SendSafeAsync(ChatProtocol.C_ROOM_LIST_REQ, "");
            return;
        }

        AddSystemLine($"Leave failed: {payload}");
    }

    private void HandleRoomUserLeave(string payload)
    {
        string[] parts = payload.Split('|');
        if (parts.Length < 3)
            return;

        AddSystemLine($"{parts[0]} left room {parts[1]}.");

        if (int.TryParse(parts[1], out int roomId) && int.TryParse(parts[2], out int userCount))
            UpdateRoomCount(roomId, userCount);
    }

    private void HandleChat(string payload)
    {
        string[] parts = payload.Split('|', 2);
        if (parts.Length == 2)
            ChatLines.Add(ChatLine.Chat(parts[0], parts[1], parts[0] == nickname));
    }

    private void ShowLoginView(string status)
    {
        currentRoomId = 0;
        LoginButton.IsEnabled = true;
        LoginStatusTextBlock.Text = status;
        LoginView.Visibility = Visibility.Visible;
        RoomListView.Visibility = Visibility.Collapsed;
        ChatRoomView.Visibility = Visibility.Collapsed;
    }

    private void ShowRoomListView()
    {
        LoginView.Visibility = Visibility.Collapsed;
        RoomListView.Visibility = Visibility.Visible;
        ChatRoomView.Visibility = Visibility.Collapsed;
    }

    private void ShowChatRoomView()
    {
        LoginView.Visibility = Visibility.Collapsed;
        RoomListView.Visibility = Visibility.Collapsed;
        ChatRoomView.Visibility = Visibility.Visible;
    }

    private void UpdateRoomCount(int roomId, int userCount)
    {
        RoomItem? room = Rooms.FirstOrDefault(x => x.RoomId == roomId);
        if (room != null)
            room.UserCount = userCount;
    }

    private void AddSystemLine(string message)
    {
        if (currentRoomId != 0)
            ChatLines.Add(ChatLine.System(message));
    }

    private void ScrollChatToEnd()
    {
        if (ChatLines.Count > 0)
            ChatListBox.ScrollIntoView(ChatLines[^1]);
    }

    private void OnDisconnected(string reason)
    {
        Dispatcher.Invoke(() =>
        {
            ChatLines.Clear();
            ShowLoginView($"Disconnected: {reason}");
        });
    }

    protected override async void OnClosed(EventArgs e)
    {
        await client.DisposeAsync();
        base.OnClosed(e);
    }
}

public sealed class RoomItem : INotifyPropertyChanged
{
    private int userCount;

    public RoomItem(int roomId, int userCount)
    {
        RoomId = roomId;
        this.userCount = userCount;
    }

    public int RoomId { get; }

    public string Title => $"Room {RoomId}";

    public int UserCount
    {
        get => userCount;
        set
        {
            if (userCount == value)
                return;

            userCount = value;
            OnPropertyChanged();
            OnPropertyChanged(nameof(UserCountText));
        }
    }

    public string UserCountText => $"{UserCount} user(s)";

    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}

public sealed record ChatLine(
    string Title,
    string Message,
    HorizontalAlignment Alignment,
    Brush BubbleBrush,
    Brush TitleBrush,
    Brush MessageBrush)
{
    public static ChatLine System(string message) =>
        new("Notice", message, HorizontalAlignment.Center, Brushes.Transparent, Brushes.SlateGray, Brushes.SlateGray);

    public static ChatLine Chat(string nickname, string message, bool mine) =>
        mine
            ? new(nickname, message, HorizontalAlignment.Right, new SolidColorBrush(Color.FromRgb(37, 99, 235)), Brushes.White, Brushes.White)
            : new(nickname, message, HorizontalAlignment.Left, Brushes.White, Brushes.SeaGreen, Brushes.Black);
}
