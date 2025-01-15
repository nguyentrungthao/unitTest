import tkinter as tk
from tkinter import scrolledtext
import socket
import threading


class SocketServer:
    def __init__(self):
        self.server_socket = None
        self.client_socket = None
        self.client_address = None
        self.running = False
        self.ui = None  # Được gán sau khi khởi tạo UI

    def start_server(self, port):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.server_socket.bind(("0.0.0.0", port))
            self.server_socket.listen(1)
            self.running = True
            self.ui.update_chat_log(f"Server đang lắng nghe trên cổng {port}...")
            threading.Thread(target=self.accept_client, daemon=True).start()
        except Exception as e:
            self.ui.update_chat_log(f"Lỗi khi khởi động server: {e}")

    def accept_client(self):
        try:
            self.client_socket, self.client_address = self.server_socket.accept()
            self.ui.update_chat_log(f"Kết nối từ: {self.client_address}")
            threading.Thread(target=self.receive_messages, daemon=True).start()
        except Exception as e:
            self.ui.update_chat_log(f"Lỗi khi chấp nhận kết nối: {e}")

    def receive_messages(self):
        while self.running:
            try:
                message = self.client_socket.recv(1024).decode()
                if message:
                    self.ui.update_chat_log(f"Client: {message}")
                else:
                    self.ui.update_chat_log("Client đã ngắt kết nối.")
                    self.running = False
                    break
            except Exception:
                break

    def send_message(self, message):
        if self.client_socket:
            try:
                self.client_socket.send(message.encode())
                self.ui.update_chat_log(f"Bạn: {message}")
            except Exception as e:
                self.ui.update_chat_log(f"Lỗi gửi tin nhắn: {e}")
        else:
            self.ui.update_chat_log("Chưa có client kết nối!")

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
        self.ui.update_chat_log("Server đã dừng.")


class ServerUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Socket Server")
        self.root.geometry("500x500")

        self.server = SocketServer()
        self.server.ui = self  # Gán UI vào server

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
        self.msg_entry.bind("<Return>", lambda event: self.send_message())

        # Nút gửi tin nhắn
        self.send_button = tk.Button(root, text="Gửi", command=self.send_message)
        self.send_button.pack(pady=5)

        # Nút dừng server
        self.stop_button = tk.Button(root, text="Dừng Server", command=self.stop_server, state="disabled")
        self.stop_button.pack(pady=5)

    def start_server(self):
        port = int(self.port_entry.get())
        self.server.start_server(port)
        self.start_button.config(state="disabled")
        self.stop_button.config(state="normal")

    def send_message(self):
        message = self.msg_entry.get()
        if message:
            self.server.send_message(message)
            self.msg_entry.delete(0, tk.END)

    def stop_server(self):
        self.server.stop_server()
        self.start_button.config(state="normal")
        self.stop_button.config(state="disabled")

    def update_chat_log(self, message):
        self.chat_log.config(state="normal")
        self.chat_log.insert(tk.END, f"{message}\n")
        self.chat_log.yview(tk.END)
        self.chat_log.config(state="disabled")


# Chạy ứng dụng
if __name__ == "__main__":
    root = tk.Tk()
    app = ServerUI(root)
    root.mainloop()
