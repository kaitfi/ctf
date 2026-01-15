# Write-up starlink challenge - Dreamhack
### 1. Checksec & Recon
   * Tác giả cho ta file zip. Giải nén ra ta được 1 dockerfile và 1 thư mục deploy
   * Challenge và Test Flag đều ở trong thư mục deploy
   * Sử dụng `checksec` với `challenge` :

<img width="897" height="211" alt="image" src="https://github.com/user-attachments/assets/df28922c-f1e0-48e1-9515-56dc37e9c96c" />

**Dockerfile :**

<img width="1182" height="456" alt="image" src="https://github.com/user-attachments/assets/6f65add0-6c73-434a-a1b7-3a7b9f087654" />

   * Lấy `Libc` của `server` trong `Dockerfile`
   * Build docker : `docker build -t startlink .`
   * Run dockerfile : `docker run -d -p 8080:8080 startlink`
   * Netcat -> port = 8080 : `nc localhost 8080`
   * Lấy pid của `prob` đang chạy trong docker : `ps aux`
   * DEBUG đính kèm tiến trình : `gdb -p <pid>`
   * Lấy path của Libc, Ld, các file liên quan và copy về thư mục deploy : `docker cp <Container ID>:<Path> deploy/`
   * Patch libc, ld, .. vào challenge
### 2. Reverse Engineering
**Structure :**

<img width="439" height="266" alt="image" src="https://github.com/user-attachments/assets/6ec51255-6138-4167-8107-e8da7500d99f" />

Để `code clean` hơn và đồng thời cũng dễ khi thác ta sẽ add thêm `structure` 

**Main() :**
``` text
__int64 __fastcall main(const char *a1, char **a2, char **a3)
{
  __int64 result; // rax
  int v4; // [rsp+4h] [rbp-Ch] BYREF
  unsigned __int64 v5; // [rsp+8h] [rbp-8h]

  v5 = __readfsqword(0x28u);
  v4 = 0;
  init();
  while ( 2 )
  {
    msg();
    if ( (unsigned int)__isoc99_scanf("%d", &v4) == 1 )
    {
      getchar();
      switch ( v4 )
      {
        case 1:
          allocate();
          continue;
        case 2:
          edit();
          continue;
        case 3:
          delete();
          continue;
        case 4:
          show();
          continue;
        case 5:
          result = 0;
          break;
        default:
          puts("Invalid choice");
          continue;
      }
    }
    else
    {
      puts("Invalid input");
      return 0;
    }
    break;
  }
  return result;
}
```
<img width="495" height="256" alt="image" src="https://github.com/user-attachments/assets/5d2c5f24-1558-4fca-bce4-1c406070ba1f" />

   * Chương ta cho ta 5 `options` gồm `Allocate`, `Edit_note`, `Free_note`, `Print_note`, `Exit`

**Allocate() :**
``` text
unsigned __int64 allocate()
{
  int v0; // eax
  link *i; // [rsp+0h] [rbp-20h]
  link *ptr; // [rsp+8h] [rbp-18h]
  ssize_t size; // [rsp+10h] [rbp-10h]
  unsigned __int64 v5; // [rsp+18h] [rbp-8h]

  v5 = __readfsqword(0x28u);
  ptr = (link *)malloc(0x410u);
  if ( ptr )
  {
    ptr->prev = 0;
    ptr->next = 0;
    printf("Input note: ");
    size = read(0, ptr->data, 0x400u);         
    if ( size > 0 )
    {
      if ( ptr->data[size - 1] == 10 )
        ptr->data[size - 1] = 0;
      if ( (unsigned __int64)size > 0x3FF )
        ptr->data[1023] = 0;
      else
        ptr->data[size] = 0;
      if ( start_link_list )
      {
        for ( i = (link *)start_link_list; i->next; i = (link *)i->next )
          ;
        i->next = (__int64 *)ptr;
        ptr->prev = (prev_t *)i;             
      }
      else
      {
        start_link_list = (__int64)ptr;
      }
      v0 = count++;
      printf("Allocated note %d\n", v0);
    }
    else
    {
      free(ptr);
    }
  }
  else
  {
    puts("Allocation failed");
  }
  return v5 - __readfsqword(0x28u);
}
```
   * Chương trình ptr = malloc(0x410) là một size khá lớn => Nguồn leak libc tiềm năng (khi Free sẽ đi vào unsorted bin)
   * Sau đó nó cho ta nhập `0x400` bytes vào `ptr->data` và nếu `size` vượt quá `0x3FF` thì chương trình lập tức `set NULL` tại `ptr->data[0x3FF] => không có Heap Overflow ở đây
   * Cuối cùng nó chèn `ptr` vừa xong vào cuối `link list` tại `ptr->next` và chèn con trỏ đã malloc trước `ptr` vào 8 bytes đầu của `ptr` hiện tại (tức là `ptr->prev`)
   * Nếu `ptr` hiện tại là `Index = 0` thì Gán `start_link_list = ptr`

**Edit_note() :**

``` text
unsigned __int64 edit()
{
  int idx; // [rsp+4h] [rbp-1Ch] BYREF
  link *pointer_idx; // [rsp+8h] [rbp-18h]
  ssize_t v3; // [rsp+10h] [rbp-10h]
  unsigned __int64 v4; // [rsp+18h] [rbp-8h]

  v4 = __readfsqword(0x28u);
  idx = 0;
  printf("Index to edit: ");
  if ( (unsigned int)__isoc99_scanf("%d", &idx) == 1 )
  {
    getchar();
    pointer_idx = get_pointer_idx(idx);
    if ( pointer_idx )
    {
      sub_40147D((unsigned __int64)pointer_idx);// safe libc
      printf("New content: ");
      v3 = read(0, pointer_idx->data, 0x408u);  // overflow
      if ( v3 > 0 )
      {
        if ( pointer_idx->data[v3 - 1] == 10 )
          pointer_idx->data[v3 - 1] = 0;
        if ( (unsigned __int64)v3 > 0x3FF )
          pointer_idx->data[1023] = 0;
        else
          pointer_idx->data[v3] = 0;
        puts("Edited successfully");
      }
    }
    else
    {
      puts("No such note");
    }
  }
  else
  {
    puts("Invalid index");
  }
  return v4 - __readfsqword(0x28u);
}
```
   * `sub_40147D` Xuất hiện Nhằm tăng bảo mật cho libc, tránh `Attacker` có thể ghi đè vào `Libc GOT` hoặc các giá trị quan trọng khác trong `LIBC`
   * Thực hiện `update` vào `ptr->data` tại `index` mà chương trình cho nhập ở trên
   * Cho chương trình cho phép nhập `0x408` bytes vào `ptr->data` nên ở đây ta đã overwrite ngay sau ptr->data
   * Mà ngay sau `ptr->data` là `ptr->next` lưu con trỏ tiếp theo trong `link list` => Ta có thể ghi đè `ptr->next` chuyển hướng `link list` và ta có thể control để nó trỏ đến con trỏ mà ta mong muốn => Ta có `AAW` nếu ta leak được địa chỉ đó

**Free_note() :** 

<img width="887" height="647" alt="image" src="https://github.com/user-attachments/assets/d139b09b-c5a5-4234-86bf-45fbbce9192f" />

   * Cho phép ta nhập `Index` và `Free` Node tại `Index`
   * Trước khi `Free` chương trình sẽ cập nhật các `Note` được mô tả như sau:

``` text
ptrA -> ptrB -> ptrC
- Free(ptrB)
=>  ptrA->next = ptrC : được thể hiện thông qua phép gán sau : ptr->prev->next = ptr->next 
    ptrC->prev = ptrA : được thể hiện thông qua phép gán sau : *ptr->next = (__int64)ptr->prev
- Free(ptrA)
=>  Start_lisk_list = ptrB
    Phần còn lại giống với trường hợp Free Node ptrB
```

   * Ở đây tuy chương trình đã `free` và không `set NULL` cho `ptr` nhưng ta không có `UAF` do chương trình đã xóa `Node` đó ra khỏi `link list` nên ta không thể truy cập được

**Print_node() :**

<img width="743" height="473" alt="image" src="https://github.com/user-attachments/assets/f3a2e110-fcae-4fe3-8dfa-b4bc7bbd0fbe" />

   * Cho nhập vào `Index` và in ra `ptr->data` tại `index`

### 3. Exploit

   * Để `leak libc` ta khai thác lỗ hổng có thể overwrite vào `ptr->next` ngay sau `ptr->data` và `PIE disable`
   * Ta có thể chuyển hướng `link list` và bắt chương trình in ra địa của `exe.got` hay địa chỉ `exe address` nào đó có chứa địa chỉ `libc`
   * Để `AAW` ta cũng overwrite `ptr->prev->next` và ghi vào `ptr->next` 
   * Sau khi có được `Libc Base` ta có thể triển khai các phương án khai thác sau ( Do đây là `libc 2.35` nên sẽ không tồn tại `__free_hook` hay `__malloc_hook`) : 
   *     Tiếp tục `leak stack` sử dụng `__libc_environ` với cách tương tự
   *     FSOP
**Ta sẽ chọn phương án FSOP** (có thể khai thác với phương án tùy người đọc)
   * Ta sẽ ghi đè vào `_IO_2_1_stdout_`

**Note for payload :**
   * Attack chain : `puts` -> `_IO_puts` -> `_IO_wfile_overflow` (overwrite `*(fd + 0xd8)` = `_IO_wfile_jumps - 0x20`) -> `_IO_wdoallocbuf` -> `system`
   * `_lock` phải có quyền `write` và bằng 1
   * `*(wide_data + 0xe0)` = `address` - 0x68 & `*address` = `system`
   * Set các giá trị để đi vào nhánh mà ta mong muốn ([_IO_wfile_overflow](https://elixir.bootlin.com/glibc/glibc-2.35/source/libio/wfileops.c#L406), [_IO_wdoallocbuf](https://elixir.bootlin.com/glibc/glibc-2.35/source/libio/wgenops.c#L364))
   * [SCRIPT](./exploit.py) 
