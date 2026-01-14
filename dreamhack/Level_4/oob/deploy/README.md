# Write-Up oob challenge - Dreamhack
### 1. Checksec & Recon

<img width="604" height="113" alt="image" src="https://github.com/user-attachments/assets/bec60d72-e65f-441d-b468-0b61f87869fd" />

<img width="584" height="101" alt="image" src="https://github.com/user-attachments/assets/13c85607-676d-4047-ab7e-d50219a9624f" />

   * Author cho ta file zip. Giải nén ra ta được 1 file docker và thư mục deploy. Challenge và Test Flag ở trong thư mục deploy
   * Sử dụng `checksec` để kiểm tra file `oob`:

<img width="809" height="225" alt="image" src="https://github.com/user-attachments/assets/f96df207-7e56-4a1c-989f-615fcd794d35" />

   * Challenge được bật Full Security, nhắm gây khó khăn cho attackers
   * Full RElRO : Không thể Overwrite Got
   * NX enable: Không thể ghi shellcode lên stack
   * PIE enable : Exe address ngẫu nhiên hóa
   * Stripped : NO => Dễ dàng hơn cho việc Reverse Engineering

Author đã cho ta Dockerfile. Ta sẽ lấy libc từ đó:

<img width="1178" height="700" alt="image" src="https://github.com/user-attachments/assets/8cef258c-380a-4300-8fe9-bdf2afc10855" />

   * docker build -t oob .

Sau khi build ta run docker:
   * docker run -d -p 9090:9090 oob

Ta netcat đến port 9090: 
   * nc localhost 9090

Sau khi netcat, tiến trình challenge đã được khởi động . Ta sử dụng lệnh `ps aux` để lấy pid của chall oob đang chạy trong Docker

GDB với tiến trình vừa lấy được `gdb -p <pid>` và vmmap đê lấy path của libc và ld

Sau đó lấy ID container của tiến tiến trình hiện tại và copy vào thư mục deploy `docker cp <ID container>:<libc path> deploy/`

Thực hiện patch libc->challenge với pwinit

### 2. Reverse Engineering

<img width="580" height="668" alt="image" src="https://github.com/user-attachments/assets/f809717a-f894-46e7-bb3d-ecb69ebea15b" />

<img width="269" height="129" alt="image" src="https://github.com/user-attachments/assets/7334efec-85f1-4190-aa4f-bc73927749f9" />

Chương trình cho ta 3 options gốm `read`, `write` và `exit` ứng các với choice là `1`, `2` và `3` 

Với Option 1 là `read` chương trình cho phép ta nhập giá trị offset vào biến _QWORD `v5[0]` và Đọc giá trị tại `oob + offset`. 
Dễ để nhận ra rằng chương trình bị `Out of bound` do biến `v5` được khai báo sai kiểu và format `lld` cho phép số dec 8 bytes có dấu. 
Do đó ta có `AAR` nên có thể leak `Libc address` và `Exe address`.

  * Cả `Libc address` và `Exe address` đều gần array `oob` :
<img width="1080" height="196" alt="image" src="https://github.com/user-attachments/assets/72dcae36-ebbc-4ce6-88de-fbec4e53d60c" />


Note for Stage Leak Libc:

  * Mảng oob ở printf là kiểu char do đó với mỗi idx nó sẽ nhảy đúng 1 bytes và format của printf là `%c`do đó ta chỉ có thể in từng bytes.

**Option 2:**
  * Đối với Option 2 là `Write` chương trình cũng cho ta nhập offset vào biến `v5[0]` nhưng thay vào đó chương trình lại cho ta 8 bytes tại `oob + offset`
  * Vậy ta có quyền AAW vào bất kỳ địa chỉ chỉ nào nếu ta leak được địa chỉ đó `offset = AAW - oob`
  * Do ta có AAW nên cũng sẽ có nhiều cách khai thác như `__run_exit_handler (Initial & TLS & Debug trong Docker và tìm offset Ld base trên server)`, Leak stack để `ret2libc` qua `__libc_environ`, `FSOP` (sẽ khá phức tạp khi chương trình chỉ cho ta nhập 8 bytes tưng lần 1), `GOT Libc overwrite(RELRO_libc = Partial)`... etc,
  * Do đó ở đây ta sẽ chọn leak stack qua `__libc_environ` (vì nó đơn giản nhất -_-) (Người đọc cũng có thể thử khai thác `__run_exit_handler` nó sẽ không phức tạp hơn `__libc_environ` nhiều đâu)

**Option 3:**
  * Break ra khỏi vòng lặp và return 0; (đồng thỡi cũng thực hiên hàm exit(0) điều kiện để khai thác `__run_exit_handlers`)
  * Do đó Main sẽ return 0 => Target của ta là `Saved rip` của `Main`

 # 3. Exploit
  * Quay trở về với bước leak `Libc address` & `Exe address`
  * Do ta đã có `Libc Base` & `Exe address` nên để leak stack qua `__libc_environ` nên ta sẽ input vào option `1` là `read` với `offset` = `__libc_environ` - `oob` là leak được stack
  * Khi đã có `Saved rip` của `main` rồi ta chỉ cần input vào option `2` là `write` với `offset` = `Saved rip` - `oob` để `ret2libc`

[SCRIPT](./exploit.py)
