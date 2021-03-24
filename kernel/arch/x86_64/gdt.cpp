#include <com.h>
#include <device/local_data.h>
#include <gdt.h>
#include <kernel.h>
#include <logging.h>
#include <utility.h>
#include <utils/liballoc.h>

gdt_descriptor_array<GDT_DESCRIPTORS> gdt_descriptors[smp::max_cpu];

char tss_ist1[8192] __attribute__((aligned(16)));
char tss_ist2[8192] __attribute__((aligned(16)));
char tss_ist3[8192] __attribute__((aligned(16)));

ASM_FUNCTION void gdtr_install(gdtr *, unsigned short, unsigned short);

void tss_set_rsp0(uint64_t rsp0)
{
    get_current_cpu()->ctss.rsp0 = rsp0;
};

void setup_gdt_descriptors(gdt_descriptor_array<GDT_DESCRIPTORS> *target, uintptr_t tss_base, uintptr_t tss_limit)
{

    target->zero();

    target->set(gdt_selector::KERNEL_CODE, gdt_descriptor(gdt_flags::CS | gdt_flags::WRITABLE, gdt_granularity::LONG_MODE_GRANULARITY));
    target->set(gdt_selector::KERNEL_DATA, gdt_descriptor(gdt_flags::DS | gdt_flags::WRITABLE, 0));
    target->set(gdt_selector::USER_CODE, gdt_descriptor(gdt_flags::CS | gdt_flags::USER | gdt_flags::WRITABLE, gdt_granularity::LONG_MODE_GRANULARITY));
    target->set(gdt_selector::USER_DATA, gdt_descriptor(gdt_flags::DS | gdt_flags::USER | gdt_flags::WRITABLE, 0));

    target->xset(gdt_selector::TSS_SELECTOR, gdt_xdescriptor(gdt_flags::TSS, tss_base, tss_limit));

    gdt_descriptors[get_current_cpu_id()].fill_gdt_register(&get_current_cpu()->cgdt);
}

void __attribute__((optimize("O0"))) setup_gdt()
{
    log("gdt", LOG_DEBUG) << "loading gdt";

    uintptr_t tss_base = (uintptr_t)&get_current_cpu()->ctss;
    uintptr_t tss_limit = tss_base + sizeof(get_current_cpu()->ctss);

    setup_gdt_descriptors(&gdt_descriptors[get_current_cpu_id()], tss_base, tss_limit);

    gdtr_install((&get_current_cpu()->cgdt), gdt_selector::KERNEL_CODE, gdt_selector::KERNEL_DATA);
}

void tss_init(uint64_t i)
{
    memzero(&get_current_cpu()->ctss, sizeof(tss));

    get_current_cpu()->ctss.iomap_base = sizeof(tss);
    get_current_cpu()->ctss.rsp0 = (uintptr_t)i;
    get_current_cpu()->ctss.ist1 = (uintptr_t)i;

    asm volatile("mov ax, %0 \n ltr ax"
                 :
                 : "i"(gdt_selector::TSS_SELECTOR)
                 : "rax");
}

void gdt_ap_init()
{

    log("gdt ap", LOG_DEBUG) << "loading gdt for ap";

    uintptr_t tss_base = (uintptr_t)&get_current_cpu()->ctss;
    uintptr_t tss_limit = tss_base + sizeof(get_current_cpu()->ctss);

    log("gdt ap", LOG_INFO) << "resetting gdt";

    setup_gdt_descriptors(&gdt_descriptors[get_current_cpu_id()], tss_base, tss_limit);

    gdtr_install((&get_current_cpu()->cgdt), gdt_selector::KERNEL_CODE, gdt_selector::KERNEL_DATA);
    memzero(&get_current_cpu()->ctss, sizeof(tss));

    get_current_cpu()->ctss.iomap_base = sizeof(tss);

    get_current_cpu()->ctss.rsp0 = (uintptr_t)POKE(smp_cpu_init_address::STACK);
    get_current_cpu()->ctss.ist1 = (uintptr_t)get_current_cpu()->stack_data_interrupt + 8192;

    asm volatile("mov ax, %0 \n ltr ax"
                 :
                 : "i"(gdt_selector::TSS_SELECTOR)
                 : "rax");
}
