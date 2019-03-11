import subprocess

f = open("m1.txt", "r")
scale = f.readline()
data = scale.split()
x = int(data[0])
f.close()
f = open("m2.txt", "r")
scale = f.readline()
data = scale.split()
y = int(data[1])
f.close()

arr = ["static", "dynamic", "guided"]

for k in range(0, 8):
    # static 8 threads 1 chunk
    # j = x * y // (k + 1)
    j = 1    
    for p in range(0, 3):
        filename = str(k + 1) + "_threads_" + str(j) + "_chunks_" + arr[p] + "_schedule.json"    
        result = "["
        first = True
        # final series        
        for i in range(0, 25):
            if not first:
                result += ", "
            var = subprocess.check_output([r"path_to_exe", "m1.txt", "m2.txt", str(p), str(j), str(k + 1), "0"])
            result += str(var, 'utf-8')
            if first:
                first = False
        result += "]"        
        #result = str(var, 'utf-8')
        f = open(filename,"w+")
        f.write(result)
        f.close()

