#include <arch/arch.h>
#include <arch/mem/liballoc.h>
#include <arch/mem/physical.h>
#include <arch/mem/virtual.h>
#include <arch/process.h>
#include <arch/smp.h>
#include <com.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <device/pit.h>
#include <kernel.h>
#include <loggging.h>
#define TRAMPOLINE_START 0x1000
#define TRAMPOLINE_PAGING_ADDR 0x4000

#pragma GCC optimize("-O0")
smp main_smp;
int cpu_counter = 0;
extern "C" uint64_t start_cpu_entry;
extern "C" uint64_t end_cpu_entry;
extern "C" uint32_t trampoline_start, trampoline_end, nstack;
bool SMPloaded = false;

smp::smp()
{
}

extern "C" void cpuupstart(void)
{
    SMPloaded = true;
    while (true)
    {
    }
}

void smp::init()
{
    log("smp", LOG_DEBUG) << "loading smp";
    memzero(cpu_tss, sizeof(tss_t) * max_cpu);
    for (int i = 0; i < max_cpu; i++)
    {
        mt_lapic[i] = nullptr;
    }
    MADT_record_table_entry *mrte = madt::the()->get_madt_table_record();
    processor_count = 0;
    while (uint64_t(mrte) < madt::the()->get_madt_table_lenght())
    {
        mrte = (MADT_record_table_entry *)(((uint64_t)mrte) + mrte->tlenght);

        if (mrte->ttype == MADT_type::MADT_LAPIC)
        {

            auto local_apic = reinterpret_cast<MADT_table_LAPIC *>(mrte);

            mt_lapic[processor_count] = local_apic;
            get_current_data(processor_count)->lapic_id = mt_lapic[processor_count]->apic_id;

            processor_count++;
        }
    }

    processor_count--;

    log("smp", LOG_INFO) << "total processor count" << processor_count;
    if (processor_count > max_cpu)
    {
        log("smp", LOG_ERROR) << "too much processor count we will use only " << max_cpu - 1;
    }

    for (int i = 0; i < processor_count; i++)
    {

        log("smp", LOG_INFO) << "set up processor " << mt_lapic[i]->processor_id;
        if (apic::the()->get_current_processor_id() != mt_lapic[i]->processor_id)
        {
            init_cpu(mt_lapic[i]->apic_id, mt_lapic[i]->processor_id);
        }
        else
        {
        }
    }
}
void smp::init_cpu(int apic, int id)
{
    log("smp cpu", LOG_DEBUG) << "loading smp cpu : " << id << "/ apic id :" << apic;
    get_current_data(id)->lapic_id = apic;

    uint64_t trampoline_len = (uint64_t)&trampoline_end - (uint64_t)&trampoline_start;

    for (int i = 0; i < (trampoline_len / 4096) + 12; i++)
    {
        virt_map(0x1000 + (i * 4096), 0x1000 + (i * 4096), 0x1 | 0x2 | 0x4);
    }

    virt_map(0x4000, 0x4000, 0x1 | 0x2 | 0x4);
    virt_map(0x5000, 0x5000, 0x1 | 0x2 | 0x4);

    log("smp cpu", LOG_INFO) << "trampoline length " << trampoline_len;

    uint64_t end_addr = 0x4000;
    end_addr /= 4096;
    end_addr *= 4096;

    virt_map((uint64_t)end_addr, end_addr, 0x1 | 0x2 | 0x4);
    virt_map(0x0, 0x0, 0x1 | 0x2 | 0x4);

    POKE(get_mem_addr(end_addr)) = ((uint64_t)&pl4_table);
    POKE((end_addr)) = ((uint64_t)&pl4_table);
    POKE((0x500)) =
        get_rmem_addr((uint64_t)&pl4_table);
    POKE((0x540)) = get_rmem_addr((uint64_t)&pl4_table);

    asm volatile(" \n"
                 "sgdt [0x580]\n"
                 "sidt [0x590]\n");

    POKE((0x520)) = (uint64_t)&cpuupstart;

    virt_map((uint64_t)&cpuupstart, (uint64_t)&cpuupstart, 0x1 | 0x2 | 0x4);

    uint64_t saddress = 0x8000;
    saddress /= 4096;
    saddress *= 4096;

    virt_map(saddress, saddress, 0x1 | 0x2 | 0x4);

    saddress += 4096;

    virt_map(saddress, saddress, 0x1 | 0x2 | 0x4);

    memset((void *)(saddress - 4096), 0, 4096);

    log("smp cpu", LOG_INFO) << "stack real address" << saddress;
    log("smp cpu", LOG_INFO) << "paging real address" << get_rmem_addr((uint64_t)&pl4_table);

    memcpy((void *)0x1000, &trampoline_start, trampoline_len);
    log("smp cpu", LOG_INFO) << "pre loading cpu : " << id;

    apic::the()->preinit_processor(apic);

    // waiting a little bit

    for (uint64_t i = 0; i < 300; i++)
    {
        for (uint64_t b = 0; b < i * 2; b++)
        {
            inb(0);
        }
    }

    log("smp cpu", LOG_INFO) << " loading cpu : " << id;
    apic::the()->init_processor(apic, 0x1000);
    for (uint64_t i = 0; i < 300; i++)
    {
        for (uint64_t b = 0; b < i * 2; b++)
        {
            inb(0);
        }
    }
    while (SMPloaded != true)
    {
        for (uint64_t i = 0; i < 300; i++)
        {
            for (uint64_t b = 0; b < i * 2; b++)
            {
                inb(0);
            }
        }
    }

    log("smp cpu", LOG_DEBUG) << " loaded cpu : " << id;
    SMPloaded = false;
}
smp *smp::the()
{
    return &main_smp;
}