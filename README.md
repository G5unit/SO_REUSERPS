# SO_REUSERPS
Linux kernel patch to route incoming connections/sockets to listening processes based on RPS network scaling Linux kernel capability. RPS should be enabled for this option to work. get_rps_cpu() function is not called, we are past that point when assigning a listener to incomming socket, instead we lookup cpu id and use that to route the incomming connection.

I believe there is a case to be made for configurable incoming linux socket routing. We should be able to route incoming socket connections to the process/thread we want, and not necessarily be limited to one assignment scheme. Currently SO_REUSEPORT provides socket connection balancing across listeners, ones that are listening on the same IP & port, yet does not provide ability to control socket assignment.

One way to achieve configurable socket assignment to listening processes is to introduce a new socket option SO_REUSERPS.
This option would work very similar to how RPS works. When this option is used, each listener (bind-ed to the same AF_NET address & port) would provide a bitmask of CPUs it wants to receive incoming socket/FD from (CPU is one kernel is processing the packet protocol layer on, and same one handing it off to user space application).

A table (reuserpsMatchTable) is kept where each index in the table corresponds to CPU #, and each table entry [one per CPU] is an array containing registered FDs. Entries into array are added to the end as processes register, and when removed from array, array is reshuffled to not have empty space between entries. Processes registering and deregistering to listen for incoming sockets is an action that is not done at high frequency, usually at startup, shutdown, in case of scaling, or recovery from some unexpected event.

RPS uses 128bit mask making use of 128 processors. SO_REUSERPS would use the same mask for CPU ids, so max number of indexes in reuserpsMatchTable is 128.
Maximum number or registered listeners per CPU governs the size of array which serves as each entry in reuserpsMatchTable. I will start off with 128 entries to keep the memory used low.

SO_REUSERPS would assign new connections to registered listeners in round robin fashion. This is the simplest way of doing so and does not necessarily require us knowing how many connections each listener is serving.
We could go a step further and have different assignment algorithms. A useful one would be to assign connections in top->down order. This way we load up the first listener in the array, and move on to next one when first one is full.

Being able to set a limit on number of connections per listener (process) is very desirable. With this, protecting the process from overload is achieved, and ensures load could be spread out across registered listeners evenly. 

First process that registers should have SO_REUSERPS option set. This triggers initialization of reuserpsMatchTable which is then attached to the incoming sock(kernel socket). If this option is not specified by first process registering, SO_REUSERPS is disabled. As an attempt at a security measure only processes that belong to the same user as the first registered process, are allowed to register.

When a new connection arrives, kernel socket handler would find CPU id it is running on, and lookup FD to hand connection to in reuserpsMatchTable as reuserpsMatchTable[CPU ID].next .
