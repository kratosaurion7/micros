global ks_get_registers
global ks_do_activate

global ks_save_fix_registers

extern Debugger

ks_save_fix_registers:
    push ebp
    mov ebp, esp
    sub esp, 48 ; 44 byte for struct and 4 for something
    
    push [ebp+144] ; esp
    push [ebp+148] ; eip
    pushf ; flags
    push 0x8 ; cs
    pusha ; regs
    
    mov eax, [ebp+8]
    
    mov edx, [ebp-76] ; ESP
    mov [eax+24], edx
    
    mov edx, [ebp-72] ; EIP
    mov [eax], edx
    
    mov edx, [ebp-68] ; EFLAGS
    mov [eax+8], edx
    
    mov edx, [ebp-64] ; CS
    mov [eax+4], edx
    
    pop ebp
    ret

ks_get_registers: ; p *(struct regs_t*)$ebx or [ebp+8]
    push ebp
    mov ebp, esp
    push eax

    pusha
    pushf ; Eflags
    push 0x8 ; CS
    call ks_get_eip
    push eax ; EIP
    
    ; mov ebx, [ebp+12]
    ; cmp ebx, 1
    ; je 

    ; Moving the parameter into ebx after we push the regs so we save the
    ; old value of ebx.
    mov ebx, [ebp+8]

    ; mov eax, [esp+12] ; [REG] - Where we take the value from the stack
    ; mov [ebx+12], eax ; Where we place the value in the structure

    mov eax, [esp+12] ; EDI
    mov [ebx+12], eax

    mov eax, [esp+16] ; ESI
    mov [ebx+16], eax

    mov eax, [esp+20] ; EBP
    mov [ebx+20], eax

    mov eax, [esp+24] ; ESP
    mov [ebx+24], eax

    mov eax, [esp+28] ; EBX
    mov [ebx+28], eax

    mov eax, [esp+32] ; EDX
    mov [ebx+32], eax

    mov eax, [esp+36] ; ECX
    mov [ebx+36], eax

    ; For EAX we select the one that was pushed in the beginning
    ; This is because EAX is used to return the value of ks_get_eip
    mov eax, [esp+44] ; EAX
    mov [ebx+40], eax

    mov eax, [esp+8] ; EFLAGS
    mov [ebx+8], eax

    mov eax, [esp+4] ; CS
    mov [ebx+4], eax

    mov eax, [esp]
    mov [ebx], eax ; EIP

    mov [ebp - 4], eax ; 4-byte Spill
    add esp, 48 ; Roll back our pushes

    pop ebp
    ret

ks_get_eip:
    mov eax, [esp]
    ret

ks_do_activate: ; p *(struct task_t*)$eax
    push ebp
    mov ebp, esp

    ;call Debugger

    mov eax, [ebp+8]

    mov ebx, [eax+28] ; Target stack

    ;  ---> $ebp+4 <--- contains the saved EIP
    ; Move all the registers to target stack
    ; SUB target ESP to top of target stack
    ; Switch current stack to target stack
    ; pop registers back
    ; pop flags
    ; pop cs
    ; execute ret, which will pop eip

    ; Registers :   EAX = target task_t structure
    ;               EBX = target stack address
    ;               ECX = scratch register

    mov ecx, [eax+44] ; Emplace EAX
    mov [ebx-12], ecx

    mov ecx, [eax+40] ; Emplace ECX
    mov [ebx-16], ecx

    mov ecx, [eax+36] ; Emplace EDX
    mov [ebx-20], ecx

    mov ecx, [eax+32] ; Emplace EBX
    mov [ebx-24], ecx

    mov ecx, [eax+28] ; Emplace ESP STUB
    mov [ebx-28], DWORD 0

    mov ecx, [eax+24] ; Emplace EBP
    mov [ebx-32], ecx

    mov ecx, [eax+20] ; Emplace ESI
    mov [ebx-36], ecx

    mov ecx, [eax+16] ; Emplace EDI
    mov [ebx-40], ecx

   ; Order of POPA
;    EDI ← Pop();
;    ESI ← Pop();
;    EBP ← Pop();
;    Increment ESP by 4; (* Skip next 4 bytes of stack *)
;    EBX ← Pop();
;    EDX ← Pop();
;    ECX ← Pop();
;    EAX ← Pop();
    ;call Debugger

    mov ecx, [eax+12] ; Emplace EFLAGS
    mov [ebx-8], ecx

    ; TODO : Cannot switch CS during normal switching operations
    ; mov ecx, [eax+8] ; Emplace CS
    ; mov [ebx-52], ecx

    mov ecx, [eax+4] ; Emplace EIP
    mov [ebx-4], ecx

    mov ebp, [eax+24] ; Use the new ebp
    mov esp, [eax+28] ; Use the new stack

    sub esp, 40

    popa
    popfd
    ;pop cs
    ;mov cs, 0x8
    ; +4 byte to account for saved EIP

    ;pop ebp ; Prolly not need that
    ret
