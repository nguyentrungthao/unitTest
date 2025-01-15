import socket
import tkinter as tk

# Tạo cửa sổ chính
root = tk.Tk()
root.title("Ứng dụng đầu tiên")
root.geometry("300x200")
# Thêm nhãn
label = tk.Label(root, text="Xin chào, đây là giao diện Tkinter!")
label.pack()

# Thêm nút
button = tk.Button(root, text="Nhấn tôi!", command=lambda: print("Bạn đã nhấn nút!"))
button.pack()
# Đặt địa chỉ IP và cổng cho server
HOST = '192.168.137.5'  # Địa chỉ IP Wi-Fi hoặc Ethernet của máy tính
PORT = 12345            # Cổng bất kỳ (12345 là ví dụ)

# Tạo socket TCP
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((HOST, PORT))  # Gắn socket với địa chỉ IP và cổng
server_socket.listen(5)           # Lắng nghe tối đa 5 kết nối

print(f"Server đang chạy tại {HOST}:{PORT}...")

try:
    while True:
        # Chấp nhận kết nối từ client
        client_socket, client_address = server_socket.accept()
        print(f"Kết nối từ: {client_address}")

        # Nhận dữ liệu từ client
        data = client_socket.recv(1024).decode('utf-8')
        print(f"Dữ liệu nhận được: {data}")

        # Phản hồi lại client
        client_socket.send("Dữ liệu đã nhận!".encode('utf-8'))

        # Đóng kết nối
        client_socket.close()

except KeyboardInterrupt:
    print("Đang tắt server...")
    server_socket.close()
