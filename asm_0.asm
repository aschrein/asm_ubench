global _asm_0
global _asm_empty
global _asm_test_kernel
extern puts

section .data
  message_0 db 'Hello from asm', 0x5b, 0xff00005d, 0x0

section .text
_asm_0:
  push rbp
  lea rdi, [rel message_0]
  call puts wrt ..plt
  pop rbp
  ret

_asm_empty:
  ret

_asm_test_kernel:
  ; mov rax, rdi
  xor rax, rax
  mov rdx, 10
.top:
  times 100 add rax, rax
  dec rdx
jnz .top
  ret
