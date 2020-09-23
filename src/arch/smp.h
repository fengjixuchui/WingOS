#pragma once
#include <arch/gdt.h>
#include <arch/lock.h>
#include <device/madt.h>
class smp
{

public:
    static const int max_cpu = 64;
    smp();
    void init();
    void init_cpu(int apic, int id);

    int processor_count = 1;
    // we need to do the APIC
    static smp *the();

    MADT_table_LAPIC *mt_lapic[max_cpu];

private:
    tss_t cpu_tss[max_cpu];
};
