import tkinter as tk
from tkinter import scrolledtext
import socket
import threading

def receive_full_message(client_socket):
    # Khởi tạo một biến để chứa dữ liệu đã nhận
    full_message = ''
    
    while True:
        # Nhận dữ liệu với kích thước tối đa là 1024 byte
        chunk = client_socket.recv(1024).decode()
        
        # Nếu không có dữ liệu (client ngắt kết nối), thoát khỏi vòng lặp
        if not chunk:
            break
        
        # Thêm phần dữ liệu nhận được vào chuỗi đầy đủ
        full_message += chunk
        
        # Kiểm tra nếu chuỗi đã hoàn thành, có thể thêm một dấu hiệu kết thúc (tuỳ ý)
        if "END_OF_MESSAGE" in full_message:  # Ví dụ nếu bạn dùng một từ khóa kết thúc
            break
    
    return full_message

class SocketServerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Socket Server")
        self.root.geometry("400x300")

        # Biến quản lý socket
        self.server_socket = None
        self.client_socket = None
        self.client_address = None
        self.running = False

        # Khung nhập cổng
        tk.Label(root, text="Server Port:").pack()
        self.port_entry = tk.Entry(root)
        self.port_entry.pack()
        self.port_entry.insert(1, "12345")  # Cổng mặc định

        # Nút bắt đầu server
        self.start_button = tk.Button(root, text="Bắt đầu Server", command=self.start_server)
        self.start_button.pack(pady=5)

        # Khung log tin nhắn
        self.chat_log = scrolledtext.ScrolledText(root, state="disabled", width=50, height=10)
        self.chat_log.pack(pady=5)

        # Khung nhập tin nhắn
        self.msg_entry = tk.Entry(root)
        self.msg_entry.pack(fill="x", pady=5)
        self.msg_entry.bind("<Return>", lambda event: self.send_message())  # Gửi tin nhắn khi nhấn Enter

        # Nút gửi tin nhắn
        self.send_button = tk.Button(root, text="Gửi", command=self.send_message)
        self.send_button.pack(pady=5)

        # Nút dừng server
        self.stop_button = tk.Button(root, text="Dừng Server", command=self.stop_server, state="disabled")
        self.stop_button.pack(pady=5)

    def start_server(self):
        # Lấy cổng từ người dùng
        port = int(self.port_entry.get())
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.server_socket.bind(("0.0.0.0", port))
            self.server_socket.listen(1)
            self.update_chat_log(f"Server đang lắng nghe trên cổng {port}...")
            self.running = True

            # Vô hiệu hóa nút "Bắt đầu Server"
            self.start_button.config(state="disabled")
            self.stop_button.config(state="normal")

            # Bắt đầu luồng chấp nhận kết nối
            threading.Thread(target=self.accept_client, daemon=True).start()
        except Exception as e:
            self.update_chat_log(f"Lỗi khi khởi động server: {e}")

    def accept_client(self):
        try:
            self.client_socket, self.client_address = self.server_socket.accept()
            self.update_chat_log(f"Kết nối từ: {self.client_address}")
            # Bắt đầu luồng nhận tin nhắn từ client
            threading.Thread(target=self.receive_messages, daemon=True).start()
        except Exception as e:
            self.update_chat_log(f"Lỗi khi chấp nhận kết nối: {e}")

    def receive_messages(self):
        while self.running:
            try:
                message = self.client_socket.recv(1024).decode()
                if message:
                    self.update_chat_log(f"Client: {message}")
                else:
                    self.update_chat_log("Client đã ngắt kết nối.")
                    self.running = False
                    break
            except Exception:
                break

    def send_message(self):
        if self.client_socket:
            message = self.msg_entry.get()
            if message:
                try:
                    self.client_socket.send(message.encode())
                    self.update_chat_log(f"Bạn: {message}")
                    self.msg_entry.delete(0, tk.END)  # Xóa ô nhập tin nhắn
                except Exception as e:
                    self.update_chat_log(f"Lỗi gửi tin nhắn: {e}")
        else:
            self.update_chat_log("Chưa có client kết nối!")

    def stop_server(self):
        self.running = False
        if self.client_socket:
            try:
                self.client_socket.close()
            except Exception:
                pass
        if self.server_socket:
            try:
                self.server_socket.close()
            except Exception:
                pass
        self.update_chat_log("Server đã dừng.")
        self.start_button.config(state="normal")
        self.stop_button.config(state="disabled")

    def update_chat_log(self, message):
        self.chat_log.config(state="normal")
        self.chat_log.insert(tk.END, f"{message}\n")
        self.chat_log.yview(tk.END)  # Cuộn xuống cuối log
        self.chat_log.config(state="disabled")

# Chạy ứng dụng
if __name__ == "__main__":
    root = tk.Tk()
    app = SocketServerApp(root)
    root.mainloop()
