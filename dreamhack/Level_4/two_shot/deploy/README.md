# Write-up two-shot challenge - Dreamhack

### 1. Checksec & Recon 

<img width="746" height="132" alt="image" src="https://github.com/user-attachments/assets/fde09ea9-9b9f-4066-8eb8-8aacbc0211fc" />
<img width="745" height="172" alt="image" src="https://github.com/user-attachments/assets/26c54c12-d1f8-4a45-bd0f-69a10e6e7704" />

   * Author cho ta file zip. Giải nén ra ta được 1 dockerfile và thư mục deploy. Challenge và Test Flag ở trong thư mục deploy
   * Sử dụng `checksec` để kiểm tra file `prob` và `libc.so.6`:

<img width="813" height="212" alt="image" src="https://github.com/user-attachments/assets/968dedab-9647-43c3-bbf8-b33e3af7dd2d" />

<img width="864" height="202" alt="image" src="https://github.com/user-attachments/assets/3dab1786-2ca3-4d60-b436-e7f96070fea8" />

**Dockerfile :**

<img width="1172" height="512" alt="image" src="https://github.com/user-attachments/assets/b610e4b3-bdb0-44c3-89b7-da034f786fc1" />

   * Lấy `Libc` của `server` trong `Dockerfile`
   * Build docker : `docker build -t two_shot .`
   * Run dockerfile : `docker run -d -p 9090:9090 two_shot`
   * Netcat -> port = 9090 : `nc localhost 9090`
   * Lấy pid của `prob` đang chạy trong docker : `ps aux`
   * DEBUG đính kèm tiến trình : `gdb -p <pid>`
   * Lấy path của Libc, Ld, các file liên quan và copy về thư mục deploy : `docker cp <Container ID>:<Path> deploy/`
   * Patch libc, ld, .. vào challenge
### 2. Reverse Engineering 
``` text
unsigned __int64 __fastcall main(int a1, char **a2, char **a3)
{
  _QWORD *v3; // rbx
  signed __int64 v4; // rax
  signed __int64 v5; // rax
  signed __int64 v6; // rax
  signed __int64 v7; // rax
  signed __int64 v8; // rax
  char *v9; // rdi
  signed __int64 v10; // rax
  signed __int64 v11; // rax
  signed __int64 v12; // rax
  unsigned int v13; // r9d
  signed __int64 v14; // rax
  unsigned int v15; // r8d
  signed __int64 v16; // rax
  unsigned int v17; // r9d
  signed __int64 v18; // rax
  unsigned int v19; // r8d
  signed __int64 v20; // rax
  signed __int64 v21; // rax
  signed __int64 v22; // rax
  unsigned int v23; // r8d
  unsigned int v24; // edi
  signed __int64 v25; // rax
  size_t v26; // r9
  signed __int64 v27; // rax
  size_t v28; // r8
  signed __int64 v29; // rax
  signed __int64 v30; // rax
  signed __int64 v31; // rax
  char v33[2]; // [rsp+6h] [rbp-52h] BYREF
  char *v34; // [rsp+8h] [rbp-50h] BYREF
  char buf[8]; // [rsp+10h] [rbp-48h] BYREF
  unsigned __int64 v36; // [rsp+18h] [rbp-40h]

  v36 = __readfsqword(0x28u);
  *(_QWORD *)buf = 0;
  *(_WORD *)v33 = 0;
  v3 = (_QWORD *)sys_mmap(0, 8u, 3u, 0x22u, 0xFFFFFFFFFFFFFFFFLL, 0);
  *v3 = 0;
  v4 = sys_write(1u, "Welcome to WarGame!\n", 0x14u);
  while ( 1 )
  {
    v5 = sys_write(1u, "1. WRITE\n", 9u);
    v6 = sys_write(1u, "2. READ\n", 8u);
    v7 = sys_write(1u, "Select your option: ", 0x14u);
    v8 = sys_read(0, buf, 7u);
    v9 = strtok(buf, "\n");
    if ( v9 )
    {
      if ( !strcasecmp(v9, "exit") )
        break;
    }
    if ( *v3 == 2 )
    {
      v30 = sys_write(1u, "bye bye!\n", 9u);
      break;
    }
    if ( buf[0] == 49 )
    {
      v34 = 0;
      v11 = sys_write(1u, "Enter the Read Size: ", 0x15u);
      v12 = sys_read(0, v33, 2u);
      v14 = sys_write(v13, "Enter your Address: ", 0x14u);
      v16 = sys_read(v15, (char *)&v34, 8u);
      v18 = sys_write(v17, "Input your data: ", 0x11u);
      v20 = sys_read(v19, v34, *(unsigned __int16 *)v33);
      ++*v3;
    }
    else if ( buf[0] == 50 )
    {
      v34 = 0;
      v21 = sys_write(1u, "Enter the memory area you want to read: ", 0x28u);
      v22 = sys_read(0, (char *)&v34, 8u);
      v24 = v23;
      v25 = sys_write(v23, ": ", 2u);
      v27 = sys_write(v24, v34, v26);
      v29 = sys_write(v24, "\n", v28);
      ++*v3;
    }
    else
    {
      v10 = sys_write(1u, "Invalid option!\n", 0x10u);
    }
  }
  v31 = sys_exit(0);
  return v36 - __readfsqword(0x28u);
}
```
   * Chương trình khá đơn giản, có 2 option là `write` và `read` tương ứng là `1` và `2`
   * Cả hai option đều cho ta quyền đọc và ghi vào bất kỳ `address` nếu ta leak được
### 3. Exploit 

   * Dựa vào `AAR` và `PIE tắt`. Ta có thể leak địa chỉ libc một cách dễ dàng
   * Sau khi leak được địa chỉ libc và ta có quyền `AAW`, `AAR`
   * Ta có khả năng thực hiện các khai thác sau : leak stack qua libc `__libc_environ => ret2libc` & `__run_exit_handlers` & `FSOP` &  `GOT Libc overwrite`
   * Nhìn vào đoạn code trên ta đã loại bỏ được các phương án khai thác không khả thi là `__libc_environ => ret2libc`(sys_exit(0) trước khi return),  `__run_exit_handlers`(sys_exit(0) chứ không phải exit(0)),  `FSOP`(không có trigger FSOP)
   * Vậy ta còn lại 1 phương án khả thi là `GOT libc overwrite`
   * Quay ngược lại đoạn code bên trên ta thấy rằng chương trình có gọi strtok(buf, "\n") với `RDI` là `buf` nên ta có thể kiểm soát và truyền vào tham số `/bin/sh`
   * Overwrite GOT của Libc hàm `strtok` bằng `system` :

<img width="1067" height="163" alt="image" src="https://github.com/user-attachments/assets/b1a3a326-a6fd-49d2-8f2e-9a1714215bd9" />

<img width="1075" height="602" alt="image" src="https://github.com/user-attachments/assets/d039cd58-003c-401b-a069-95dc0845113f" />

   * `call   0x7ffff7dbb410 <*ABS*+0xa8b80@plt>` tại `strtok_r + 39`

<img width="1574" height="104" alt="image" src="https://github.com/user-attachments/assets/5508e937-b6ce-479b-9117-7824d11ae7d8" />

   * Chương trình sẽ `jmp` đến `0x7ffff7fac058 <*ABS*@got.plt>` với `RDI` là `buf` của ta và `0x7ffff7fac058` là địa chỉ có quyền write

<img width="1298" height="189" alt="image" src="https://github.com/user-attachments/assets/411029fe-72b1-4a4b-bfaf-9c76d3b393e3" />

   * `0x7ffff7fac058 <*ABS*@got.plt>` là mục tiêu của ta

<img width="940" height="205" alt="image" src="https://github.com/user-attachments/assets/ab3ffce9-b61e-4ba7-9f37-9a0d7328bc27" />

<img width="955" height="482" alt="image" src="https://github.com/user-attachments/assets/df429b9c-2f7d-4639-9b55-edb1087f5dcf" />

<img width="1177" height="192" alt="image" src="https://github.com/user-attachments/assets/3cbe9d62-d64b-4c75-a5c9-5e906013ea71" />


[SCRIPT](./exploit.py)
