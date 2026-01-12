# Write-up Pwn 1 - VCS CTF Red 2025

### 1. Check Security & Recon
Author cho 1 file zip gồm:

<img width="632" height="156" alt="image" src="https://github.com/user-attachments/assets/02109cd4-2b6e-4c20-9852-0ca27cfe9837" />


Sử dụng `checksec` để kiểm tra file `chall`:
```text
[*] '/mnt/d/ctf/pwn/training/vcs-ctf-red-2025/pwn2/pwn1/chall'
Arch:     amd64-64-little
RELRO:    Full RELRO
Stack:    Canary found
NX:       NX enabled
PIE:      PIE enabled
SHSTK:    Enabled
IBT:      Enabled
```
Full Security => Chương trình đã tăng các lớp bảo mật lên mức tối đa để ngăn chặn người có thể pwn
### 2. Reverse Engineering
<img width="844" height="588" alt="image" src="https://github.com/user-attachments/assets/58b26f52-5e4d-41c1-9bfa-7ed997eb930c" /> 

Structure: 

<img width="413" height="199" alt="image" src="https://github.com/user-attachments/assets/6237bf89-cfda-49f4-bcfc-399f3d4e9560" />

**Add_user()**
```text
unsigned __int64 add_user()
{
  user *user; // rbx
  int i; // [rsp+8h] [rbp-128h]
  int idx; // [rsp+Ch] [rbp-124h]
  char s[264]; // [rsp+10h] [rbp-120h] BYREF
  unsigned __int64 v5; // [rsp+118h] [rbp-18h]

  v5 = __readfsqword(0x28u);
  idx = -1;
  for ( i = 0; i <= 9; ++i )
  {
    if ( !::user[i] )
    {
      idx = i;                             
      break;
    }
  }
  if ( idx == -1 )
  {
    puts("User limit reached");
  }
  else
  {
    ::user[idx] = (user *)malloc(0x68u);
    if ( !::user[idx] )
    {
      puts("Malloc failed");
      exit(-1);
    }
    printf("Name: ");
    read_data(::user[idx], 0x20u);
    printf("Address: ");
    read_data(::user[idx]->Address, 0x40u);
    printf("Bio: ");
    read_data(s, 0x100u);
    user = ::user[idx];
    user->Bio = (__int64 *)strdup(s);
    printf("User added at index %d\n", idx);
  }
  return v5 - __readfsqword(0x28u);
}
```
    - Kiểm tra user[idx] (idx : 0 -> 9) để khởi tạo
    - Malloc(0x68) vào idx chưa được khởi tạo -> size đã cố định => không có malloc unlimited size ở đây
    - Nhập dữ liệu vào các field Name & Address => không có heap overflow ở đây
    - Cuối cùng chương trình cho phép nhập 0x100 bytes vào biến s sau đó sẽ gán user[idx]-_Bio = strdup(s)
    - Strdup(s) sẽ ptr = malloc(size = strlen(s) + 1) & copy buffer s->ptr
**Edit_bio()**

<img width="720" height="274" alt="image" src="https://github.com/user-attachments/assets/fbc77986-3175-4af9-afe2-67daaf585f7c" />

    - Chương trình in ra data của Bio hiện tại trước khi update Bio
    - Ta có thể thấy chương trình cho nhập cố định 0x100 bytes vào User->Bio.
    => Heap overflow do ở add_user strdup(s) sẽ malloc(size = strlen(s) + 1) 
        -> ta có thể malloc size nhỏ hơn và overwrite fd của next chunk
**Delete_user()**

<img width="566" height="265" alt="image" src="https://github.com/user-attachments/assets/33a87540-173f-4a3b-ba5f-63fc490f916f" />

    - Chương trình free(User->Bio) và free(user[idx]) và đặt lại user[idx] = 0 => Không có UAF
**Print_user()**

<img width="655" height="250" alt="image" src="https://github.com/user-attachments/assets/bfc18213-11c9-4b28-b0f2-41c381797f79" />

    _ In các info hiện tại
**Tổng kết :**
    - Ta có heap overflow -> Có thể overwrite fd của next chunk để tcache poisioning => Có thể đạt được AAW
### 3. Exploit
**Stage 1: Leak Heap Base & Libc Base**

**Leak Heap Base:**

    - Ở hàm add_user ta có thể fill field Names & Address để leak Addr Heap của User->Bio => Heap Base 
    
**Leak Libc Base:**

    - Dùng strdup(s) để malloc ra 7 chunk size 0x100 sau đó tiếp 3 chunk là A, B, C:
    - Sau đó free 7 chunk đầu tiên size = 0x100 để fill tcache
        Heap Structure:
        0x20  | user[A]->Bio (overflow xuống để leak libc)s
        0x70  | user[B]
        0x100 | user[B]->Bio (into unsorted bin)
        0x70  | user[C] (Ngăn Consolidation)
        0x100 | user[C]->Bio (into unsorted bin)
    Flow: Free(User[B]->Bio) -> Free(User[B]) -> Free(User[C]->Bio) 
    Free(User[B]->Bio) thì nó đã gắn cho next chunk của prev_bit (prev_inuse) của next chunk = 0 
    Khi Free(User[C]->Bio) thì sẽ kiểm tra các prev_bit của các chunk liền kể trước User[C]->Bio để quyết định hợp nhất
    Nếu prev_bit = 1(Tức đang in use) thì sẽ không hợp nhất và ngược lại
    Mục đích làm cho chunk User[B]->Bio hợp nhất lùi với User[B] và main arena được khi vào FD và BK của user[B]
    Dễ dàng leak hơn khi không phải overflow dài và quan trọng không phải ghi đè lên nhiều dữ liệu quan trọng tránh crash chương trình

**Stage 2: FSOP**

    Sau khi leak được Libc Base lợi dụng strdup(s) tạo 3 chunk để tcache poisioning => AAW
        0x100 | User[A]->Bio (Ghi payload)
        0x70  | User[B]
        0x20  | User[B]->Bio (Oveflow xuống C)
        0x70  | User[C] 
        0x20  | User[C]->Bio
        0x70  | User[D]
        0x20  | User[D]->Bio
    Nhưng do ta đã chunk size = 0x70 đã trong tcache nên khi malloc(0x68) nó chỉ lấy trong tcache nên các user[idx] sẽ nằm trước các user[idx]->Bio
    Và các User[idx]->Bio sẽ nằm liền kề nhau do cung lấy ra từ unsorted bin trước đó đã hợp nhất
    Nên ta có thể tcache poisioning dễ dàng hơn
    Flow : Free(User[D]->Bio) -> Free(User[idx]-Bio) .Sau đó Overflow từ User[B]->Bio xuống fd của User[C]->Bio với target = _IO_list_all 
    Malloc 2 lần để Sử dụng AAW overwrite _IO_list_all = User[A]->Bio

**Payload:**

``` text
pld = b'\x01\x01;sh;\x00\x00'
pld = pld.ljust(0x28, b'\x00') + p64(1) #write_ptr > write_base
pld = pld.ljust(0x88, b'\x00') + p64(libc.sym._IO_2_1_stdout_ - 0x50)
pld = pld.ljust(0xa0, b'\x00') + p64(heap_base + 0xcf8 - 0xe0)
pld = pld.ljust(0xd8, b'\x00') + p64(libc.sym._IO_wfile_jumps)
pld += p64(libc.sym.system) + p64(heap_base + 0xcf0 - 0x68)
```
<img width="761" height="378" alt="image" src="https://github.com/user-attachments/assets/dd764481-3c95-45b0-b0c6-e072d11e054e" />

    Some note for payload: 
        Do ở nhánh điều kiện if có Short-circuit Evaluation (đánh giá ngắn mạch). Ta cần cần điều kiện đầu tiên đúng để vào _IO_OVERFLOW
        Ta sẽ chọn nhánh đầu tiên trong toán tử || . Ta chỉ cần mode <= 0 && fp->_IO_write_ptr > fp->_IO_write_base để thỏa mãn điều kiện 1
        _Lock cần là 1 địa chỉ ghi được và phải bằng 1
        Attack chain: Exit -> _run_exit_handler -> _IO_cleanup -> _IO_flush_all -> _IO_wfile_overflow -> _IO_wdoallocbuf -> system


    
    
