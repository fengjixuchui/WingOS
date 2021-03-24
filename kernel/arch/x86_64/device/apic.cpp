#include <arch.h>
#include <com.h>
#include <device/acpi.h>
#include <device/apic.h>
#include <device/local_data.h>
#include <logging.h>
#include <pic.h>
#include <virtual.h>
apic main_apic = apic();

apic::apic()
{
    loaded = false;
}

void apic::io_write(uint64_t base, uint32_t reg, uint32_t data)
{

    *((volatile uint32_t *)(((uintptr_t)base))) = reg;
    *((volatile uint32_t *)(((uintptr_t)base + 16))) = data;
}

uint32_t apic::io_read(uint64_t base, uint32_t reg)
{
    *((volatile uint32_t *)(((uintptr_t)base))) = reg;

    return *((volatile uint32_t *)(((uintptr_t)base + 16)));
}

void apic::enable()
{
    write(sivr, read(sivr) | 0x1FF);
}

void apic::EOI()
{
    write(eoi, 0);
}

bool apic::isloaded()
{
    return loaded;
}

void apic::log_all()
{
    log("apic", LOG_INFO) << "apic address" << (uintptr_t)apic_addr;
    log("apic", LOG_INFO) << "current processor id " << get_current_processor_id();

    table = madt::the()->get_madt_ioAPIC();

    for (int i = 0; table[i] != 0; i++)
    {
        log("io apic", LOG_INFO) << "info for io apic" << i;

        uintptr_t addr = (table[i]->ioapic_addr);

        log("io apic", LOG_INFO) << "io apic addr " << addr;

        map_page(addr, addr, 0x03);
        update_paging();

        uint32_t raw_table = (io_read(addr, version_reg));

        io_apic_version_table *tables = (io_apic_version_table *)&raw_table;
        io_version_data = *tables;

        log("io apic", LOG_INFO) << "version         : " << tables->version;
        log("io apic", LOG_INFO) << "max redirection : " << tables->maximum_redirection;
        log("io apic", LOG_INFO) << "gsi start       : " << table[i]->gsib;
        log("io apic", LOG_INFO) << "gsi end         : " << table[i]->gsib + tables->maximum_redirection;
    }
    for (int i = 0; iso_table[i] != 0; i++)
    {

        log("iso", LOG_INFO) << " info for iso" << i;
        log("iso", LOG_INFO) << "iso source : " << iso_table[i]->irq;
        log("iso", LOG_INFO) << "iso target : " << iso_table[i]->interrupt;

        if (iso_table[i]->misc_flags & 0x4)
        {
            log("iso", LOG_INFO) << "iso is active high";
        }
        else
        {
            log("iso", LOG_INFO) << "iso is active low";
        }

        if (iso_table[i]->misc_flags & 0x100)
        {
            log("iso", LOG_INFO) << "iso is edge triggered";
        }
        else
        {
            log("iso", LOG_INFO) << "iso is level triggered";
        }
    }
}
void apic::init()
{
    log("apic", LOG_DEBUG) << "loading apic";

    apic_addr = (void *)((uintptr_t)madt::the()->lapic_base);
    for (int i = 0; i < 3; i++)
    {
        map_page((uint64_t)apic_addr + i * PAGE_SIZE, (uint64_t)apic_addr + i * PAGE_SIZE, 0x03);
    }
    if (apic_addr == nullptr)
    {
        log("apic", LOG_FATAL) << "can't find apic";

        while (true)
        {
            asm("hlt");
        }

        return;
    }

    x86_wrmsr(MSR_REGISTERS::APIC, (x86_rdmsr(MSR_REGISTERS::APIC) | 0x800) & ~(LAPIC_ENABLE));
    enable();

    outb(PIC1_DATA, 0xff); // turn off the pic
    pic_wait();
    outb(PIC2_DATA, 0xff);

    log("io apic", LOG_DEBUG) << "loading io apic";

    table = madt::the()->get_madt_ioAPIC();

    log("iso", LOG_DEBUG) << "loading iso";

    iso_table = madt::the()->get_madt_ISO();

    loaded = true;

    log_all();
}

uint32_t apic::read(uint32_t regs)
{
    return *((volatile uint32_t *)((uintptr_t)apic_addr + regs));
}

void apic::write(uint32_t regs, uint32_t val)
{
    *((volatile uint32_t *)(((uintptr_t)apic_addr) + regs)) = val;
}

uint32_t apic::IO_get_max_redirect(uint32_t apic_id)
{
    uintptr_t addr = (table[apic_id]->ioapic_addr);
    uint32_t raw_table = (io_read(addr, version_reg));

    io_apic_version_table *tables = (io_apic_version_table *)&raw_table;
    return tables->maximum_redirection;
}

void apic::set_apic_addr(uint32_t new_address)
{
    // not used
}

apic *apic::the()
{
    return &main_apic;
}

uint32_t apic::get_current_processor_id()
{
    return (read(lapic_id) >> 24);
}

void apic::preinit_processor(uint32_t processorid)
{
    write(icr2, (processorid << 24));
    write(icr1, 0x500);
}

void apic::init_processor(uint32_t processorid, uint64_t entry)
{
    write(icr2, (processorid << 24));
    write(icr1, 0x600 | ((uint32_t)entry / PAGE_SIZE));
}

void apic::set_raw_redirect(uint8_t vector, uint32_t target_gsi, uint16_t flags, int cpu, int status)
{
    uint64_t end = vector;

    int64_t io_apic_target = -1;
    for (uint64_t i = 0; table[i] != 0; i++)
    {
        if (table[i]->gsib <= target_gsi)
        {
            if (table[i]->gsib + IO_get_max_redirect(i) > target_gsi)
            {
                io_apic_target = i;
                break;
            }
        }
    }

    if (io_apic_target == -1)
    {

        log("io apic", LOG_ERROR) << "error while trying to setup raw redirect for io apic :( no iso table found ";
        return;
    }

    if (flags & active_high_low)
    {
        end |= (1 << 13);
    }

    if (flags & edge_level)
    {
        end |= (1 << 15);
    }

    if (!status)
    {
        end |= (1 << 16);
    }

    end |= (((uintptr_t)get_current_cpu(cpu)->lapic_id) << 56);
    uint32_t io_reg = (target_gsi - table[io_apic_target]->gsib) * 2 + 16;

    io_write(table[io_apic_target]->ioapic_addr, io_reg, (uint32_t)end);
    io_write(table[io_apic_target]->ioapic_addr, io_reg + 1, (uint32_t)(end >> 32));
}

void apic::send_ipi(uint8_t cpu, uint32_t interrupt_num)
{
    interrupt_num = (1 << 14) | interrupt_num;

    write(apic_register::icr2, (cpu << 24));
    write(apic_register::icr1, interrupt_num);
}

void apic::set_redirect_irq(int cpu, uint8_t irq, int status)
{
    log("io apic", LOG_INFO) << "setting redirect irq for cpu : " << cpu << " irq : " << irq << " status : " << status;

    for (uint64_t i = 0; iso_table[i] != 0; i++)
    {
        if (iso_table[i]->irq == irq)
        {
            log("io apic", LOG_INFO) << "iso matching : " << i << " mapping to source" << iso_table[i]->irq + 0x20 << " gsi : " << iso_table[i]->interrupt;

            set_raw_redirect(iso_table[i]->irq + 0x20, iso_table[i]->interrupt, iso_table[i]->misc_flags, cpu, status);
            return;
        }
    }

    set_raw_redirect(irq + 0x20, irq, 0, cpu, status);
}
