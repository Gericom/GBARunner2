.text

.global cpu_disableIrqs
cpu_disableIrqs:
    dint
    ret always

.global cpu_enableIrqs
cpu_enableIrqs:
    eint
    ret always

.global cpu_disableInt0
cpu_disableInt0:
    rst 0x100, mod3
    ret always

.global cpu_enableInt0
cpu_enableInt0:
    set 0x100, mod3
    ret always

.global cpu_disableInt1
cpu_disableInt1:
    rst 0x200, mod3
    ret always

.global cpu_enableInt1
cpu_enableInt1:
    set 0x200, mod3
    ret always

.global cpu_disableInt2
cpu_disableInt2:
    rst 0x400, mod3
    ret always

.global cpu_enableInt2
cpu_enableInt2:
    set 0x400, mod3
    ret always

.global cpu_disableVInt
cpu_disableVInt:
    rst 0x800, mod3
    ret always

.global cpu_enableVInt
cpu_enableVInt:
    set 0x800, mod3
    ret always
