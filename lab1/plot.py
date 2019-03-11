import matplotlib.pyplot as plt
import json
import os

series_size = 25

dyn_number_of_threads = [1, 2, 3, 4, 5, 6, 7, 8]
dyn_execution_time = []
stc_execution_time = []
gui_execution_time = []

for filename in os.listdir("."):
    if filename.endswith(".json"):
        json_object = json.load(open(filename, "r"))
        final_exec_time = 0
        ex_time = 0        
        sch_t = json_object[0]["schedule_type"]
        #chunk_size = json_object[0]["number_of_chunks"]
        for i in range(0, series_size):            
            final_exec_time += json_object[i]["execution_time"]
        ex_time = final_exec_time / series_size
        if filename.endswith("dynamic_schedule.json"):
            dyn_execution_time.append(ex_time)
        if filename.endswith("static_schedule.json"):   
            stc_execution_time.append(ex_time)
        if filename.endswith("guided_schedule.json"):                         
            gui_execution_time.append(ex_time)           
        continue
    else:
        continue

print(" Type of schedule | Number of threads | Execution Time (ms) ")
print()
sch_t = "STATIC"
for i in range(0,8):
	print('{0:{align}{width}}{1:{align}{width}}{2:{align}{width}.2f}'.
		format(sch_t, i, stc_execution_time[i], align='^', width='20'))
	sch_t = ""
print()
sch_t = "DYNAMIC"
for i in range(0,8):
	print('{0:{align}{width}}{1:{align}{width}}{2:{align}{width}.2f}'.
		format(sch_t, i, dyn_execution_time[i], align='^', width='20'))
	sch_t = ""
print()
sch_t = "GUIDED"
for i in range(0,8):
	print('{0:{align}{width}}{1:{align}{width}}{2:{align}{width}.2f}'.
		format(sch_t, i, gui_execution_time[i], align='^', width='20'))
	sch_t = ""



fig = plt.figure()
fig.suptitle('Dependency of execution time from number of threads on average', fontsize=14)
plt.ylabel('Execution time (ms)', fontsize=14)
plt.xlabel('Number of threads (count)', fontsize=14)
s, = plt.plot(dyn_number_of_threads, stc_execution_time, 'bo--', label='static')
d, = plt.plot(dyn_number_of_threads, dyn_execution_time, 'ro--', label='dynamic')
g, = plt.plot(dyn_number_of_threads, gui_execution_time, 'go--', label='guided')
plt.legend(handles=[s, d, g])
plt.show()