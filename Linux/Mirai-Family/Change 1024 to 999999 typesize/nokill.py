#!/usr/bin/python
# Made by Kadens

import subprocess, time

def system(cmd):
  subprocess.call(cmd, shell=True)

print ("Fixing your killing problems! :)")
file = open("/proc/sys/fs/file-max", "w")
file.write ("999999999999999999")
file.close()
time.sleep(1)
print("File max was set up!")
system ("sed -i 's/1024/9999999/g' /usr/include/bits/typesizes.h")
system ("ulimit -n 99999")
time.sleep(1)
print("Typesizes file was edited!")
time.sleep(0.5)
print("Created by Kaden.")
time.sleep(0.3)
print("All done!")