test.so:	file format elf64-x86-64

.debug_abbrev contents:
Abbrev table for offset: 0x00000000
[1] DW_TAG_compile_unit	DW_CHILDREN_yes
	DW_AT_producer	DW_FORM_strp
	DW_AT_language	DW_FORM_data2
	DW_AT_name	DW_FORM_strp
	DW_AT_stmt_list	DW_FORM_sec_offset
	DW_AT_comp_dir	DW_FORM_strp
	DW_AT_GNU_pubnames	DW_FORM_flag_present
	DW_AT_low_pc	DW_FORM_addr
	DW_AT_high_pc	DW_FORM_data4

[2] DW_TAG_subprogram	DW_CHILDREN_yes
	DW_AT_low_pc	DW_FORM_addr
	DW_AT_high_pc	DW_FORM_data4
	DW_AT_frame_base	DW_FORM_exprloc
	DW_AT_name	DW_FORM_strp
	DW_AT_type	DW_FORM_ref4
	DW_AT_external	DW_FORM_flag_present

[3] DW_TAG_formal_parameter	DW_CHILDREN_no
	DW_AT_location	DW_FORM_sec_offset
	DW_AT_name	DW_FORM_strp
	DW_AT_type	DW_FORM_ref4

[4] DW_TAG_base_type	DW_CHILDREN_no
	DW_AT_name	DW_FORM_strp
	DW_AT_encoding	DW_FORM_data1
	DW_AT_byte_size	DW_FORM_data1

[5] DW_TAG_pointer_type	DW_CHILDREN_no

[6] DW_TAG_pointer_type	DW_CHILDREN_no
	DW_AT_type	DW_FORM_ref4


.debug_info contents:
0x00000000: Compile Unit: length = 0x0000009a, format = DWARF32, version = 0x0004, abbr_offset = 0x0000, addr_size = 0x08 (next unit at 0x0000009e)

0x0000000b: DW_TAG_compile_unit
              DW_AT_producer	("TVM")
              DW_AT_language	(DW_LANG_C)
              DW_AT_name	("main.tir")
              DW_AT_stmt_list	(0x00000000)
              DW_AT_comp_dir	(".")
              DW_AT_GNU_pubnames	(true)
              DW_AT_low_pc	(0x0000000000001100)
              DW_AT_high_pc	(0x0000000000001388)

0x0000002a:   DW_TAG_subprogram
                DW_AT_low_pc	(0x0000000000001100)
                DW_AT_high_pc	(0x0000000000001388)
                DW_AT_frame_base	(DW_OP_reg7 RSP)
                DW_AT_name	("main")
                DW_AT_type	(0x00000090 "int32")
                DW_AT_external	(true)

0x00000041:     DW_TAG_formal_parameter
                  DW_AT_location	(0x00000000: 
                     [0x0000000000001100, 0x0000000000001186): DW_OP_reg5 RDI
                     [0x0000000000001186, 0x0000000000001218): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
                     [0x0000000000001218, 0x0000000000001226): DW_OP_reg5 RDI
                     [0x0000000000001226, 0x000000000000122f): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
                     [0x000000000000122f, 0x000000000000123d): DW_OP_reg5 RDI
                     [0x000000000000123d, 0x0000000000001246): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
                     [0x0000000000001246, 0x0000000000001254): DW_OP_reg5 RDI
                     [0x0000000000001254, 0x000000000000125d): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
                     [0x000000000000125d, 0x000000000000126b): DW_OP_reg5 RDI
                     [0x000000000000126b, 0x0000000000001274): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
                     [0x0000000000001274, 0x0000000000001282): DW_OP_reg5 RDI
                     [0x0000000000001282, 0x000000000000128b): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
                     [0x000000000000128b, 0x0000000000001299): DW_OP_reg5 RDI
                     [0x0000000000001299, 0x0000000000001388): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value)
                  DW_AT_name	("arg1")
                  DW_AT_type	(0x00000097 "void *")

0x0000004e:     DW_TAG_formal_parameter
                  DW_AT_location	(0x0000012f: 
                     [0x0000000000001100, 0x0000000000001140): DW_OP_reg4 RSI
                     [0x0000000000001140, 0x0000000000001218): DW_OP_GNU_entry_value(DW_OP_reg4 RSI), DW_OP_stack_value
                     [0x0000000000001218, 0x0000000000001228): DW_OP_reg4 RSI
                     [0x0000000000001228, 0x000000000000122f): DW_OP_GNU_entry_value(DW_OP_reg4 RSI), DW_OP_stack_value
                     [0x000000000000122f, 0x000000000000123f): DW_OP_reg4 RSI
                     [0x000000000000123f, 0x0000000000001246): DW_OP_GNU_entry_value(DW_OP_reg4 RSI), DW_OP_stack_value
                     [0x0000000000001246, 0x0000000000001256): DW_OP_reg4 RSI
                     [0x0000000000001256, 0x0000000000001388): DW_OP_GNU_entry_value(DW_OP_reg4 RSI), DW_OP_stack_value)
                  DW_AT_name	("arg2")
                  DW_AT_type	(0x00000098 "int32 *")

0x0000005b:     DW_TAG_formal_parameter
                  DW_AT_location	(0x000001e3: 
                     [0x0000000000001100, 0x000000000000118a): DW_OP_reg1 RDX
                     [0x000000000000118a, 0x0000000000001218): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
                     [0x0000000000001218, 0x0000000000001228): DW_OP_reg1 RDX
                     [0x0000000000001228, 0x000000000000122f): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
                     [0x000000000000122f, 0x000000000000123f): DW_OP_reg1 RDX
                     [0x000000000000123f, 0x0000000000001246): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
                     [0x0000000000001246, 0x0000000000001256): DW_OP_reg1 RDX
                     [0x0000000000001256, 0x000000000000125d): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
                     [0x000000000000125d, 0x000000000000126d): DW_OP_reg1 RDX
                     [0x000000000000126d, 0x0000000000001274): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
                     [0x0000000000001274, 0x0000000000001284): DW_OP_reg1 RDX
                     [0x0000000000001284, 0x000000000000128b): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
                     [0x000000000000128b, 0x000000000000129b): DW_OP_reg1 RDX
                     [0x000000000000129b, 0x0000000000001388): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value)
                  DW_AT_name	("arg3")
                  DW_AT_type	(0x00000090 "int32")

0x00000068:     DW_TAG_formal_parameter
                  DW_AT_location	(0x00000312: 
                     [0x0000000000001100, 0x000000000000111a): DW_OP_reg2 RCX
                     [0x000000000000111a, 0x0000000000001218): DW_OP_GNU_entry_value(DW_OP_reg2 RCX), DW_OP_stack_value
                     [0x000000000000122f, 0x0000000000001246): DW_OP_GNU_entry_value(DW_OP_reg2 RCX), DW_OP_stack_value
                     [0x0000000000001246, 0x0000000000001256): DW_OP_reg2 RCX
                     [0x0000000000001256, 0x0000000000001388): DW_OP_GNU_entry_value(DW_OP_reg2 RCX), DW_OP_stack_value)
                  DW_AT_name	("arg4")
                  DW_AT_type	(0x00000097 "void *")

0x00000075:     DW_TAG_formal_parameter
                  DW_AT_location	(0x0000038a: 
                     [0x0000000000001100, 0x0000000000001182): DW_OP_reg8 R8
                     [0x0000000000001182, 0x0000000000001218): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
                     [0x0000000000001218, 0x0000000000001228): DW_OP_reg8 R8
                     [0x0000000000001228, 0x000000000000122f): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
                     [0x000000000000122f, 0x000000000000123f): DW_OP_reg8 R8
                     [0x000000000000123f, 0x0000000000001246): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
                     [0x0000000000001246, 0x0000000000001256): DW_OP_reg8 R8
                     [0x0000000000001256, 0x000000000000125d): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
                     [0x000000000000125d, 0x000000000000126d): DW_OP_reg8 R8
                     [0x000000000000126d, 0x0000000000001274): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
                     [0x0000000000001274, 0x0000000000001284): DW_OP_reg8 R8
                     [0x0000000000001284, 0x000000000000128b): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
                     [0x000000000000128b, 0x000000000000129b): DW_OP_reg8 R8
                     [0x000000000000129b, 0x0000000000001388): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value)
                  DW_AT_name	("arg5")
                  DW_AT_type	(0x00000098 "int32 *")

0x00000082:     DW_TAG_formal_parameter
                  DW_AT_location	(0x000004b9: 
                     [0x0000000000001100, 0x0000000000001211): DW_OP_reg9 R9
                     [0x0000000000001211, 0x0000000000001218): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x0000000000001218, 0x0000000000001228): DW_OP_reg9 R9
                     [0x0000000000001228, 0x000000000000122f): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x000000000000122f, 0x000000000000123f): DW_OP_reg9 R9
                     [0x000000000000123f, 0x0000000000001246): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x0000000000001246, 0x0000000000001256): DW_OP_reg9 R9
                     [0x0000000000001256, 0x000000000000125d): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x000000000000125d, 0x000000000000126d): DW_OP_reg9 R9
                     [0x000000000000126d, 0x0000000000001274): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x0000000000001274, 0x0000000000001284): DW_OP_reg9 R9
                     [0x0000000000001284, 0x000000000000128b): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x000000000000128b, 0x000000000000129b): DW_OP_reg9 R9
                     [0x000000000000129b, 0x00000000000012a2): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x00000000000012a2, 0x00000000000012b2): DW_OP_reg9 R9
                     [0x00000000000012b2, 0x00000000000012b9): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x00000000000012b9, 0x00000000000012c9): DW_OP_reg9 R9
                     [0x00000000000012c9, 0x00000000000012d0): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x00000000000012d0, 0x00000000000012e0): DW_OP_reg9 R9
                     [0x00000000000012e0, 0x00000000000012e7): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x00000000000012e7, 0x00000000000012f7): DW_OP_reg9 R9
                     [0x00000000000012f7, 0x00000000000012fe): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x00000000000012fe, 0x000000000000130e): DW_OP_reg9 R9
                     [0x000000000000130e, 0x0000000000001315): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x0000000000001315, 0x0000000000001325): DW_OP_reg9 R9
                     [0x0000000000001325, 0x000000000000132c): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x000000000000132c, 0x000000000000133c): DW_OP_reg9 R9
                     [0x000000000000133c, 0x0000000000001343): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x0000000000001343, 0x0000000000001353): DW_OP_reg9 R9
                     [0x0000000000001353, 0x000000000000135a): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x000000000000135a, 0x000000000000136a): DW_OP_reg9 R9
                     [0x000000000000136a, 0x0000000000001371): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
                     [0x0000000000001371, 0x0000000000001381): DW_OP_reg9 R9
                     [0x0000000000001381, 0x0000000000001388): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value)
                  DW_AT_name	("arg6")
                  DW_AT_type	(0x00000097 "void *")

0x0000008f:     NULL

0x00000090:   DW_TAG_base_type
                DW_AT_name	("int32")
                DW_AT_encoding	(DW_ATE_signed)
                DW_AT_byte_size	(0x04)

0x00000097:   DW_TAG_pointer_type

0x00000098:   DW_TAG_pointer_type
                DW_AT_type	(0x00000090 "int32")

0x0000009d:   NULL

.debug_loc contents:
0x00000000: 
            (0x0000000000000000, 0x0000000000000086): DW_OP_reg5 RDI
            (0x0000000000000086, 0x0000000000000118): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
            (0x0000000000000118, 0x0000000000000126): DW_OP_reg5 RDI
            (0x0000000000000126, 0x000000000000012f): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
            (0x000000000000012f, 0x000000000000013d): DW_OP_reg5 RDI
            (0x000000000000013d, 0x0000000000000146): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
            (0x0000000000000146, 0x0000000000000154): DW_OP_reg5 RDI
            (0x0000000000000154, 0x000000000000015d): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
            (0x000000000000015d, 0x000000000000016b): DW_OP_reg5 RDI
            (0x000000000000016b, 0x0000000000000174): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
            (0x0000000000000174, 0x0000000000000182): DW_OP_reg5 RDI
            (0x0000000000000182, 0x000000000000018b): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value
            (0x000000000000018b, 0x0000000000000199): DW_OP_reg5 RDI
            (0x0000000000000199, 0x0000000000000288): DW_OP_GNU_entry_value(DW_OP_reg5 RDI), DW_OP_stack_value

0x0000012f: 
            (0x0000000000000000, 0x0000000000000040): DW_OP_reg4 RSI
            (0x0000000000000040, 0x0000000000000118): DW_OP_GNU_entry_value(DW_OP_reg4 RSI), DW_OP_stack_value
            (0x0000000000000118, 0x0000000000000128): DW_OP_reg4 RSI
            (0x0000000000000128, 0x000000000000012f): DW_OP_GNU_entry_value(DW_OP_reg4 RSI), DW_OP_stack_value
            (0x000000000000012f, 0x000000000000013f): DW_OP_reg4 RSI
            (0x000000000000013f, 0x0000000000000146): DW_OP_GNU_entry_value(DW_OP_reg4 RSI), DW_OP_stack_value
            (0x0000000000000146, 0x0000000000000156): DW_OP_reg4 RSI
            (0x0000000000000156, 0x0000000000000288): DW_OP_GNU_entry_value(DW_OP_reg4 RSI), DW_OP_stack_value

0x000001e3: 
            (0x0000000000000000, 0x000000000000008a): DW_OP_reg1 RDX
            (0x000000000000008a, 0x0000000000000118): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
            (0x0000000000000118, 0x0000000000000128): DW_OP_reg1 RDX
            (0x0000000000000128, 0x000000000000012f): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
            (0x000000000000012f, 0x000000000000013f): DW_OP_reg1 RDX
            (0x000000000000013f, 0x0000000000000146): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
            (0x0000000000000146, 0x0000000000000156): DW_OP_reg1 RDX
            (0x0000000000000156, 0x000000000000015d): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
            (0x000000000000015d, 0x000000000000016d): DW_OP_reg1 RDX
            (0x000000000000016d, 0x0000000000000174): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
            (0x0000000000000174, 0x0000000000000184): DW_OP_reg1 RDX
            (0x0000000000000184, 0x000000000000018b): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value
            (0x000000000000018b, 0x000000000000019b): DW_OP_reg1 RDX
            (0x000000000000019b, 0x0000000000000288): DW_OP_GNU_entry_value(DW_OP_reg1 RDX), DW_OP_stack_value

0x00000312: 
            (0x0000000000000000, 0x000000000000001a): DW_OP_reg2 RCX
            (0x000000000000001a, 0x0000000000000118): DW_OP_GNU_entry_value(DW_OP_reg2 RCX), DW_OP_stack_value
            (0x000000000000012f, 0x0000000000000146): DW_OP_GNU_entry_value(DW_OP_reg2 RCX), DW_OP_stack_value
            (0x0000000000000146, 0x0000000000000156): DW_OP_reg2 RCX
            (0x0000000000000156, 0x0000000000000288): DW_OP_GNU_entry_value(DW_OP_reg2 RCX), DW_OP_stack_value

0x0000038a: 
            (0x0000000000000000, 0x0000000000000082): DW_OP_reg8 R8
            (0x0000000000000082, 0x0000000000000118): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
            (0x0000000000000118, 0x0000000000000128): DW_OP_reg8 R8
            (0x0000000000000128, 0x000000000000012f): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
            (0x000000000000012f, 0x000000000000013f): DW_OP_reg8 R8
            (0x000000000000013f, 0x0000000000000146): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
            (0x0000000000000146, 0x0000000000000156): DW_OP_reg8 R8
            (0x0000000000000156, 0x000000000000015d): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
            (0x000000000000015d, 0x000000000000016d): DW_OP_reg8 R8
            (0x000000000000016d, 0x0000000000000174): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
            (0x0000000000000174, 0x0000000000000184): DW_OP_reg8 R8
            (0x0000000000000184, 0x000000000000018b): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value
            (0x000000000000018b, 0x000000000000019b): DW_OP_reg8 R8
            (0x000000000000019b, 0x0000000000000288): DW_OP_GNU_entry_value(DW_OP_reg8 R8), DW_OP_stack_value

0x000004b9: 
            (0x0000000000000000, 0x0000000000000111): DW_OP_reg9 R9
            (0x0000000000000111, 0x0000000000000118): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x0000000000000118, 0x0000000000000128): DW_OP_reg9 R9
            (0x0000000000000128, 0x000000000000012f): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x000000000000012f, 0x000000000000013f): DW_OP_reg9 R9
            (0x000000000000013f, 0x0000000000000146): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x0000000000000146, 0x0000000000000156): DW_OP_reg9 R9
            (0x0000000000000156, 0x000000000000015d): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x000000000000015d, 0x000000000000016d): DW_OP_reg9 R9
            (0x000000000000016d, 0x0000000000000174): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x0000000000000174, 0x0000000000000184): DW_OP_reg9 R9
            (0x0000000000000184, 0x000000000000018b): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x000000000000018b, 0x000000000000019b): DW_OP_reg9 R9
            (0x000000000000019b, 0x00000000000001a2): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x00000000000001a2, 0x00000000000001b2): DW_OP_reg9 R9
            (0x00000000000001b2, 0x00000000000001b9): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x00000000000001b9, 0x00000000000001c9): DW_OP_reg9 R9
            (0x00000000000001c9, 0x00000000000001d0): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x00000000000001d0, 0x00000000000001e0): DW_OP_reg9 R9
            (0x00000000000001e0, 0x00000000000001e7): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x00000000000001e7, 0x00000000000001f7): DW_OP_reg9 R9
            (0x00000000000001f7, 0x00000000000001fe): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x00000000000001fe, 0x000000000000020e): DW_OP_reg9 R9
            (0x000000000000020e, 0x0000000000000215): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x0000000000000215, 0x0000000000000225): DW_OP_reg9 R9
            (0x0000000000000225, 0x000000000000022c): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x000000000000022c, 0x000000000000023c): DW_OP_reg9 R9
            (0x000000000000023c, 0x0000000000000243): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x0000000000000243, 0x0000000000000253): DW_OP_reg9 R9
            (0x0000000000000253, 0x000000000000025a): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x000000000000025a, 0x000000000000026a): DW_OP_reg9 R9
            (0x000000000000026a, 0x0000000000000271): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value
            (0x0000000000000271, 0x0000000000000281): DW_OP_reg9 R9
            (0x0000000000000281, 0x0000000000000288): DW_OP_GNU_entry_value(DW_OP_reg9 R9), DW_OP_stack_value

.eh_frame contents:

00000000 00000014 00000000 CIE
  Format:                DWARF32
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: 16
  Augmentation data:     1B

  DW_CFA_def_cfa: RSP +8
  DW_CFA_offset: RIP -8
  DW_CFA_nop:
  DW_CFA_nop:

  CFA=RSP+8: RIP=[CFA-8]

00000018 00000024 0000001c FDE cie=00000000 pc=00001020...00001030
  Format:       DWARF32
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 6
  DW_CFA_def_cfa_offset: +24
  DW_CFA_advance_loc: 10
  DW_CFA_def_cfa_expression: DW_OP_breg7 RSP+8, DW_OP_breg16 RIP+0, DW_OP_lit15, DW_OP_and, DW_OP_lit11, DW_OP_ge, DW_OP_lit3, DW_OP_shl, DW_OP_plus
  DW_CFA_nop:
  DW_CFA_nop:
  DW_CFA_nop:
  DW_CFA_nop:

  0x1020: CFA=RSP+16: RIP=[CFA-8]
  0x1026: CFA=RSP+24: RIP=[CFA-8]
  0x1030: CFA=DW_OP_breg7 RSP+8, DW_OP_breg16 RIP+0, DW_OP_lit15, DW_OP_and, DW_OP_lit11, DW_OP_ge, DW_OP_lit3, DW_OP_shl, DW_OP_plus: RIP=[CFA-8]

00000040 00000014 00000044 FDE cie=00000000 pc=00001030...00001038
  Format:       DWARF32
  DW_CFA_nop:
  DW_CFA_nop:
  DW_CFA_nop:
  DW_CFA_nop:
  DW_CFA_nop:
  DW_CFA_nop:
  DW_CFA_nop:

  0x1030: CFA=RSP+8: RIP=[CFA-8]

00000058 00000078 0000005c FDE cie=00000000 pc=00001100...00001388
  Format:       DWARF32
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc2: 278
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_advance_loc: 1
  DW_CFA_def_cfa_offset: +16
  DW_CFA_advance_loc: 22
  DW_CFA_def_cfa_offset: +8
  DW_CFA_nop:
  DW_CFA_nop:
  DW_CFA_nop:

  0x1100: CFA=RSP+8: RIP=[CFA-8]
  0x1101: CFA=RSP+16: RIP=[CFA-8]
  0x1217: CFA=RSP+8: RIP=[CFA-8]
  0x1218: CFA=RSP+16: RIP=[CFA-8]
  0x122e: CFA=RSP+8: RIP=[CFA-8]
  0x122f: CFA=RSP+16: RIP=[CFA-8]
  0x1245: CFA=RSP+8: RIP=[CFA-8]
  0x1246: CFA=RSP+16: RIP=[CFA-8]
  0x125c: CFA=RSP+8: RIP=[CFA-8]
  0x125d: CFA=RSP+16: RIP=[CFA-8]
  0x1273: CFA=RSP+8: RIP=[CFA-8]
  0x1274: CFA=RSP+16: RIP=[CFA-8]
  0x128a: CFA=RSP+8: RIP=[CFA-8]
  0x128b: CFA=RSP+16: RIP=[CFA-8]
  0x12a1: CFA=RSP+8: RIP=[CFA-8]
  0x12a2: CFA=RSP+16: RIP=[CFA-8]
  0x12b8: CFA=RSP+8: RIP=[CFA-8]
  0x12b9: CFA=RSP+16: RIP=[CFA-8]
  0x12cf: CFA=RSP+8: RIP=[CFA-8]
  0x12d0: CFA=RSP+16: RIP=[CFA-8]
  0x12e6: CFA=RSP+8: RIP=[CFA-8]
  0x12e7: CFA=RSP+16: RIP=[CFA-8]
  0x12fd: CFA=RSP+8: RIP=[CFA-8]
  0x12fe: CFA=RSP+16: RIP=[CFA-8]
  0x1314: CFA=RSP+8: RIP=[CFA-8]
  0x1315: CFA=RSP+16: RIP=[CFA-8]
  0x132b: CFA=RSP+8: RIP=[CFA-8]
  0x132c: CFA=RSP+16: RIP=[CFA-8]
  0x1342: CFA=RSP+8: RIP=[CFA-8]
  0x1343: CFA=RSP+16: RIP=[CFA-8]
  0x1359: CFA=RSP+8: RIP=[CFA-8]
  0x135a: CFA=RSP+16: RIP=[CFA-8]
  0x1370: CFA=RSP+8: RIP=[CFA-8]
  0x1371: CFA=RSP+16: RIP=[CFA-8]
  0x1387: CFA=RSP+8: RIP=[CFA-8]

000000d4 00000010 000000d8 FDE cie=00000000 pc=00001390...000013a0
  Format:       DWARF32
  DW_CFA_nop:
  DW_CFA_nop:
  DW_CFA_nop:

  0x1390: CFA=RSP+8: RIP=[CFA-8]

000000e8 ZERO terminator

.debug_line contents:
debug_line[0x00000000]
Line table prologue:
    total_length: 0x0000004b
          format: DWARF32
         version: 4
 prologue_length: 0x00000020
 min_inst_length: 1
max_ops_per_inst: 1
 default_is_stmt: 1
       line_base: -5
      line_range: 14
     opcode_base: 13
standard_opcode_lengths[DW_LNS_copy] = 0
standard_opcode_lengths[DW_LNS_advance_pc] = 1
standard_opcode_lengths[DW_LNS_advance_line] = 1
standard_opcode_lengths[DW_LNS_set_file] = 1
standard_opcode_lengths[DW_LNS_set_column] = 1
standard_opcode_lengths[DW_LNS_negate_stmt] = 0
standard_opcode_lengths[DW_LNS_set_basic_block] = 0
standard_opcode_lengths[DW_LNS_const_add_pc] = 0
standard_opcode_lengths[DW_LNS_fixed_advance_pc] = 1
standard_opcode_lengths[DW_LNS_set_prologue_end] = 0
standard_opcode_lengths[DW_LNS_set_epilogue_begin] = 0
standard_opcode_lengths[DW_LNS_set_isa] = 1
file_names[  1]:
           name: "main.tir"
      dir_index: 0
       mod_time: 0x00000000
         length: 0x00000000

Address            Line   Column File   ISA Discriminator Flags
------------------ ------ ------ ------ --- ------------- -------------
0x0000000000001100      0      0      1   0             0  is_stmt
0x0000000000001101      5      6      1   0             0  is_stmt prologue_end
0x0000000000001115      0      6      1   0             0 
0x0000000000001126      5      6      1   0             0 
0x000000000000112f      0      6      1   0             0 
0x0000000000001140      5      6      1   0             0 
0x0000000000001169      0      6      1   0             0 
0x000000000000116d      5      6      1   0             0 
0x0000000000001176      0      6      1   0             0 
0x000000000000118a      5      6      1   0             0 
0x0000000000001388      5      6      1   0             0  end_sequence


.debug_str contents:
0x00000000: "TVM"
0x00000004: "main.tir"
0x0000000d: "."
0x0000000f: "main"
0x00000014: "int32"
0x0000001a: "arg1"
0x0000001f: "arg2"
0x00000024: "arg3"
0x00000029: "arg4"
0x0000002e: "arg5"
0x00000033: "arg6"

.debug_pubnames contents:
length = 0x00000017, format = DWARF32, version = 0x0002, unit_offset = 0x00000000, unit_size = 0x0000009e
Offset     Name
0x0000002a "main"

.debug_pubtypes contents:
length = 0x00000018, format = DWARF32, version = 0x0002, unit_offset = 0x00000000, unit_size = 0x0000009e
Offset     Name
0x00000090 "int32"
