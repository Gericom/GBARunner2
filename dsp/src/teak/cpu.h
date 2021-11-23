#pragma once

extern void cpu_disableIrqs(void);
extern void cpu_enableIrqs(void);

extern void cpu_disableInt0(void);
extern void cpu_enableInt0(void);

extern void cpu_disableInt1(void);
extern void cpu_enableInt1(void);

extern void cpu_disableInt2(void);
extern void cpu_enableInt2(void);

extern void cpu_disableVInt(void);
extern void cpu_enableVInt(void);