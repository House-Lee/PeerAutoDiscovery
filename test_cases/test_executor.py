import common
import sys
import time

sock = common.create_connection("10.1.212.136" , 7017)
if sock is False:
    print "Connect to server failed"
    sys.exit(1)

last_pluse = common.pluse(sock)
print "[Pluse]" + str(last_pluse)

code,msg = common.auth(sock)
print "[Auth]" + str(code) + ":" + msg

tid,code,msg = common.Execute(sock , "ls -al")
print "[Execute]" + str(code) + ":" + msg

time.sleep(2)
res,output = common.GetResult(sock , str(tid) )
print "----" + output + "----"

