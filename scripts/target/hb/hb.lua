CPUS["Z80"] = true
MACHINES["Z80DAISY"] = true
MACHINES["Z80SIO"] = true
MACHINES["ACIA6850"] = true
MACHINES["CLOCK"] = true


CPUS["M680X0"] = true
MACHINES["MC68901"] = true

--------------------------------------
-- To be able to use serial devices --
--------------------------------------
CPUS["IE15"] = true
SOUNDS["BEEP"] = true
MACHINES["IE15"] = true
MACHINES["SWTPC8212"] = true
CPUS["M6800"] = true
BUSES["RS232"] = true
BUSES["SUNKBD"] = true
MACHINES["6821PIA"] = true
MACHINES["INS8250"] = true
MACHINES["INPUT_MERGER"] = true
VIDEOS["MC6845"] = true


function createProjects_hb_hb(_target, _subtarget)
	project ("hb_hb")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-hb-hb"))
	addprojectflags()
	precompiledheaders_novs()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

files{
	MAME_DIR .. "src/hb/drivers/rc2014.cpp",
	MAME_DIR .. "src/hb/drivers/rc2014_sio.cpp",
	MAME_DIR .. "src/hb/drivers/rosco.cpp",
}
end

function linkProjects_hb_hb(_target, _subtarget)
	links {
		"hb_hb",
	}
end