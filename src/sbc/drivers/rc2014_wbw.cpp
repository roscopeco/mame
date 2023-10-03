#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "bus/ata/ataintf.h"

class rc2014_wbw_state : public driver_device
{
public:
	rc2014_wbw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)		
		, m_bank_enabled(0)
		, m_maincpu(*this, "maincpu")
		, m_ata(*this, "ata")
		, m_region_maincpu(*this, "maincpu")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
	{ }

	void rc2014(machine_config &config);

private:
    void acia_irq_w(int state);
    uint8_t ide_cs0_r(offs_t offset);
    void ide_cs0_w(offs_t offset, uint8_t data);
    void bank_w(offs_t offset, uint8_t data);
    void bank_en_w(uint8_t data);
    void ram_w(uint8_t data);

	void mem_map(address_map &map);
	void mem_io(address_map &map);

	void update_banks();

	virtual void machine_start() override;
	virtual void machine_reset() override;
	
	uint8_t m_bank_reg[4];
	uint8_t m_bank_enabled;
	required_device<cpu_device> m_maincpu;
	required_device<ata_interface_device> m_ata;
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
};


void rc2014_wbw_state::acia_irq_w(int state)
{
	m_maincpu->set_input_line(0, state);
}

uint8_t rc2014_wbw_state::ide_cs0_r(offs_t offset)
{
	return m_ata->cs0_r(offset);
}

void rc2014_wbw_state::ide_cs0_w(offs_t offset, uint8_t data)
{
	m_ata->cs0_w(offset, data);
}

void rc2014_wbw_state::bank_w(offs_t offset, uint8_t data)
{
	m_bank_reg[offset & 3] = data & 0x3f;
	update_banks();
}

void rc2014_wbw_state::bank_en_w(uint8_t data)
{
	m_bank_enabled = data & 1;
	update_banks();
}

/******************************************************************************
 Machine Start/Reset
******************************************************************************/

void rc2014_wbw_state::machine_start()
{
}

void rc2014_wbw_state::update_banks()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint8_t *mem = m_region_maincpu->base();

	m_bank1->set_base(mem + (m_bank_reg[0] << 14));
	m_bank2->set_base(mem + (m_bank_reg[1] << 14));
	m_bank3->set_base(mem + (m_bank_reg[2] << 14));
	m_bank4->set_base(mem + (m_bank_reg[3] << 14));

	if (m_bank_reg[0] & 0x20) {
		space.install_write_bank(0x0000, 0x3fff, membank("bank1"));
	} else {
		space.unmap_write(0x0000, 0x3fff);
	}
	if (m_bank_reg[1] & 0x20) {
		space.install_write_bank(0x4000, 0x7fff, membank("bank2"));
	} else {
		space.unmap_write(0x4000, 0x7fff);
	}
	if (m_bank_reg[2] & 0x20) {
		space.install_write_bank(0x8000, 0xbfff, membank("bank3"));
	} else {
		space.unmap_write(0x8000, 0xbfff);
	}
	if (m_bank_reg[3] & 0x20) {
		space.install_write_bank(0xc000, 0xffff, membank("bank4"));
	} else {
		space.unmap_write(0xc000, 0xffff);
	}
}

void rc2014_wbw_state::machine_reset()
{
	m_bank_reg[0] = 0;
	m_bank_reg[1] = 1;
	m_bank_reg[2] = 32;
	m_bank_reg[3] = 33;
	m_bank_enabled = 0;
	update_banks();
}
/******************************************************************************
 Address Maps
******************************************************************************/

void rc2014_wbw_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

void rc2014_wbw_state::mem_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x81).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x10, 0x17).rw(FUNC(rc2014_wbw_state::ide_cs0_r), FUNC(rc2014_wbw_state::ide_cs0_w));
	map(0x78, 0x7b).w(FUNC(rc2014_wbw_state::bank_w));
	map(0x7c, 0x7f).w(FUNC(rc2014_wbw_state::bank_en_w));
}

/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( rc2014 )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

void rc2014_wbw_state::rc2014(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_PROGRAM, &rc2014_wbw_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &rc2014_wbw_state::mem_io);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 7372800));
	uart_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	uart_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));

	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	acia.irq_handler().set(FUNC(rc2014_wbw_state::acia_irq_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set("acia", FUNC(acia6850_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
}


/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( rc2014_wbw )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "rcz80_std.rom",    0x00000, 0x80000, CRC(6d6b60c5) SHA1(5c642cb3113bc0a51562dc92c8b46bde414adb6c))
ROM_END


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                         FULLNAME                            FLAGS
COMP( 2014, rc2014_wbw, 0,      0,      rc2014,  rc2014, rc2014_wbw_state, empty_init, "Spencer Owen", "RC2014 RomWBW", MACHINE_IS_SKELETON )
