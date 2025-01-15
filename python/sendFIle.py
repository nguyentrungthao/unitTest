import socket
import tkinter as tk
from tkinter import filedialog, messagebox
import threading

ESP32_IP = "192.168.137.6"  # Địa chỉ IP của ESP32
ESP32_PORT = 12345           # Cổng của ESP32
firmware_file = None         # Biến toàn cục lưu đường dẫn file firmware

def select_file():
    global firmware_file
    firmware_file = filedialog.askopenfilename(filetypes=[("Binary Files", "*.bin")])
    if firmware_file:
        label_file.config(text=f"File: {firmware_file}")
    else:
        label_file.config(text="No file selected")

def connect_to_esp32(self):
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

def send_firmware():
    global firmware_file
    if not firmware_file:
        messagebox.showerror("Error", "No firmware file selected!")
        return

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
            client.connect((ESP32_IP, ESP32_PORT))
            print("Connected to ESP32")

            with open(firmware_file, "rb") as firmware:
                while True:
                    chunk = firmware.read(1024)
                    if not chunk:
                        break
                    client.sendall(chunk)
                    print(f"Sent {len(chunk)} bytes")

            messagebox.showinfo("Success", "Firmware sent successfully!")
    except Exception as e:
        messagebox.showerror("Error", f"Failed to send firmware: {e}")

# Tạo giao diện Tkinter
root = tk.Tk()
root.title("ESP32 Firmware Update")

# Nhãn hiển thị đường dẫn file
label_file = tk.Label(root, text="No file selected", wraplength=400, anchor="w")
label_file.pack(pady=10, padx=10)

# Nút chọn file
btn_select_file = tk.Button(root, text="Select Firmware File", command=select_file)
btn_select_file.pack(pady=5)

# Nút kết nối
btn_connect = tk.Button(root, text="Connect to ESP32", command=connect_to_esp32)
btn_connect.pack(pady=5)

# Nút gửi firmware
btn_send_firmware = tk.Button(root, text="Send Firmware", command=send_firmware)
btn_send_firmware.pack(pady=5)

# Chạy vòng lặp giao diện
root.mainloop()
