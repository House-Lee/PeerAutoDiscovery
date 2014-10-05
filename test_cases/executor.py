from publiclib import *
import time
from threading import Lock

oplock = Lock()

def show_help(s , param):
    return "under construction..."
    
sock = False
def keep_connection(s):
    global sock
    while True:
        oplock.acquire()
        if sock is not False:
            pluse(sock , None)
        oplock.release()
        time.sleep(15)

def setup_connection(s , cmd):
    param = filter_cmd(cmd)
    if len(param) < 2:
        print "command error"
        return False
    global sock
    global oplock
    host = param[0]
    port = param[1]
    rtn = "ok"
    oplock.acquire()
    if sock is not False:
        close_connection(sock)
        rtn = "connect fail"
    sock = create_connection(host , int(port))
    oplock.release()
    auth(sock , None)
    return rtn
    
        
command_map = {
               "help":show_help,
               "execute":Execute,
               "execute_taskid":ExecuteWithTaskID,
               "getoutput":GetOutput,
               "getres":GetResult,
               "connect":setup_connection,
               "config_get":GetConfig,
               "config_set":SetConfig,
               }

def main():
    global command_map
    global sock;
    pluse_thread = myThread(1 , keep_connection , sock)
    pluse_thread.start()
    while True:
        input = raw_input(">")
        end_pos = input.find(' ')
        if end_pos == -1:
            end_pos = len(input)
        command = input[0:end_pos]
        param = input[input.find(command) + len(command) + 1:]
#        if not isset(command_map[command]):
        if not command_map.has_key(command):
            print "unrecognized command.[%s] type \"help\" for usage"%command
        else:
            rtn = command_map[command](sock , param)
            print rtn

#start main thread
if (__name__ == "__main__"):
    main()