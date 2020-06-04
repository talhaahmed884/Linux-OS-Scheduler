# Linux-OS-Scheduler
Scheduling algorithms for Linux OS

The Commands are saved in command.txt for running the stimulator.
There can be a slight difference in the completion time and hence in the Turn Around Time and waiting time as the processes
aren't synchronised completely so a slight change can occur due to the fact that a completely separate clock is being run
in exit state that doesn't stop running and it assign completion value to every process that exits after complete execution.
The SRTF is implemented by taking quantum as 1 but it could also be implemented by following the FCFS approach and but
instead of queues so whenever the ready state dispatch the prcoess, process with lowest running time would be dispatched,
keeping in view the process has arrived. But i have implemented it by taking quantum as 1 and a clock is being run in the
running state of SRTF so that after evert 5 seconds the current process is blocked. The clock is running continously and it
is totally independent. I have created separate running state files for each algorithm implementation so the code becomes
very easy to understand and only one file is executed according to the respective algorithm. The processes will continue
to the wait for input processes to execute it, so after all the processes in the file are executed, the processes won't
shutdown and would need to be ended by system monitor. In the multi threading part of the ready state, i have also included the insertion
of processes into the ready queue or min heap in the thread so it not only make it easy to understand but also improves the performance
of the whole stimulator.
