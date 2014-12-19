ns-3-dce FattreeHelper
======================

FattreeHelper provides helper class for constructing 3-level k-ary fat-tree 
topology on the ns-3 simulator environment. It requires ns-3 Direct
Code Execution (http://www.nsnam.org/overview/projects/direct-code-execution/).

3-level k-ary fat-tree topology is described on 
"A Scalable, Commodity Data Center Network Architecture",
Mohammad Al-Fares et.al. SIGCOMM'08. All switches and end nodes consist of 
Linux kernel stack (liblinux.so), and all links are IP link (switches
forward packet in layer-3).
