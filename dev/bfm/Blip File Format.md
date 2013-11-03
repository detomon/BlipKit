Blip File Format
================

Commands / Data Types
---------------------

	0x00: End
	0x01: Group begin ([????;) ['????']
	0x02: Group end (])
	0x04: Integer (iN)
	0x06: String ('xyz')
	0x07: Data (x'HHHH')

	0x40: Attack
	0x41: Release
	0x42: Mute

Example
-------

	"[????" => !01 '????'
	"]"     => !04


	'BFM1'
	[blip i1

		!1005 i255    // global volume
		!1006 i16     // global speed
		!1007 i1 i240 // speed rate (1/240 sec per tick; default)

		[trck i4
			[ptrn
				!
			]
		]

	]


	[BFM1:blip:1;
	]
