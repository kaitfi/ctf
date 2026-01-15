# Write-up xrop challenge - Dreamhack 
### 1. Checksec & Recon
   * Author cho ta file zip. Giải nén ra ta được `Dockerfile` và thư mục `Deploy`
   * Challenge ở trong `Deploy`

**Checksec :**

<img width="706" height="233" alt="image" src="https://github.com/user-attachments/assets/1c88633e-13ec-497e-8524-d3f3636e161f" />

**Dockerfile :**

<img width="1185" height="461" alt="image" src="https://github.com/user-attachments/assets/f5336834-73d1-4b23-ad86-f9fb4b66c64d" />

   * Lấy `Libc` của `server` trong `Dockerfile`
   * Build docker : `docker build -t xrop .`
   * Run dockerfile : `docker run -d -p 8080:8080 xrop`
   * Netcat -> port = 8080 : `nc localhost 8080`
   * Lấy pid của `prob` đang chạy trong docker : `ps aux`
   * DEBUG đính kèm tiến trình : `gdb -p <pid>`
   * `Vmmap` để lấy path của Libc, Ld, các file liên quan và copy về thư mục deploy : `docker cp <Container ID>:<Path> deploy/`
   * Patch libc, ld, .. vào challenge

### 2. Reverse Engineering

<img width="859" height="411" alt="image" src="https://github.com/user-attachments/assets/d2edd942-c37a-47ba-9c7e-e2a2688e3934" />

   * Có thể thấy chương trình bị BOF và có `printf` với `format` là `%s` và `RSI = buf`
   * Mục tiêu của ta sẽ là `leak libc`, `canary` và `ret2libc`
   * Payload của ta sẽ bị `encode` theo cách `buf[i - 1] ^= buf[i]`
   * `exit` để thoát

### 3. Exploit
   * Để `leak libc` và `canary` ta chỉ cần `padding` vào các bytes không trùng lặp để không bị `set NULL` đến địa chỉ `stack` có chứa `libc` và `canary`
   * Để payload `ret2libc` hoạt động ta cần `xor` ngược các bytes theo chuỗi từ cuối về đầu của `payload` và truyền `payload` mới `encode` xong vào `buf` => `get_shell`
   * [SCRIPT](./exploit.py)   
