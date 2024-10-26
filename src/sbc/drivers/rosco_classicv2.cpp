/*
 *------------------------------------------------------------
 *                                  ___ ___ _   
 *  ___ ___ ___ ___ ___       _____|  _| . | |_ 
 * |  _| . |_ -|  _| . |     |     | . | . | '_|
 * |_| |___|___|___|___|_____|_|_|_|___|___|_,_| 
 *                     |_____|                       
 * ------------------------------------------------------------
 * Copyright (c) 2024 The rosco_m68k Open Source Project
 * MIT License
 
 * Portions (c) Chris Hanson
 * BSD 3-clause License
 * ------------------------------------------------------------
 */

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68010.h"
#include "cpu/m68000/m68020.h"
#include "cpu/m68000/m68030.h"
#include "machine/mc68681.h"
#include "machine/spi_sdcard.h"


class rosco_m68k_state : public driver_device
{
public:
	rosco_m68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_duart(*this, "duart")
		, m_terminal(*this, "terminal")
		, m_host(*this, "host")
		, m_sdcard(*this, "sdcard")
		, m_ata(*this, "ata")
	{ }

protected:
	required_device<m68000_base_device> m_maincpu;
	required_device<xr68c681_device> m_duart;
	bool m_bus_error = false;

	void rosco_m68k(machine_config &config);

	void mem_map(address_map &map);
	void cpu_space_map(address_map &map);

	virtual void delegated_mem_map(address_map &map) = 0;
	virtual void delegated_cpu_space_map(address_map &map) = 0;
	virtual void bootvec_reset() = 0;

	uint16_t unmapped_ram_r(offs_t offset, uint16_t mem_mask);
	void unmapped_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t unmapped_exp_r(offs_t offset, uint16_t mem_mask);
	void unmapped_exp_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void set_bus_error(uint32_t address, bool rw, uint16_t mem_mask);	

private:
	required_device<rs232_port_device> m_terminal;
	required_device<rs232_port_device> m_host;

	required_device<spi_sdcard_device> m_sdcard;
	required_device<ata_interface_device> m_ata;

	emu_timer* m_bus_error_timer = nullptr;

	void write_red_led(int state);
	void write_green_led(int state);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	TIMER_CALLBACK_MEMBER(bus_error);
};

class rosco_m68k_000_state : public rosco_m68k_state
{
public:
	rosco_m68k_000_state(const machine_config &mconfig, device_type type, const char *tag)
		: rosco_m68k_state(mconfig, type, tag)
		, m_bootvect(*this, "bootvect")
		, m_sysram(*this, "ram")
	{}

	void rosco_m68k_000(machine_config &config);

private:
	memory_view m_bootvect;
	required_shared_ptr<uint16_t> m_sysram; // Pointer to System RAM needed by bootvect_w and masking RAM buffer for post reset accesses

	virtual void delegated_mem_map(address_map &map) override;
	virtual void delegated_cpu_space_map(address_map &map) override;

	void bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	virtual void bootvec_reset() override;
};

class rosco_m68k_010_state : public rosco_m68k_state
{
public:
	rosco_m68k_010_state(const machine_config &mconfig, device_type type, const char *tag)
		: rosco_m68k_state(mconfig, type, tag)
		, m_bootvect(*this, "bootvect")
		, m_sysram(*this, "ram")
	{}

	void rosco_m68k_010(machine_config &config);

private:
	memory_view m_bootvect;
	required_shared_ptr<uint16_t> m_sysram; // Pointer to System RAM needed by bootvect_w and masking RAM buffer for post reset accesses

	virtual void delegated_mem_map(address_map &map) override;
	virtual void delegated_cpu_space_map(address_map &map) override;

	void bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	virtual void bootvec_reset() override;
};

class rosco_m68k_020_state : public rosco_m68k_state
{
public:
	rosco_m68k_020_state(const machine_config &mconfig, device_type type, const char *tag)
		: rosco_m68k_state(mconfig, type, tag)
		, m_bootvect(*this, "bootvect")
		, m_sysram(*this, "ram")
	{}

	void rosco_m68k_020(machine_config &config);

private:
	memory_view m_bootvect;
	required_shared_ptr<uint32_t> m_sysram; // Pointer to System RAM needed by bootvect_w and masking RAM buffer for post reset accesses

	virtual void delegated_mem_map(address_map &map) override;
	virtual void delegated_cpu_space_map(address_map &map) override;

	void bootvect_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	virtual void bootvec_reset() override;
};

class rosco_m68k_030_state : public rosco_m68k_state
{
public:
	rosco_m68k_030_state(const machine_config &mconfig, device_type type, const char *tag)
		: rosco_m68k_state(mconfig, type, tag)
		, m_bootvect(*this, "bootvect")
		, m_sysram(*this, "ram")
	{}

	void rosco_m68k_030(machine_config &config);

private:
	memory_view m_bootvect;
	required_shared_ptr<uint32_t> m_sysram; // Pointer to System RAM needed by bootvect_w and masking RAM buffer for post reset accesses

	virtual void delegated_mem_map(address_map &map) override;
	virtual void delegated_cpu_space_map(address_map &map) override;

	void bootvect_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	virtual void bootvec_reset() override;
};


/* Input ports */
static INPUT_PORTS_START( rosco_m68k )
INPUT_PORTS_END


/* Terminal default settings. */
static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


void rosco_m68k_000_state::rosco_m68k_000(machine_config &config)
{
	M68000(config, m_maincpu, 10_MHz_XTAL);

	rosco_m68k(config);
}

void rosco_m68k_010_state::rosco_m68k_010(machine_config &config)
{
	M68010(config, m_maincpu, 10_MHz_XTAL);

	rosco_m68k(config);
}

void rosco_m68k_020_state::rosco_m68k_020(machine_config &config)
{
	M68020(config, m_maincpu, 20_MHz_XTAL);

	rosco_m68k(config);
}

void rosco_m68k_030_state::rosco_m68k_030(machine_config &config)
{
	M68030(config, m_maincpu, 20_MHz_XTAL);

	rosco_m68k(config);
}

void rosco_m68k_state::rosco_m68k(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &rosco_m68k_state::mem_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &rosco_m68k_state::cpu_space_map);

	// Set up DUART, both binding to serial ports and handling GPIO.
	// IP0 = CTS_A
	// IP1 = CTS_B
	// IP2 = SPI_CIPO
	// IP3 = ???
	// IP4 = ???
	// IP5 = ???
	//
	// OP0 = RTS_A
	// OP1 = RTS_B
	// OP2 = SPI_CS
	// OP3 = RED_LED
	// OP4 = SPI_SCK
	// OP5 = GREEN_LED
	// OP6 = SPI_COPI
	// OP7 = SPI_CS1

	XR68C681(config, m_duart, 10_MHz_XTAL);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	m_duart->a_tx_cb().set("terminal", FUNC(rs232_port_device::write_txd));
	m_duart->outport_cb().set("terminal", FUNC(rs232_port_device::write_rts)).bit(0);
	m_duart->b_tx_cb().set("host", FUNC(rs232_port_device::write_txd));
	m_duart->outport_cb().append("host", FUNC(rs232_port_device::write_rts)).bit(1);
	m_duart->outport_cb().append(FUNC(rosco_m68k_state::write_red_led)).bit(3);
	m_duart->outport_cb().append(FUNC(rosco_m68k_state::write_green_led)).bit(5);
	m_duart->outport_cb().append(m_sdcard, FUNC(spi_sdcard_device::spi_ss_w)).bit(2).invert();
	m_duart->outport_cb().append(m_sdcard, FUNC(spi_sdcard_device::spi_clock_w)).bit(4);
	m_duart->outport_cb().append(m_sdcard, FUNC(spi_sdcard_device::spi_mosi_w)).bit(6);

	RS232_PORT(config, m_terminal, default_rs232_devices, "terminal");
	m_terminal->rxd_handler().set(m_duart, FUNC(xr68c681_device::rx_a_w));
	m_terminal->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	m_terminal->cts_handler().set(m_duart, FUNC(xr68c681_device::ip0_w));

	RS232_PORT(config, m_host, default_rs232_devices, nullptr);
	m_host->rxd_handler().set(m_duart, FUNC(xr68c681_device::rx_b_w));
	m_host->cts_handler().set(m_duart, FUNC(xr68c681_device::ip1_w));

	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set(m_duart, FUNC(xr68c681_device::ip2_w));

	ATA_INTERFACE(config, m_ata, 0).options(ata_devices, "hdd", nullptr, false);
	m_ata->irq_handler().set_inputline(m_maincpu, M68K_IRQ_3);
}

void rosco_m68k_state::write_red_led(int state)
{
	// Nothing yet
}

void rosco_m68k_state::write_green_led(int state)
{
	// Nothing yet
}

void rosco_m68k_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share("ram"); /* 1MB RAM */
	map(0xe00000, 0xefffff).rom().region("monitor", 0); /* 1MB ROM (max) */
	map(0xf00000, 0xf0001f).rw("duart", FUNC(xr68c681_device::read), FUNC(xr68c681_device::write)).umask16(0x00ff);
	map(0xf80040, 0xf8004f).rw("ata", FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)).umask16(0xffff);
	map(0xf80050, 0xf8005f).rw("ata", FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w)).umask16(0xffff);

	// Unmapped areas to bus error...
	map(0x100000, 0xdfffff).rw(FUNC(rosco_m68k_state::unmapped_ram_r), FUNC(rosco_m68k_state::unmapped_ram_w));
	map(0xf00020, 0xf8003f).rw(FUNC(rosco_m68k_state::unmapped_exp_r), FUNC(rosco_m68k_state::unmapped_exp_w));
	map(0xf80060, 0xfffdff).rw(FUNC(rosco_m68k_state::unmapped_exp_r), FUNC(rosco_m68k_state::unmapped_exp_w));

	delegated_mem_map(map);
}

void rosco_m68k_000_state::delegated_mem_map(address_map &map)
{
	map(0x000000, 0x000007).view(m_bootvect);
	m_bootvect[0](0x000000, 0x000007).rom().region("monitor", 0);           		// After first write we act as RAM
	m_bootvect[0](0x000000, 0x000007).w(FUNC(rosco_m68k_000_state::bootvect_w)); 	// ROM mirror just during reset
}

void rosco_m68k_010_state::delegated_mem_map(address_map &map)
{
	map(0x000000, 0x000007).view(m_bootvect);
	m_bootvect[0](0x000000, 0x000007).rom().region("monitor", 0);           		// After first write we act as RAM
	m_bootvect[0](0x000000, 0x000007).w(FUNC(rosco_m68k_010_state::bootvect_w)); 	// ROM mirror just during reset
}

void rosco_m68k_020_state::delegated_mem_map(address_map &map)
{
	map(0x000000, 0x000007).view(m_bootvect);
	m_bootvect[0](0x000000, 0x000007).rom().region("monitor", 0);           		// After first write we act as RAM
	m_bootvect[0](0x000000, 0x000007).w(FUNC(rosco_m68k_020_state::bootvect_w)); 	// ROM mirror just during reset

	// Additional address space to bus error
	map(0x01000000, 0xffffffff).rw(FUNC(rosco_m68k_020_state::unmapped_ram_r), FUNC(rosco_m68k_020_state::unmapped_ram_w));
}

void rosco_m68k_030_state::delegated_mem_map(address_map &map)
{
	map(0x000000, 0x000007).view(m_bootvect);
	m_bootvect[0](0x000000, 0x000007).rom().region("monitor", 0);           		// After first write we act as RAM
	m_bootvect[0](0x000000, 0x000007).w(FUNC(rosco_m68k_030_state::bootvect_w)); 	// ROM mirror just during reset

	// Additional address space to bus error
	map(0x01000000, 0xfffffdff).rw(FUNC(rosco_m68k_030_state::unmapped_ram_r), FUNC(rosco_m68k_030_state::unmapped_ram_w));
}

void rosco_m68k_state::cpu_space_map(address_map &map)
{
	delegated_cpu_space_map(map);
}

void rosco_m68k_000_state::delegated_cpu_space_map(address_map &map)
{
	map(0x00fffff0, 0x00ffffff).m(m_maincpu, FUNC(m68000_device::autovectors_map));
	map(0x00fffff9, 0x00fffff9).r(m_duart, FUNC(xr68c681_device::get_irq_vector));
}

void rosco_m68k_010_state::delegated_cpu_space_map(address_map &map)
{
	map(0x00fffff0, 0x00ffffff).m(m_maincpu, FUNC(m68010_device::autovectors_map));
	map(0x00fffff9, 0x00fffff9).r(m_duart, FUNC(xr68c681_device::get_irq_vector));
}

void rosco_m68k_020_state::delegated_cpu_space_map(address_map &map)
{
	map(0xfffffff0, 0xffffffff).m(m_maincpu, FUNC(m68020_device::autovectors_map));
	map(0xfffffff9, 0xfffffff9).r(m_duart, FUNC(xr68c681_device::get_irq_vector));
}

void rosco_m68k_030_state::delegated_cpu_space_map(address_map &map)
{
	map(0xfffffff0, 0xffffffff).m(m_maincpu, FUNC(m68030_device::autovectors_map));
	map(0xfffffff9, 0xfffffff9).r(m_duart, FUNC(xr68c681_device::get_irq_vector));
}

void rosco_m68k_state::machine_start()
{
	m_bus_error_timer = timer_alloc(FUNC(rosco_m68k_state::bus_error), this);
}

void rosco_m68k_state::machine_reset()
{
	m_sdcard->spi_clock_w(CLEAR_LINE);

	bootvec_reset();
}

void rosco_m68k_000_state::bootvec_reset()
{
	// Reset pointer to bootvector in ROM for bootvector view
	m_bootvect.select(0);
}

void rosco_m68k_010_state::bootvec_reset()
{
	// Reset pointer to bootvector in ROM for bootvector view
	m_bootvect.select(0);
}

void rosco_m68k_020_state::bootvec_reset()
{
	// Reset pointer to bootvector in ROM for bootvector view
	m_bootvect.select(0);
}

void rosco_m68k_030_state::bootvec_reset()
{
	// Reset pointer to bootvector in ROM for bootvector view
	m_bootvect.select(0);
}

// Boot vector handlers: The PCB hardwires the first 8 bytes from 0x008000 to 0x0 at reset.
void rosco_m68k_000_state::bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sysram[offset]);
	m_bootvect.disable(); // redirect all upcoming accesses to masking RAM until reset.
}

void rosco_m68k_010_state::bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sysram[offset]);
	m_bootvect.disable(); // redirect all upcoming accesses to masking RAM until reset.
}

void rosco_m68k_020_state::bootvect_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sysram[offset]);
	m_bootvect.disable(); // redirect all upcoming accesses to masking RAM until reset.
}

void rosco_m68k_030_state::bootvect_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sysram[offset]);
	m_bootvect.disable(); // redirect all upcoming accesses to masking RAM until reset.
}

uint16_t rosco_m68k_state::unmapped_ram_r(offs_t offset, uint16_t mem_mask)
{
	/* Unmapped RAM - bus error */
	set_bus_error((offset << 1), 0, mem_mask);
	return 0xff;
}

void rosco_m68k_state::unmapped_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* Unmapped RAM - bus error */
	set_bus_error((offset << 1), 1, mem_mask);
}

uint16_t rosco_m68k_state::unmapped_exp_r(offs_t offset, uint16_t mem_mask)
{
	/* Unmapped expansion  - bus error */
	set_bus_error((offset << 1) + 0xf00000, 0, mem_mask);
	return 0xff;
}

void rosco_m68k_state::unmapped_exp_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* These are expansion devices, if not present, they cause a bus error */
	set_bus_error((offset << 1) + 0xf00000, 1, mem_mask);
}

TIMER_CALLBACK_MEMBER(rosco_m68k_state::bus_error)
{
	m_bus_error = false;
}

void rosco_m68k_state::set_bus_error(uint32_t address, bool rw, uint16_t mem_mask)
{
 	if(m_bus_error) {
		return;
	}

	m_bus_error = true;

	m68000_musashi_device *cpuptr = downcast<m68000_musashi_device *>(m_maincpu.target());
	cpuptr->set_buserror_details(address, rw, cpuptr->get_fc());
	cpuptr->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	cpuptr->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	m_bus_error_timer->adjust(cpuptr->cycles_to_attotime(16)); // let rmw cycles complete
}


/* ROM definitions */
ROM_START( rosco_m68k_000 )
	ROM_REGION16_BE(0x100000, "monitor", 0)
	ROM_LOAD( "rosco_m68k.rom", 0x00000, 0x100000, CRC(e7502a9b) SHA1(c729b5e2dd78de1d3484402a5fa8ea27ea492a3f))
ROM_END

ROM_START( rosco_m68k_010 )
	ROM_REGION16_BE(0x100000, "monitor", 0)
	ROM_LOAD( "rosco_m68k.rom", 0x00000, 0x100000, CRC(e7502a9b) SHA1(c729b5e2dd78de1d3484402a5fa8ea27ea492a3f))
ROM_END

ROM_START( rosco_m68k_020 )
	ROM_REGION32_BE(0x100000, "monitor", 0)
	ROM_LOAD( "rosco_m68k.rom", 0x00000, 0x100000, CRC(e7502a9b) SHA1(c729b5e2dd78de1d3484402a5fa8ea27ea492a3f))
ROM_END

ROM_START( rosco_m68k_030 )
	ROM_REGION32_BE(0x100000, "monitor", 0)
	ROM_LOAD( "rosco_m68k.rom", 0x00000, 0x100000, CRC(e7502a9b) SHA1(c729b5e2dd78de1d3484402a5fa8ea27ea492a3f))
ROM_END


/* Driver */
/*    YEAR  NAME            PARENT  COMPAT  MACHINE         INPUT       CLASS             INIT        COMPANY  			FULLNAME                     FLAGS */
COMP( 2022, rosco_m68k_000, 0,      0,      rosco_m68k_000, rosco_m68k, rosco_m68k_000_state, empty_init, "The Really Old-School Company", "rosco_m68k Classic V2 000", MACHINE_NO_SOUND_HW )
COMP( 2022, rosco_m68k_010, 0,      0,      rosco_m68k_010, rosco_m68k, rosco_m68k_010_state, empty_init, "The Really Old-School Company", "rosco_m68k Classic V2 010", MACHINE_NO_SOUND_HW )
COMP( 2023, rosco_m68k_020, 0,      0,      rosco_m68k_020, rosco_m68k, rosco_m68k_020_state, empty_init, "The Really Old-School Company", "rosco_m68k Classic V2 020", MACHINE_NO_SOUND_HW )
COMP( 2023, rosco_m68k_030, 0,      0,      rosco_m68k_030, rosco_m68k, rosco_m68k_030_state, empty_init, "The Really Old-School Company", "rosco_m68k Classic V2 030", MACHINE_NO_SOUND_HW )