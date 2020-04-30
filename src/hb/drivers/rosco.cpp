#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68901.h"
#include "imagedev/snapquik.h"
#include "formats/imageutl.h"


class rosco_state : public driver_device
{
public:
	rosco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
	{
	}

	void rosco(machine_config &config);
private:
	void rosco_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_ram;
};

/******************************************************************************
 Address Maps
******************************************************************************/

void rosco_state::rosco_map(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share("ram");
	map(0xf80000, 0xf8002f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xfc0000, 0xffffff).rom().region("monitor", 0);
}

/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( rosco )
INPUT_PORTS_END

static const input_device_default terminal_defaults[] =
{
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
	{ nullptr, 0, 0 }
};

/******************************************************************************
 Machine Start/Reset
******************************************************************************/

void rosco_state::machine_start()
{
}

void rosco_state::machine_reset()
{
	m_maincpu->set_pc(0xfc0000);
}

/******************************************************************************
 Machine Drivers
******************************************************************************/

QUICKLOAD_LOAD_MEMBER(rosco_state::quickload_cb)
{
	int quick_length;
	int read_;
	std::vector<uint8_t> temp_copy;

	quick_length = image.length();
	temp_copy.resize(quick_length);
	read_ = image.fread(&temp_copy[0], quick_length);
	if (read_ != quick_length) {
		return image_init_result::FAIL;
	}

	uint16_t *ROM16 = (uint16_t *) m_ram + 0x28000/2;
	for (int i = 0; i < quick_length; i += 2)
		ROM16[i / 2] = pick_integer_be(&temp_copy[0], i, 2);
	return image_init_result::PASS;
}

void rosco_state::rosco(machine_config &config)
{
	M68000(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &rosco_state::rosco_map);

	mc68901_device &mfp(MC68901(config, "mfp", 8_MHz_XTAL));
	mfp.set_timer_clock(3.6864_MHz_XTAL);
	mfp.out_so_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	mfp.out_tdo_cb().set("mfp", FUNC(mc68901_device::rc_w));
	mfp.out_tdo_cb().append("mfp", FUNC(mc68901_device::tc_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("mfp", FUNC(mc68901_device::si_w));
	rs232.set_option_device_input_defaults("terminal", terminal_defaults);

	/* quickload */
	QUICKLOAD(config, "quickload", "bin").set_load_callback(FUNC(rosco_state::quickload_cb));
}

/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( rosco )
	ROM_REGION16_BE(0x40000, "monitor", 0)
	ROM_LOAD( "serial_receive.rom.bin", 0x0000, 0x038d4, CRC(cf36efd9) SHA1(aca55e15559efa9aabb2db336f3bcd73074dfd2b))
ROM_END

COMP( 2020, rosco, 0, 0, rosco, rosco, rosco_state, empty_init, "Ross Bamford", "rosco-m68k", MACHINE_IS_SKELETON )
