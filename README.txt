 ============ PROJECT C README FILE ==================

1) Did you use code from anywhere ?
ANS : NO

2) Detailed changes from Project B to Project C
ANS : 	1] Use of Additional VLAN ID's
		In project B I used only one vlanid per source destination pair. In project C, I have used vlanid's in following ways
		- 4 VlanId's : For src-dest pair where source and destination are in different pods
		- 2 VlanId's : For src-dest pair where source and destination are in same pod but are not connected to same TOR switch
		- 1 VlanId : For src-dest pair where source and destination are in same pod and are conneccted to same TOR switch

	2] Time Slot allocation and Path Allocation Algorithm
		- I used the concept mentioned in Fastpass paper to minimize the FCT.
		- The arbiter receives requests from the hosts and process them in the increasing order of number of flows (with priority given to latency sensitive flows)
		- To optimize the algorithm, I used Heap Data structures to store the requests seperately for latency sensitive, throughtput sensitive as well as 
		sensitive to both throughtput and latency.
		- From the set of demands at an instance, the arbiter chooses a subset of demands which can be fulfilled in that time slot.
		- The subset of demands is choosen with following constraints
			- no source will transmit more than 200 packets
			- no destination will recieve more than 200 packets 
			- a maximum of 200 packets(of a particular vlanid 1xxx/2xxx/3xxx/4xxx) can be sent from a pod to a core switch
			- In a timeslot instance, the total number of packets in a pod should not exceed 400.
		- The arbiter then chooses a set of demands to be fulfilled with an appropriate vlani (depending on which vlanid is available for the timeslot
		for a src-dest pair)
		- These demands are then send to the corresponding source hosts.
		- The source hosts then transmit the packets.
		- As soon as a host finishes sending packetss to a destination host, it sends an acknowledgement to arbiter that it has finished
		sending packets in this timeslot
		- The arbiter after receiving such ack's from all the source hosts (in this timeslot) moves on to serving the requests for the next timeslot.
		- This process continues till all requests are served.
