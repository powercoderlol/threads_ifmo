@echo off
cls
Set Counter=0
:start
if %Counter% == 10 ( goto end )
mpiexec -n 1 mpi_jacobi.exe -3 input_data.txt first_precision.txt 0 >> 4000report_1_threads.json
Set /A Counter+=1
goto start
:end
echo "finished"
pause