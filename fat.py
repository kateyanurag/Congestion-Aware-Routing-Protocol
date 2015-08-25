from mininet.topo import Topo
class AskFatTree(Topo):
	def __init__(self):
		Topo.__init__(self)
		# Create the custom topo here by using:
		# self.addHost, self.addSwitch, and self.addLink

		switch = ['0','s1','s2','s3','s4','s5','s6','s7','s8','s9','s10','s11','s12','s13','s14','s15','s16','s17','s18','s19','s20']
                host = ['0','h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'h7', 'h8', 'h9', 'h10', 'h11', 'h12','h13','h14','h15', 'h16']		
		
		
#		switch[0] = self.addSwitch("");
#		host[0] = self.addHost("");
		i = 1
		while (i <=20):
			switch[i] = self.addSwitch("s" + `i`)
			i = i + 1

		i = 1
		while (i <= 16):
			host[i] = self.addHost("h" + `i`)
			i = i + 1

                i = 5
		while (i <= 18):
              		self.addLink(switch[i], switch[1])
                        self.addLink(switch[i], switch[2])
              		self.addLink(switch[i + 1], switch[3])
                        self.addLink(switch[i + 1], switch[4])
			i = i + 4
		i = 5
		j = 7
		while (i <= 18 and j <= 20):
			self.addLink(switch[i], switch[j])
			self.addLink(switch[i], switch[j + 1])
			self.addLink(switch[i + 1], switch[j])
			self.addLink(switch[i + 1], switch[j + 1])
			i = i + 4
			j = j + 4
		
		i = 1
		j = 7
		while (i <= 16 and j <= 20):
			self.addLink(host[i], switch[j])
			self.addLink(host[i + 1], switch[j])
			self.addLink(host[i + 2], switch[j + 1])
			self.addLink(host[i + 3], switch[j + 1])
			i = i + 4
			j = j + 4
				
	@classmethod
	def create(cls):
		return cls()

topos = {'fattopo': AskFatTree.create}

