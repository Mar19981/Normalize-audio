.data
Params struct
	peaks dq ?
	max real4 ?
Params ends
.code DllEntry  
proc hInstDll: dword, fdwReason: dword, lpReserved: dword
	mov rax, 1
	ret
DllEntry  endp

_DllMainCRTStartup proc
mov rax, 1
ret
_DllMainCRTStartup endp


normalize proc 
push rsi 
mov rsi, [r9].Params.peaks 
movss xmm2, [r9].Params.max 
vbroadcastss xmm3, xmm2 
cmp r8, 4 
jl remainingLoop 
processLoop: 
movups xmm1, xmm3 
movups xmm0, real4 ptr [rcx][rdx * 4] 
movups xmm2, real4 ptr [rsi][rdx * 4] 
divps xmm1, xmm2 
mulps xmm0, xmm1 
movups real4 ptr [rcx][rdx * 4], xmm0 
add rdx, 4 
sub r8, 4 
cmp r8, 4 
jbe processLoop 
cmp r8, 0 
je return 
remainingLoop: 
movups xmm1, xmm3 
movss xmm0, real4 ptr [rcx][rdx * 4] 
movss xmm2, real4 ptr [rsi][rdx * 4] 
divps xmm1, xmm2 
mulps xmm0, xmm1 
movss real4 ptr [rcx][rdx * 4], xmm0 
inc rdx 
dec r8 
cmp r8, 0 
jnz remainingLoop 
return: 
pop rsi 
ret 
normalize endp 

end 