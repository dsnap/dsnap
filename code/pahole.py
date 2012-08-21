import subprocess as sp
import re

def get_map(arg):
    map = {}
    lines = get_class(arg).split("\n")
    for line in lines:
        
        match = re.search("\t(.+[\w\d*])\s\s+(\w.*);.+/*\s+(\d+)\s+(\d+)",line)

        if match:

            t,name,offset,size =match.group(1),match.group(2),match.group(3),match.group(4)
#            print line,t,name,offset,size
            map[int(offset)] = (t,name,int(size))
 #   print map
    if map != {}:
        return map
    else:
        return None
            
def get_class(arg):
    return run_pahole(["-C",str(arg)])



def run_pahole(args=[]):
    #list of args passed in
    args =["pahole"]+args+["e1000.ko"]
    
    val=sp.check_output(args)
    return val


#get_map("e1000_rx_ring")
