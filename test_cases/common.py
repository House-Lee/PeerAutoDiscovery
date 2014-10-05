import socket
import time
import struct
import hashlib
from threading import Thread,Lock
from common_msg_pb2 import *
from taskmsg_pb2 import *

socket_lock = Lock()

def send_request(sock , msgid , pkglen, package):
    print "sending msg{id:%d,len:%d}" %(msgid , pkglen)
    msghead = struct.pack("4siiii","v001",msgid,pkglen,0,0)
    sock.send(msghead + package)
    msghead = ""
    while len(msghead) < 20 :
        msghead += sock.recv(20 - len(msghead))
    ver,msgid,msglen,nouse1,nouse2 = struct.unpack("4siiii" , msghead)
    msg = ""
    while len(msg) < msglen:
        msg += sock.recv(msglen - len(msg))
    return msghead + msg

def create_connection(host , port):
    try:
        s = socket.socket(socket.AF_INET , socket.SOCK_STREAM )
        s.connect((host , port))
        return s
    except socket.error,msg:
        print "Socket Failed. Error Code:" + str(msg[0]) + ', Error Message:' + msg[1]
        return False
    
#inherit from Standard Thread Class
class myThread(Thread):
    def __init__(self , thread_id , func , args):
        Thread.__init__(self)
        self.tid_ = thread_id
        self.func_ = func
        self.args_ = args
    def run(self):
        self.func_(self.args_)

def close_connection(sock):
    socket_lock.acquire()
    sock.close()
    socket_lock.release()
    
#is_numeric alternative version
def is_numeric(val):
    try:
        float(val)
        return True
    except ValueError:
        return False
#isset alternative version
def isset( val ):
    try:
        val
        return True
    except NameError:
        return False
    
    
    
    
    
    
def pluse(sock):
    pluse_pb = Pluse()
    pluse_pb.timestamp = int(time.time())
    request = pluse_pb.SerializeToString()
    socket_lock.acquire()
    res = send_request(sock , 0 , len(request) , request)
    socket_lock.release()
    ver,msgid,msglen,nouse1,nouse2 = struct.unpack("4siiii" , res[0:20])
    pluse_result = Pluse()
    pluse_result.ParseFromString(res[20:])
    return pluse_result.timestamp

def auth(sock):
    auth = ReqAuth()
    auth.timestamp = int(time.time())
    auth.auth_key = "7054132"
    auth.sign = hashlib.md5(str(auth.timestamp) + auth.auth_key + "XYZ").hexdigest()
    request = auth.SerializeToString()
    socket_lock.acquire()
    res = send_request(sock , 1, len(request) , request)
    socket_lock.release()
    ver,msgid,msglen,nouse1,nouse2 = struct.unpack("4siiii" , res[0:20])
    auth_result = AckCommon()
    auth_result.ParseFromString(res[20:])
    return (auth_result.code , auth_result.msg)
#    return "auth code:%d\nserver response:%s" % (auth_result.code , auth_result.msg)

task_id = 10002
def Execute(sock, cmd):
    global task_id
    task_id += 1
    reqtask_pb = ReqExecuteTask()
    reqtask_pb.task_id = str(task_id)
    reqtask_pb.command = cmd
    request = reqtask_pb.SerializeToString()
    socket_lock.acquire()
    res = send_request(sock , 2, len(request) , request)
    socket_lock.release()
    result = AckCommon()
    result.ParseFromString(res[20:])
    return (task_id , result.code , result.msg)
#    return "Task ID:%s\nExecute result:%d\n Server Response:%s" %(str(task_id) , result.code , result.msg)

def GetResult(sock , tid):
    reqtask_pb = ReqTaskCommon()
    reqtask_pb.task_id = tid
    request = reqtask_pb.SerializeToString()
    socket_lock.acquire()
    res = send_request(sock , 3, len(request) , request)
    print len(res)
    socket_lock.release()
    result = AckTaskOutput()
    result.ParseFromString(res[20:])
    return (result.req_result , result.output)
#    return "Result:%d\nOutput:\n%s" % (result.req_result , result.output)
    


