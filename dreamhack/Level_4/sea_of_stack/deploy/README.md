# Write-up sea of stack challenge - Dreamhack
### 1. Checksec & Recon
   * `checksec` :
<img width="847" height="253" alt="image" src="https://github.com/user-attachments/assets/0c5300aa-6465-4cb9-b3e3-190a68dc5e4e" />

   * `Dockerfile`
<img width="1178" height="529" alt="image" src="https://github.com/user-attachments/assets/de621d11-6662-4721-8e0b-53a02d866c0a" />

   * Lấy `Libc` của `server` trong `Dockerfile`
   * Build docker : `docker build -t sea_of_stack .`
   * Run dockerfile : `docker run -d -p 31337:31337 sea_of_stack`
   * Netcat -> port = 31337 : `nc localhost 31337`
   * Lấy pid của `prob` đang chạy trong docker : `ps aux`
   * DEBUG đính kèm tiến trình : `gdb -p <pid>`
   * `Vmmap` Lấy path của Libc, Ld, các file liên quan và copy về thư mục deploy : `docker cp <Container ID>:<Path> deploy/`
   * Patch libc, ld, .. vào challenge
### 2. Reverse Engineering
<img width="908" height="535" alt="image" src="https://github.com/user-attachments/assets/84eb0030-f589-4f89-bd55-afcc40257c52" />
