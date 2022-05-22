target remote :3333
b gdisp.c:368

define reset
	print *Reset_Handler
	set $pc=$1
	set $lr=0xffffffff
	set $r0=0x0
	set $r2=0x0
	set $r3=0x0
	set $r4=0x0
	set $r5=0x0
	set $r6=0x0
	set $r7=0x0
	set $r8=0x0
	set $r9=0x0
	set $r10=0x0
	set $r11=0x0
	set $r12=0x0
end

define q
	quit
	y
end
