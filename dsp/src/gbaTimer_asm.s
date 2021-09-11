.text

//Increases the timer one step of 47kHz, and returns the
//number of overflows within this step
//u16 gbat_updateTimerAsm(u32* pCounter, u16 reload, u32 gbaTicks)
.global gbat_updateTimerAsm
gbat_updateTimerAsm:
    push r0

    mov a0l, r0 //pCounter
    mov a1l, a1l //reload &= 0xFFFF
    shfi a1, a1, +16 //reload <<= 16
    mov [arrn0+ars0], a0 //counter = *pCounter
    set 0x80, mod0 //set logic shift mode
    //clear guard bits of a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    //b0 = step
    //mov 0x0160, b0h
    //set 0x6CC3, b0l
    add b0, a0 //count = counter + step
    clr b0, always //oldTop = 0
loop:
    shfi a0, b1, -32 //newTop = count >> 32
    cmp b0, b1
    br finish, eq
    mov b1, b0 //oldTop = newTop
    add a1, a0 //count += reload
    br loop, always
finish:
    mov a0, [arrn0+ars0] //*pCounter = counter
    mov b1l, a0l //return overflow
    rst 0x80, mod0
    
    pop r0
    ret always