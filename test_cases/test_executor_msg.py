import socket
import sys
import struct
import time
import hashlib
import common_msg_pb2

def communicate(i,length, content):
    global s
    print "sending " + str(i) + ":" + str(length)
    msg = struct.pack("4siiii","v001",i,length,0,0)
#    for i in range(1,1000):
    s.send(msg + content)
    return s.recv(10240)

try:
    s = socket.socket(socket.AF_INET , socket.SOCK_STREAM )
    host_ip = "10.1.212.136"
    port = 7016
    s.connect((host_ip , port))
    print "Socket Connected to " + host_ip + ":" + str(port)
    #Pluse
    pluse = common_msg_pb2.Pluse()
    pluse.timestamp = int(time.time())
    request = pluse.SerializeToString()
    res = communicate(0 , len(request) , request)
    ver,msgid,msglen,nouse1,nouse2 = struct.unpack("4siiii" , res[0:20])
    pluse_result = common_msg_pb2.Pluse()
    pluse_result.ParseFromString(res[20:])
    print "Pluse Result:" + ver + "-" + str(msgid) + "-" + str(msglen) + ":" + str(pluse_result.timestamp)
    
    #Auth
    auth = common_msg_pb2.ReqAuth()
    auth.timestamp = int(time.time())
    auth.auth_key = "rmbknyzxabcde"
    auth.sign = hashlib.md5(str(auth.timestamp) + auth.auth_key + "XYZ").hexdigest()
    request = auth.SerializeToString()
    res = communicate(1, len(request) , request)
    ver,msgid,msglen,nouse1,nouse2 = struct.unpack("4siiii" , res[0:20])
    auth_result = common_msg_pb2.AckCommon()
    auth_result.ParseFromString(res[20:])
    print "Auth Result:" + ver + "-" + str(msgid) + "-" + str(msglen) + ":" + str(auth_result.code) + auth_result.msg
    time.sleep(50)
    s.close()
except socket.error, msg:
    print "Socket Failed. Error Code:" + str(msg[0]) + ', Error Message:' + msg[1]
    sys.exit(1)
