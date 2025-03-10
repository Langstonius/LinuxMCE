/*
    Capcom A0015405
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"

class capcom_state : public driver_device
{
public:
	capcom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(capcom);
};


static ADDRESS_MAP_START( capcom_map, AS_PROGRAM, 16, capcom_state )
	AM_RANGE(0x0000, 0xffffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( capcom )
INPUT_PORTS_END

void capcom_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(capcom_state,capcom)
{
}

static MACHINE_CONFIG_START( capcom, capcom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16670000) // M68306
	MCFG_CPU_PROGRAM_MAP(capcom_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Airborne (03/96)
/-------------------------------------------------------------------*/
ROM_START(abv106)
	ROM_REGION16_BE(0x00500000, "user1",0)
	ROM_LOAD16_BYTE("u1l_v16.bin", 0x000001, 0x80000, CRC(59b258f1) SHA1(764496114609d65648e1c7b12409ec582037d8df))
	ROM_LOAD16_BYTE("u1h_v16.bin", 0x000000, 0x80000, CRC(a4571905) SHA1(62fabc45e81c49125c047c6e5d268d4093b860bc))
	ROM_LOAD16_BYTE("u2l_v10.bin", 0x400001, 0x80000, CRC(a15b1ec0) SHA1(673a283ddf670109a9728fefac2bcf493d70f23d))
	ROM_LOAD16_BYTE("u2h_v10.bin", 0x400000, 0x80000, CRC(c22e3338) SHA1(1a25c85a1ed59647c40f9a4d417d78cccff7e51c))
	ROM_REGION(0x00500000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_v10.bin", 0 , 0x100000, CRC(ca3c6954) SHA1(44345c0a720c78c312459425c54180a4c5413c0d))
	ROM_LOAD("u29_v10.bin", 0x100000, 0x100000, CRC(8989d566) SHA1(f1827fb5c1d917a324fffe2035e87fcca77f362f))
	ROM_LOAD("u30_v11.bin", 0x200000, 0x80000, CRC(e16f1c4d) SHA1(9aa0ff87c303c6a8c95ef1c0e5382abad6179e21))
	ROM_FILL( 0x300000, 0x100000,0xff)
ROM_END

ROM_START(abv106r)
	ROM_REGION16_BE(0x00500000, "user1",0)
	ROM_LOAD16_BYTE("u1l_v16i.bin", 0x000001, 0x80000, CRC(7d7d2d85) SHA1(5c83022d7c0b61b15455942b3bdd0cf89fc75b57))
	ROM_LOAD16_BYTE("u1h_v16i.bin", 0x000000, 0x80000, CRC(b9bc0c5a) SHA1(e6fc393b970a2c354e0b0150dafbbbea2a85b92d))
	ROM_LOAD16_BYTE("u2l_v10.bin", 0x400001, 0x80000, CRC(a15b1ec0) SHA1(673a283ddf670109a9728fefac2bcf493d70f23d))
	ROM_LOAD16_BYTE("u2h_v10.bin", 0x400000, 0x80000, CRC(c22e3338) SHA1(1a25c85a1ed59647c40f9a4d417d78cccff7e51c))
	ROM_REGION(0x00500000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_v10.bin", 0 , 0x100000, CRC(ca3c6954) SHA1(44345c0a720c78c312459425c54180a4c5413c0d))
	ROM_LOAD("u29_v10.bin", 0x100000, 0x100000, CRC(8989d566) SHA1(f1827fb5c1d917a324fffe2035e87fcca77f362f))
	ROM_LOAD("u30_v11.bin", 0x200000, 0x80000, CRC(e16f1c4d) SHA1(9aa0ff87c303c6a8c95ef1c0e5382abad6179e21))
	ROM_LOAD("u31_v11i.bin", 0x300000, 0x20000, CRC(57794507) SHA1(9ec7648d948893a37dcda3a9c5ff56c7ce725291))
ROM_END

/*-------------------------------------------------------------------
/ Big Bang Bar - Beta (11/96)
/-------------------------------------------------------------------*/
ROM_START(bbb109)
	ROM_REGION16_BE(0x00500000, "user1",0)
	ROM_LOAD16_BYTE("u1l_b19.bin", 0x000001, 0x80000, CRC(32be6cb0) SHA1(e6c73366d5b85c0e96878e275320f82004bb97b5))
	ROM_LOAD16_BYTE("u1h_b19.bin", 0x000000, 0x80000, CRC(2bd1c06d) SHA1(ba81faa07b9d53f51bb981a82aa8684905516420))
	ROM_LOAD16_BYTE("u2l_b17.bin", 0x400001, 0x80000, CRC(9bebf271) SHA1(01c85396b96ffb04e445c03d6d2d88cce7835664))
	ROM_LOAD16_BYTE("u2h_b17.bin", 0x400000, 0x80000, CRC(afd36d9c) SHA1(b9f68b1e5792e293b9b8549dce0379ed3d8d2ceb))
	ROM_REGION(0x00500000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_b17.bin", 0 , 0x100000, CRC(af47c0f0) SHA1(09f84b9d1399183298279dfac95367741d6304e5))
	ROM_LOAD("u29_b17.bin", 0x100000, 0x100000, CRC(b5aa0d76) SHA1(c732fc76b992261da8475097adc70514e5a1c2e3))
	ROM_LOAD("u30_b17.bin", 0x200000, 0x80000, CRC(b4b6011b) SHA1(362c11353390f9ed2ee788847e6a2078b29c8806))
	ROM_LOAD("u31_b17.bin", 0x300000, 0x80000, CRC(3016563f) SHA1(432e89dd975559017771da3543e9fe36e425a32b))
ROM_END

ROM_START(bbb108)
	ROM_REGION16_BE(0x00500000, "user1",0)
	ROM_LOAD16_BYTE("u1l_b18.bin", 0x000001, 0x80000, CRC(60a02e1e) SHA1(c2967b4ba0ce01cb9f4ed5ceb4ca5f16596fc75b))
	ROM_LOAD16_BYTE("u1h_b18.bin", 0x000000, 0x80000, CRC(7a987a29) SHA1(5307b7feb8d86cf7dd51dd9c501b2539441b684e))
	ROM_LOAD16_BYTE("u2l_b17.bin", 0x400001, 0x80000, CRC(9bebf271) SHA1(01c85396b96ffb04e445c03d6d2d88cce7835664))
	ROM_LOAD16_BYTE("u2h_b17.bin", 0x400000, 0x80000, CRC(afd36d9c) SHA1(b9f68b1e5792e293b9b8549dce0379ed3d8d2ceb))
	ROM_REGION(0x00500000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_b17.bin", 0 , 0x100000, CRC(af47c0f0) SHA1(09f84b9d1399183298279dfac95367741d6304e5))
	ROM_LOAD("u29_b17.bin", 0x100000, 0x100000, CRC(b5aa0d76) SHA1(c732fc76b992261da8475097adc70514e5a1c2e3))
	ROM_LOAD("u30_b17.bin", 0x200000, 0x80000, CRC(b4b6011b) SHA1(362c11353390f9ed2ee788847e6a2078b29c8806))
	ROM_LOAD("u31_b17.bin", 0x300000, 0x80000, CRC(3016563f) SHA1(432e89dd975559017771da3543e9fe36e425a32b))
ROM_END

/*-------------------------------------------------------------------
/ Breakshot (05/96)
/-------------------------------------------------------------------*/
ROM_START(bsv103)
	ROM_REGION16_BE(0x00100000, "user1",0)
	ROM_LOAD16_BYTE("u1l_v13.bin", 0x000001, 0x80000, CRC(f8932dcc) SHA1(dab34e6c412655c60abeedc1f62254dce5ebb202))
	ROM_LOAD16_BYTE("u1h_v13.bin", 0x000000, 0x80000, CRC(508c145d) SHA1(b019d445f87bca203646c616fdc295066da90921))
	ROM_REGION(0x00100000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_v11.bin", 0 , 0x80000, CRC(b076ad2e) SHA1(1be8e3bda2890545253f6f7e4825d2db1d925255))
	ROM_LOAD("u29_v11.bin", 0x100000, 0x20000, CRC(b251a27c) SHA1(bc30791cb9b5497c11f1cff06c89a729a07b5d4a))
	ROM_FILL( 0x200000, 0x100000,0xff)
	ROM_FILL( 0x300000, 0x100000,0xff)
ROM_END

ROM_START(bsv100r)
	ROM_REGION16_BE(0x00100000, "user1",0)
	ROM_LOAD16_BYTE("u1l_v10i.bin", 0x000001, 0x80000, CRC(304b4da8) SHA1(2643f304adce3543b792bd2d0ec8abe8d9a5478c))
	ROM_LOAD16_BYTE("u1h_v10i.bin", 0x000000, 0x80000, CRC(c10b2aff) SHA1(a56af0903c8b8282baf63dc47741ef094cfd6a1c))
	ROM_REGION(0x00100000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_v11.bin", 0 , 0x80000, CRC(b076ad2e) SHA1(1be8e3bda2890545253f6f7e4825d2db1d925255))
	ROM_LOAD("u29_v11.bin", 0x100000, 0x20000, CRC(b251a27c) SHA1(bc30791cb9b5497c11f1cff06c89a729a07b5d4a))
	ROM_LOAD("u30_v10i.bin", 0x200000, 0x20000, CRC(8b7f6c41) SHA1(b564e5af3b60744df54f22940ab53956c4f89ee6))
	ROM_FILL( 0x300000, 0x100000,0xff)
ROM_END

ROM_START(bsv102r)
	ROM_REGION16_BE(0x00100000, "user1",0)
	ROM_LOAD16_BYTE("u1l_v12i.bin", 0x000001, 0x80000, CRC(ed09e463) SHA1(74b4e3e93648e05e66a20f895133f1a0ba2ecb20))
	ROM_LOAD16_BYTE("u1h_v12i.bin", 0x000000, 0x80000, CRC(71bf99e9) SHA1(cb48eb5c5df6b03022d9cb20c84dfdcc34a7d5ac))
	ROM_REGION(0x00100000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_v11.bin", 0 , 0x80000, CRC(b076ad2e) SHA1(1be8e3bda2890545253f6f7e4825d2db1d925255))
	ROM_LOAD("u29_v11.bin", 0x100000, 0x20000, CRC(b251a27c) SHA1(bc30791cb9b5497c11f1cff06c89a729a07b5d4a))
	ROM_LOAD("u30_v10i.bin", 0x200000, 0x20000, CRC(8b7f6c41) SHA1(b564e5af3b60744df54f22940ab53956c4f89ee6))
	ROM_FILL( 0x300000, 0x100000,0xff)
ROM_END

ROM_START(bsb105)
	ROM_REGION16_BE(0x00100000, "user1",0)
	ROM_LOAD16_BYTE("bsu1l_b1.05", 0x000001, 0x80000, CRC(053684c7) SHA1(cf104a6e9c245523e29989389654c12437d32776))
	ROM_LOAD16_BYTE("bsu1h_b1.05", 0x000000, 0x80000, CRC(f1dc6db8) SHA1(da209872047a1deac88fe389bcc26bcf353f6df8))
	ROM_REGION(0x00100000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("bsu28_b1.2", 0 , 0x80000, CRC(b65880be) SHA1(d42da68ab58f87516656315ad5d389a444a674ff))
	ROM_FILL( 0x100000, 0x100000,0xff)
	ROM_FILL( 0x200000, 0x100000,0xff)
	ROM_FILL( 0x300000, 0x100000,0xff)
ROM_END

/*-------------------------------------------------------------------
/ Flipper Football (10/96)
/-------------------------------------------------------------------*/
ROM_START(ffv104)
	ROM_REGION16_BE(0x00d00000, "user1",0)
	ROM_LOAD16_BYTE("u1l_v104.bin", 0x000001, 0x80000, CRC(375f4dd3) SHA1(0e3845afccf51a2d20e01afb371b8b7076a1ea79))
	ROM_LOAD16_BYTE("u1h_v104.bin", 0x000000, 0x80000, CRC(2133fc8e) SHA1(b4296f890a11aefdd09083636f416112e64fb0be))
	ROM_LOAD16_BYTE("u2l_v104.bin", 0x400001, 0x80000, CRC(b74175ae) SHA1(dd0279e20a2ccb03dbea0087ab9d15a973543553))
	ROM_LOAD16_BYTE("u2h_v104.bin", 0x400000, 0x80000, CRC(98621d17) SHA1(1656715930af09629b22569ec6b4cde537c2f83f))
	ROM_LOAD16_BYTE("u4l_v104.bin", 0x800001, 0x80000, CRC(912bc445) SHA1(01b80ba9353e6096066490943ca4a7c64131023d))
	ROM_LOAD16_BYTE("u4h_v104.bin", 0x800000, 0x80000, CRC(fb7012a9) SHA1(2e8717954dab0f30b59e716b5a47acf0f3feb379))
	ROM_LOAD16_BYTE("u3l_v104.bin", 0xc00001, 0x80000, CRC(aed63bd0) SHA1(06e4943cb06c5027abc1e63358c7c8c55344c8f3))
	ROM_LOAD16_BYTE("u3h_v104.bin", 0xc00000, 0x80000, CRC(9376881e) SHA1(a84c3fecefbc6fc455719c06bf6e77f81fbcb78c))
	ROM_REGION(0x00d00000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_v101.bin", 0 , 0x100000, CRC(68b896e0) SHA1(3d8c286d43c1db68c39fb4d130cd3cd679209a22))
	ROM_LOAD("u29_v101.bin", 0x100000, 0x80000, CRC(b79f3e58) SHA1(9abd570590216800bbfe9f12b4660fbe0200679e))
	ROM_LOAD("u30_v101.bin", 0x200000, 0x80000, CRC(f5432518) SHA1(8c26a267335289145f29db822bf7dfcb4730b208))
	ROM_LOAD("u31_v101.bin", 0x300000, 0x80000, CRC(2b14e032) SHA1(c423ae5ed2fcc582201606bac3e766ec332b395a))
ROM_END

ROM_START(ffv101)
	ROM_REGION16_BE(0x00d00000, "user1",0)
	ROM_LOAD16_BYTE("u1l_v100.bin", 0x000001, 0x80000, CRC(1c0b776f) SHA1(a1cabe9646973a97000a8f42295dfcfbed3691fa))
	ROM_LOAD16_BYTE("u1h_v100.bin", 0x000000, 0x80000, CRC(13590a38) SHA1(04cb048677b725a2563a12f18853372d5280d464))
	ROM_LOAD16_BYTE("u2l_v100.bin", 0x400001, 0x80000, CRC(ee373854) SHA1(cf4814c8c18ab5ca6bf3f134e3c3f95e6f8fe870))
	ROM_LOAD16_BYTE("u2h_v100.bin", 0x400000, 0x80000, CRC(8ebe3530) SHA1(903f38111482860eae44d5e601bbf26b50a40e2b))
	ROM_LOAD16_BYTE("u4l_v100.bin", 0x800001, 0x80000, CRC(55982601) SHA1(150c22e200855041746ac08f4817dd8d3a04f64d))
	ROM_LOAD16_BYTE("u4h_v100.bin", 0x800000, 0x80000, CRC(35b60875) SHA1(e3bc752f77cc6baeb7010e9e95bd10d4935e44da))
	ROM_LOAD16_BYTE("u3l_v101.bin", 0xc00001, 0x80000, CRC(03319c15) SHA1(dbbdcfe5baab3ec654ddf4b331d1332ec3e47c76))
	ROM_LOAD16_BYTE("u3h_v101.bin", 0xc00000, 0x80000, CRC(b55532cb) SHA1(b539a62d33eaa43057249450c40905b2d6fe1e1f))
	ROM_REGION(0x00d00000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_v100.bin", 0 , 0x100000, CRC(78c60574) SHA1(399a98b707b32096da5dc6c902ac10feca371433))
	ROM_LOAD("u29_v100.bin", 0x100000, 0x80000, CRC(8c37fbca) SHA1(5c3a3e1cc076e7a2732f3546005961d191040912))
	ROM_LOAD("u30_v100.bin", 0x200000, 0x80000, CRC(a92885a1) SHA1(b06453c710fd86e97567e70ab7558b0c2fd54c72))
	ROM_LOAD("u31_v100.bin", 0x300000, 0x80000, CRC(358c2727) SHA1(73ac6cc51a6ceb27934607909a0fff369a47ba7d))
ROM_END

/*-------------------------------------------------------------------
/ Kingpin (12/96)
/-------------------------------------------------------------------*/
ROM_START(kpv106)
	ROM_REGION16_BE(0x00500000, "user1",0)
	ROM_LOAD("u1hu1l.bin", 0x000000, 0x100000, CRC(d2d42121) SHA1(c731e0b5c9b211574dda8aecbad799bc180a59db))
	ROM_LOAD("u2hu2l.bin", 0x400000, 0x100000, CRC(9cd91371) SHA1(197a06a0ed6b661d798ed18b1db72215c28e1bc2))
	ROM_REGION(0x00500000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_b11.bin", 0 , 0x100000, CRC(aa480506) SHA1(4fbf384bc5e2d0eec4d1137784006d63091974ca))
	ROM_LOAD("u29_b11.bin", 0x100000, 0x100000, CRC(33345446) SHA1(d229d45228e13e2f02b73ce125eab4f2dd91db6e))
	ROM_LOAD("u30_b11.bin", 0x200000, 0x80000, CRC(fa35a177) SHA1(3c54c12db8e17a8c316a22b9b7ac80b6b3af8474))
	ROM_LOAD("u31_b18.bin", 0x300000, 0x80000, CRC(07a7d514) SHA1(be8cb4b6d70ccae7821110689b714612c8a0b460))
ROM_END

/*-------------------------------------------------------------------
/ Pinball Magic (10/95)
/-------------------------------------------------------------------*/
ROM_START(pmv112)
	ROM_REGION16_BE(0x00500000, "user1",0)
	ROM_LOAD16_BYTE("u1l_v112.bin", 0x000001, 0x80000, CRC(c8362623) SHA1(ebe37d3273e5cefd4fbc041ea3b15d59010b8160))
	ROM_LOAD16_BYTE("u1h_v112.bin", 0x000000, 0x80000, CRC(f6232c74) SHA1(28bab61de2ece27aff4cbdd36b10c136a4b7c936))
	ROM_LOAD16_BYTE("u2l_v10.bin", 0x400001, 0x80000, CRC(d3e4241d) SHA1(fe480ea2b3901e2e571f8871a0ebe63fbf152e28))
	ROM_LOAD16_BYTE("u2h_v10.bin", 0x400000, 0x80000, CRC(9276fd62) SHA1(b80e6186a6a2ded21bd1d6dbd306590645a50523))
	ROM_REGION(0x00500000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_v10.bin", 0 , 0x100000, CRC(5c12fc2f) SHA1(2e768fb1b5bf56f97af16c4e5542446ef740db58))
	ROM_LOAD("u29_v10.bin", 0x100000, 0x100000, CRC(74352bcd) SHA1(dc62fd651cf8408330f41b2e5387daecfe1d93d7))
	ROM_LOAD("u30_v16.bin", 0x200000, 0x80000, CRC(a7c29b8f) SHA1(1d623c3a67a8e4bf39c22bbf0e008fb2f8920351))
	ROM_FILL( 0x300000, 0x100000,0xff)
ROM_END

ROM_START(pmv112r)
	ROM_REGION16_BE(0x00500000, "user1",0)
	ROM_LOAD16_BYTE("u1lv112i.bin", 0x000001, 0x80000, CRC(28d35969) SHA1(e19856402855847286db73c17510614d8b40c882))
	ROM_LOAD16_BYTE("u1hv112i.bin", 0x000000, 0x80000, CRC(f70da65c) SHA1(0f98c95edd6f2821e3a67ff1805aa752a4d018c0))
	ROM_LOAD16_BYTE("u2l_v10.bin", 0x400001, 0x80000, CRC(d3e4241d) SHA1(fe480ea2b3901e2e571f8871a0ebe63fbf152e28))
	ROM_LOAD16_BYTE("u2h_v10.bin", 0x400000, 0x80000, CRC(9276fd62) SHA1(b80e6186a6a2ded21bd1d6dbd306590645a50523))
	ROM_REGION(0x00500000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "cpu2", 0)
	ROM_LOAD("u24_v11.bin", 0x0000, 0x2000, CRC(d46212f4) SHA1(50f1279d995b597c468805b323e0252800b28274))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("u28_v10.bin", 0 , 0x100000, CRC(5c12fc2f) SHA1(2e768fb1b5bf56f97af16c4e5542446ef740db58))
	ROM_LOAD("u29_v10.bin", 0x100000, 0x100000, CRC(74352bcd) SHA1(dc62fd651cf8408330f41b2e5387daecfe1d93d7))
	ROM_LOAD("u30_v16.bin", 0x200000, 0x80000, CRC(a7c29b8f) SHA1(1d623c3a67a8e4bf39c22bbf0e008fb2f8920351))
	ROM_LOAD("u31_v19i.bin", 0x300000, 0x20000, CRC(24735815) SHA1(6fbc1f86090ce42ea27805c700d8b132eafa271f))
ROM_END


/*-------------------------------------------------------------------
/ Unofficial
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Goofy Hoops (Romstar game) (??/95)
/-------------------------------------------------------------------*/
ROM_START(ghv101)
	ROM_REGION16_BE(0x00500000, "user1",0)
	ROM_LOAD16_BYTE("u06_v11.bin", 0x000001, 0x80000, CRC(3b6ab802))
	ROM_LOAD16_BYTE("u23_v11.bin", 0x000000, 0x80000, CRC(f6cac3aa))
	ROM_LOAD16_BYTE("u13_v10.bin", 0x400001, 0x80000, CRC(1712f21f))
	ROM_LOAD16_BYTE("u17_v10.bin", 0x400000, 0x80000, CRC(b6a39327))
	ROM_REGION(0x00500000, "maincpu", ROMREGION_ERASEFF)
ROM_END


GAME(1996,  abv106,     0,      capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Airborne",                     GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  abv106r,    abv106, capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Airborne (Redemption)",        GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  bbb109,     0,      capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Big Bang Bar (Beta 1.9 US)",   GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  bbb108,     bbb109, capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Big Bang Bar (Beta 1.8 US)",   GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  bsv103,     0,      capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Breakshot",                    GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  bsv100r,    bsv103, capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Breakshot (Redemption 1.0)",   GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  bsv102r,    bsv103, capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Breakshot (Redemption 1.2)",   GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  bsb105,     bsv103, capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Breakshot (Beta)",             GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  ffv104,     0,      capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Flipper Football (v1.04)",     GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  ffv101,     ffv104, capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Flipper Football (v1.01)",     GAME_IS_SKELETON_MECHANICAL)
GAME(1996,  kpv106,     0,      capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Kingpin (Pinball)",            GAME_IS_SKELETON_MECHANICAL)
GAME(1995,  pmv112,     0,      capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Pinball Magic",                GAME_IS_SKELETON_MECHANICAL)
GAME(1995,  pmv112r,    pmv112, capcom, capcom, capcom_state,   capcom, ROT0,   "Capcom",       "Pinball Magic (Redemption)",   GAME_IS_SKELETON_MECHANICAL)
GAME(1995,  ghv101,     0,      capcom, capcom, capcom_state,   capcom, ROT0,   "Romstar",      "Goofy Hoops",                  GAME_IS_SKELETON_MECHANICAL)
