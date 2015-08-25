from ryu.base import app_manager
from ryu.controller import ofp_event, dpset
from ryu.controller.handler import MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls

class Controller(app_manager.RyuApp):
    def __init__(self, *args, **kwargs):
        super(Controller, self).__init__(*args, **kwargs)


    @set_ev_cls(dpset.EventDP, MAIN_DISPATCHER)
    def switch_in(self, ev):
	#print self
	#print ev
        dp  = ev.dp
        entered = ev.enter
        if ev.enter:
            self.install_rules(dp)

    def dest_host_pod(vlan_id):
	dest = vlan_id % 16
	if (dest == 0):
		return 4
	if (dest >= 1 and dest <= 4):
		return 1
	elif (dest >= 5 and dest <= 8):
		return 2
	elif (dest >= 9 and dest <= 12):
		return 3
	elif ((dest >= 13 and dest <= 16) or (dest == 0)):
		return 4

    def switch_pod(dpid):
	if (dpid >=5 and dpid <= 8):
		return 1
	if (dpid >=9 and dpid <= 12):
		return 2
	if (dpid >=13 and dpid <= 16):
		return 3
	if (dpid >=17 and dpid <= 20):
		return 4

    def is_aggregate_switch(dpid):
	if (dpid == 5 or dpid == 6 or dpid == 9 or dpid == 10 or dpid == 13 or dpid == 14 or dpid == 17 or dpid == 18):
		return True
	return False


    def is_edge_switch(dpid):
	if (dpid == 7 or dpid == 8 or dpid == 11 or dpid == 12 or dpid == 15 or dpid == 16 or dpid == 19 or dpid == 20):
		return True
	return False


    def is_core_switch(dpid):
	if (dpid == 1 or dpid == 2 or dpid == 3 or dpid == 4):
		return True
	return False
    
    def get_aggregate_port(dpid, vlan_id):
	src = vlan_id / 16
	dest = vlan_id % 16
	# same pod condition - switch and destination are in same pod.
	# if ((src - 1)/4 == (dest - 1)/4):
	if (dest_host_pod(vlan_id) == switch_pod(dpid)):
		if ((dest - 1) % 4 <= 1):
			return 3
		elif ((dest - 1) % 4 >= 2):
			return 4
	
	else:		
		# return eth1. eth2 will remain unused in this case. All traffic from aggregate level switch will go to core switch via eth1 interface.
		if (dest % 2):
			return 2
		else:
			return 1
     

    def is_connected(dpid, dest):
	switch = [7,7,8,8,11,11,12,12,15,15,16,16,19,19,20,20]
	if (switch[dest - 1] == dpid):
		return True
	return False
	    


    def get_edge_port(dpid, vlan_id):
	print "ID in edge port : " + `dpid`
	src = vlan_id / 16
	dest = vlan_id % 16
	# if host is connected to switch
	if (is_connected(dpid, dest)):
		if (dest % 2):
			return 4
		else:
			return 3

	else:
		if (dest % 2):
			return 2
		else:
			return 1


    def install_rules(self, dp):
        ofp        = dp.ofproto
        ofp_parser = dp.ofproto_parser
	#print `dp.id` + ": ID"
        # Make sure the switch's forwarding table is empty
        dp.send_delete_all_flows()

	
    	def dest_host_pod(vlan_id):
		vlan_id = vlan_id % 1000;
		dest = vlan_id % 16
		if (dest == 0):
			return 4
		if (dest >= 1 and dest <= 4):
			return 1
		elif (dest >= 5 and dest <= 8):
			return 2
		elif (dest >= 9 and dest <= 12):
			return 3
		elif (dest >= 13 and dest <= 16):
			return 4
		return 0
    	
	def src_host_pod(vlan_id):
		src = vlan_id / 16
		if (vlan_id % 16 == 0):
			src = src - 1
		
		if (src >= 1 and src <= 4):
			return 1
		elif (src >= 5 and src <= 8):
			return 2
		elif (src >= 9 and src <= 12):
			return 3
		elif (src >= 13 and src <= 16):
			return 4
		return 0
	
	def switch_pod(dpid):
		if (dpid >=5 and dpid <= 8):
			return 1
		if (dpid >=9 and dpid <= 12):
			return 2
		if (dpid >=13 and dpid <= 16):
			return 3
		if (dpid >=17 and dpid <= 20):
			return 4
	

    	def is_core_switch(dpid):
		if (dpid == 1 or dpid == 2 or dpid == 3 or dpid == 4):
			return True
		return False
	
	
    	def is_aggregate_switch(dpid):
		if (dpid == 5 or dpid == 6 or dpid == 9 or dpid == 10 or dpid == 13 or dpid == 14 or dpid == 17 or dpid == 18):
			return True
		return False

	
    	def is_edge_switch(dpid):
		if (dpid == 7 or dpid == 8 or dpid == 11 or dpid == 12 or dpid == 15 or dpid == 16 or dpid == 19 or dpid == 20):
			return True
		return False


	
    	def get_aggregate_port(dpid, vlan_id):
		temp_vlanid = vlan_id % 1000
		dest = temp_vlanid % 16
		if(dest == 0):
			dest = 16
		
		# same pod condition - switch and destination are in same pod.
		# if ((src - 1)/4 == (dest - 1)/4):
		if (dest_host_pod(temp_vlanid) == switch_pod(dpid)):
			if ((dest - 1) % 4 <= 1):
				return 3
			elif ((dest - 1) % 4 >= 2):
				return 4
	
		else:
			if (vlan_id < 1000):
				return 1			
			elif (vlan_id < 2000):
				return 1
			elif (vlan_id < 3000):
				return 2
			elif (vlan_id < 4000):
				return 2

					


    	def is_connected(dpid, dest):
		switch = [7,7,8,8,11,11,12,12,15,15,16,16,19,19,20,20]
		if (switch[dest - 1] == dpid):
			return True
		return False
	    


	def get_edge_port(dpid, vlan_id):
		temp_vlanid = vlan_id % 1000;	
		src = temp_vlanid / 16
		dest = temp_vlanid % 16
		if(dest == 0):
			src = src - 1
			dest = 16
		
		# If Dest is directly connected with switch	
		if (is_connected(dpid, dest)):
			if (dest % 2 == 0):
				return 4
			else:
				return 3
		else:
			if (vlan_id < 1000):
				return 1			
			elif (vlan_id < 2000):
				return 2
			elif (vlan_id < 3000):
				return 1
			elif (vlan_id < 4000):
				return 2


	def install_rules(dpid, vlan_id, switch_level):
		switch = [7,7,8,8,11,11,12,12,15,15,16,16,19,19,20,20]
		src = vlan_id / 16
		dest = vlan_id % 16
		if(dest == 0):
			src = src - 1
			dest = 16

		# Edge Level Switches
		if (switch_level == 3):
			# If both source and dest are connected to same switch - 1 VLANID
			if(switch[src-1] == switch[dest-1]):
				from_port_to_port(vlan_id, int(get_edge_port(dpid, vlan_id)))
			# If both source and dest are in same pod - 2 VLANID's
			elif(src_host_pod(vlan_id) == dest_host_pod(vlan_id)):
				from_port_to_port(vlan_id, int(get_edge_port(dpid, vlan_id)))
				from_port_to_port(vlan_id + 1000, int(get_edge_port(dpid, vlan_id + 1000)))
			# If source and dest are in different pods - 4 VLANID's
			elif(src_host_pod(vlan_id) != dest_host_pod(vlan_id)):
				from_port_to_port(vlan_id, int(get_edge_port(dp.id, vlan_id)))		
				from_port_to_port(vlan_id + 1000, int(get_edge_port(dp.id, vlan_id + 1000)))		
				from_port_to_port(vlan_id + 2000, int(get_edge_port(dp.id, vlan_id + 2000)))		
				from_port_to_port(vlan_id + 3000, int(get_edge_port(dp.id, vlan_id + 3000)))		
					
		# Aggregate Level Switches
		elif(switch_level == 2):
			# If both source and dest are connected to same switch - No Rule
			if(switch[src-1] == switch[dest-1]):
				return
			# If both source and dest are in same pod - 2 VLANID's
			elif(src_host_pod(vlan_id) == dest_host_pod(vlan_id)):
				from_port_to_port(vlan_id, int(get_aggregate_port(dp.id, vlan_id)))		
				from_port_to_port(vlan_id + 1000, int(get_aggregate_port(dp.id, vlan_id + 1000)))		
			# If source and dest are in different pods - 4 VLANID's
			elif(src_host_pod(vlan_id) != dest_host_pod(vlan_id)):
				from_port_to_port(vlan_id, int(get_aggregate_port(dp.id, vlan_id)))		
				from_port_to_port(vlan_id + 1000, int(get_aggregate_port(dp.id, vlan_id + 1000)))		
				from_port_to_port(vlan_id + 2000, int(get_aggregate_port(dp.id, vlan_id + 2000)))		
				from_port_to_port(vlan_id + 3000, int(get_aggregate_port(dp.id, vlan_id + 3000)))		
	
		# Core Switches
		elif (switch_level == 1):	
			# If both source and dest are connected to same switch - No Rule
			if(switch[src-1] == switch[dest-1]):
				return
			# If both source and dest are in same pod - No Rule
			elif(src_host_pod(vlan_id) == dest_host_pod(vlan_id)):
				return
			# If source and dest are in different pods - 4 VLANID's
			elif(src_host_pod(vlan_id) != dest_host_pod(vlan_id)):
				from_port_to_port(vlan_id, int(dest_host_pod(vlan_id)))
				from_port_to_port(vlan_id + 1000, int(dest_host_pod(vlan_id + 1000)))
				from_port_to_port(vlan_id + 2000, int(dest_host_pod(vlan_id + 2000)))
				from_port_to_port(vlan_id + 3000, int(dest_host_pod(vlan_id + 3000)))

	
        # Creates a rule that sends out packets coming
        # from port: inport to the port: outport
        def from_port_to_port(vlan, outport):
	 #   print `vlan` + " : VLAN"
            match   = ofp_parser.OFPMatch(dl_vlan = vlan)
            actions = [ofp_parser.OFPActionOutput(outport)]
            out     = ofp_parser.OFPFlowMod(
                    datapath=dp, cookie=0,
                    command=ofp.OFPFC_ADD,
                    match=match,
                    actions=actions)
            dp.send_msg(out)

        # Rules for different switches
	for vlan_id in range(18, 272):
		if (is_core_switch(dp.id)):
			install_rules(dp.id, vlan_id, 1)
		elif (is_aggregate_switch(dp.id)):
			install_rules(dp.id, vlan_id, 2)
		elif (is_edge_switch(dp.id)):
			install_rules(dp.id, vlan_id, 3)
		
