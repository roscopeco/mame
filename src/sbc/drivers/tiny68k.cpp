#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"

class tiny68k_state : public driver_device
{
public:
	tiny68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_duart(*this, "duart")
		, m_ram(*this, "ram")
	{
	}

	void tiny68k(machine_config &config);
private:
	void tiny68k_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE8_MEMBER(duart_output);
	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);

	void cpu_space_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_shared_ptr<uint16_t> m_ram;
};

/******************************************************************************
 Address Maps
******************************************************************************/

void tiny68k_state::tiny68k_map(address_map &map)
{
	map(0x000000, 0xff7fff).ram().share("ram");
	//map(0xffe000, 0xffefff) // IDE-CF
	map(0xfff000, 0xffffff).rw("duart", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
}

/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( tiny68k )
INPUT_PORTS_END

static const input_device_default terminal_defaults[] =
{
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_38400 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_38400 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
	{ nullptr, 0, 0 }
};

/******************************************************************************
 Machine Start/Reset
******************************************************************************/

void tiny68k_state::machine_start()
{
}

void tiny68k_state::machine_reset()
{
	// CPLD loads flash content to RAM
	uint16_t* ram = (uint16_t *) m_ram;
	uint16_t* rom = (uint16_t *) memregion("monitor")->base();

	memcpy(ram,rom,0x04000);
}

/******************************************************************************
 Machine Drivers
******************************************************************************/

WRITE8_MEMBER(tiny68k_state::duart_output)
{
//	printf("duart_output : %02x\n",data);
}

WRITE_LINE_MEMBER(tiny68k_state::duart_irq_handler)
{
	m_maincpu->set_input_line(M68K_IRQ_3, state);
}

void tiny68k_state::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff7, 0xfffff7).r(m_duart, FUNC(mc68681_device::get_irq_vector));
}

void tiny68k_state::tiny68k(machine_config &config)
{
	M68000(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tiny68k_state::tiny68k_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &tiny68k_state::cpu_space_map);


	MC68681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->a_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_duart->outport_cb().set(FUNC(tiny68k_state::duart_output));
	m_duart->irq_cb().set(FUNC(tiny68k_state::duart_irq_handler));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("duart", FUNC(mc68681_device::rx_a_w));
	rs232.set_option_device_input_defaults("terminal", terminal_defaults);
}

/******************************************************************************
 ROM Definitions
******************************************************************************/
ROM_START( tiny68k )
	ROM_REGION16_BE(0x4000, "monitor", 0)
	ROM_LOAD( "tiny68kbug_r07.bin", 0x0000, 0x3112, CRC(c8565009) SHA1(55de218ff304df405d31198540c42a9937ca8252))
ROM_END

COMP( 2017, tiny68k, 0, 0, tiny68k, tiny68k, tiny68k_state, empty_init, "Hui-chien Shen", "Tiny68k", MACHINE_IS_SKELETON )
