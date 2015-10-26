#pragma once

/* Contains the clock speed to run the CPU depending on region. */
#define NES_CPU_NTSC_CLOCK_SPEED_MHZ 1.7897725
#define NES_CPU_PAL_CLOCK_SPEED_MHZ 1.773447

/* Area of mem holding the Stack. */
#define NES_CPU_STACK_START 0x0100
#define NES_CPU_STACK_SIZE 0xFF

/* Interrupt vector addresses. */
#define NES_CPU_IRQBRK_VECTOR_START 0xFFFE
#define NES_CPU_RESET_VECTOR_START 0xFFFC
#define NES_CPU_NMI_VECTOR_START 0xFFFA