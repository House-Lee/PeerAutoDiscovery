import subprocess
# run shell Command
def RunCmd(command):    
    p = subprocess.Popen(command, shell=True , stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    result = ""
    while(True):
      retcode = p.poll()  # returns None while subprocess is running
      result += p.stdout.readline()
      if(retcode is not None):
        break
    return (result, retcode)

res,code = RunCmd("cd /home/house.lee/Workspace/virtualmachine_monitor/v1.0 && make clean && make")
print res
