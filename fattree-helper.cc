
#include "ns3/object-factory.h"
#include "fattree-helper.h"
#include "ns3/names.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/network-module.h"
#include "ns3/linux-stack-helper.h"

NS_LOG_COMPONENT_DEFINE ("FattreeHelper");

#define FATTREE_DEFAULT_ARYSIZE		4
#define FATTREE_DEFAULT_LINKSPEED	"8Mbps"
#define FATTREE_DEFAULT_LINKDELAY	"1ms"

#define LINKSPEED	(this->linkspeed)
#define LINKDELAY	(this->linkdelay)


#define KARY		(this->kary)
#define KARY2		(KARY / 2)
#define PODNUM		KARY
#define AGGRSWINPODNUM	KARY2
#define EDGESWINPODNUM	KARY2
#define NODEINEDGENUM	KARY2
#define NODEINPODNUM	(NODEINEDGENUM * EDGESWINPODNUM)
#define ROOTSWNUM	(KARY2 * KARY2)
#define AGGRSWNUM	(AGGRSWINPODNUM * PODNUM)
#define EDGESWNUM	(EDGESWINPODNUM * PODNUM)
#define NODENUM		(NODEINPODNUM * PODNUM)

#define ROOTROOTSW	0       /* Root of Shortest Path Tree for default */
#define ROOTAGGRSW	0       /* Aggr of Shortest Path Tree for default */

#define ROOT2AGGRLINKS	(ROOTSWNUM * PODNUM)
#define AGGR2EDGELINKS	(AGGRSWINPODNUM * EDGESWINPODNUM * PODNUM)
#define EDGE2NODELINKS	(NODENUM)


/* Address Resolution Macros */

#define prefixlen "/24"

/* Link between root and aggrgation. root+1.pod+1.aggr+1.(1|2) */
#define ROOTAGGR_ROOTADDR(addr, root, pod, aggr, suffix)		\
  addr << root + 1 << "." << pod + 1 << "." << aggr + 1 << ".1" << suffix
#define ROOTAGGR_AGGRADDR(addr, root, pod, aggr, suffix)		\
  addr << root + 1 << "." << pod + 1 << "." << aggr + 1 << ".2" << suffix

/* Link between aggrgation and edge. pod+1+100.aggr+1.edge+1.(1|2) */
#define AGGREDGE_AGGRADDR(addr, pod, aggr, edge, suffix)		\
  addr << pod + 101 << "." << aggr + 1 << "." << edge + 1 << ".1" << suffix
#define AGGREDGE_EDGEADDR(addr, pod, aggr, edge, suffix)		\
  addr << pod + 101 << "." << aggr + 1 << "." << edge + 1 << ".2" << suffix

/* Link between edge and end node. pod+1+200.edge+1.node+1.(1|2) */
#define EDGENODE_EDGEADDR(addr, pod, edge, node, suffix)		\
  addr << pod + 201 << "." << edge + 1 << "." << node + 1 << ".1" << suffix
#define EDGENODE_NODEADDR(addr, pod, edge, node, suffix)		\
  addr << pod + 201 << "." << edge + 1 << "." << node + 1 << ".2" << suffix




namespace ns3 {

void
FattreeHelper::RunIp(Ptr<Node> node, Time at, std::string str)
{
  DceApplicationHelper process;
  ApplicationContainer apps;
  process.SetBinary("ip");
  process.SetStackSize(1 << 16);
  process.ResetArguments();
  process.ParseArguments(str.c_str());
  apps = process.Install(node);
  apps.Start(at);
}

void
FattreeHelper::AddAddress(Ptr<Node> node, Time at, int ifindex, 
			  const char *address)
{
  std::stringstream ss;
  ss << "-f inet addr add " << address
      << " dev sim" << ifindex;
  RunIp(node, at, ss.str());
}


void
FattreeHelper::AddLoAddress(Ptr<Node> node, Time at, const char *address)
{
  std::ostringstream oss;
  oss << "-f inet addr add " << address << " dev lo";
  RunIp(node, at, oss.str());
}

void
FattreeHelper::AddRoute(Ptr<Node> node, Time at,
			const char *dst, const char *next)
{
  std::ostringstream oss;
  oss << "-f inet route add to " << dst << " via " << next;
  RunIp(node, at, oss.str());
}
  
void
FattreeHelper::LinkUp(Ptr<Node> n1, Ptr<Node> n2,
		      NetDeviceContainer ndc, Time at)
{
  std::stringstream simup1, simup2;
  simup1 << "link set sim" << ndc.Get(0)->GetIfIndex() << " up";
  simup2 << "link set sim" << ndc.Get(1)->GetIfIndex() << " up";

  printf ("simup1 : %s\n", simup1.str().c_str());
  printf ("simup2 : %s\n", simup2.str().c_str());

  RunIp (n1, Seconds (at), simup1.str());
  RunIp (n2, Seconds (at), simup2.str());
  return;
}


FattreeHelper::FattreeHelper ()
{
  this->kary = FATTREE_DEFAULT_ARYSIZE;
  strcpy (LINKSPEED, FATTREE_DEFAULT_LINKSPEED);
  strcpy (LINKDELAY, FATTREE_DEFAULT_LINKDELAY);
}

void
FattreeHelper::SetAttribute (std::string name, const AttributeValue &value)
{
}

void
FattreeHelper::SetArySize (int kary)
{
  if (kary % 2 != 0) {
    NS_LOG_INFO ("invalid kary size\n");
    return;
  }
  this->kary = kary;

  return;
}



void
FattreeHelper::Create ()
{

  /* create instances of all switch nodes */
  this->rootsw.Create (ROOTSWNUM);
  this->aggrsw.Create (AGGRSWNUM);
  this->edgesw.Create (EDGESWNUM);
  this->nodes.Create (NODENUM);

  DceManagerHelper processManager;
  processManager.SetNetworkStack("ns3::LinuxSocketFdFactory", "Library",
				 StringValue ("liblinux.so"));
  processManager.Install(rootsw);
  processManager.Install(aggrsw);
  processManager.Install(edgesw);
  processManager.Install(nodes);

  LinuxStackHelper stack;
  stack.Install(rootsw);
  stack.Install(aggrsw);
  stack.Install(edgesw);
  stack.Install(nodes);

  /* set up links between root and aggregation switches */
  for (int pod = 0; pod < PODNUM; pod++) {
    for (int root = 0; root < ROOTSWNUM; root++) {

      int aggr = (int)(root / KARY2);
      int aggrn = AGGRSWINPODNUM * pod + aggr;
      
      /* install link */
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute("DataRate", StringValue (LINKSPEED));
      p2p.SetChannelAttribute("Delay", StringValue (LINKDELAY));

      NodeContainer nc;
      NetDeviceContainer ndc;
      nc = NodeContainer (this->rootsw.Get(root), this->aggrsw.Get(aggrn));
      ndc = p2p.Install (nc);

      /* setup IP addresses */
      std::stringstream roota, aggra;
      ROOTAGGR_ROOTADDR (roota, root, pod, aggr, prefixlen);
      ROOTAGGR_AGGRADDR (aggra, root, pod, aggr, prefixlen);
      AddAddress (nc.Get(0), Seconds(0.1),
		  ndc.Get(0)->GetIfIndex(), roota.str().c_str());
      AddAddress (nc.Get(1), Seconds(0.1), ndc.Get(1)->GetIfIndex(),
		  aggra.str().c_str());

      LinkUp (nc.Get(0), nc.Get(1), ndc, Seconds (0.11));
    }
  }
  
  /* set up links between aggregation and edge switches */
  for (int pod = 0; pod < PODNUM; pod++) {
    for (int aggr = 0; aggr < AGGRSWINPODNUM; aggr++) {
      for (int edge = 0; edge < EDGESWINPODNUM; edge++) {

	int aggrn = AGGRSWINPODNUM * pod + aggr;
	int edgen = EDGESWINPODNUM * pod + edge;
	
	/* install link */
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue (LINKSPEED));
	p2p.SetChannelAttribute("Delay", StringValue (LINKDELAY));

	NodeContainer nc;
	NetDeviceContainer ndc;
	nc = NodeContainer (this->aggrsw.Get(aggrn), this->edgesw.Get(edgen));
	ndc = p2p.Install (nc);

	/* setup IP addresses */
	std::stringstream aggra, edgea;
	AGGREDGE_AGGRADDR (aggra, pod, aggr, edge, prefixlen);
	AGGREDGE_EDGEADDR (edgea, pod, aggr, edge, prefixlen);
	AddAddress (nc.Get(0), Seconds(0.11),
		    ndc.Get(0)->GetIfIndex(), aggra.str().c_str());
	AddAddress (nc.Get(1), Seconds(0.11),
		    ndc.Get(1)->GetIfIndex(), edgea.str().c_str());

	LinkUp (nc.Get(0), nc.Get(1), ndc, Seconds (0.12));
      }
    }
  }

  /* set up links between edge switches and end nodes */
  for (int pod = 0; pod < PODNUM; pod++) {
    for (int edge = 0; edge < EDGESWINPODNUM; edge++) {
      for (int node = 0; node < NODEINEDGENUM; node++) {
	
	int edgen = EDGESWINPODNUM * pod + edge;
	int noden = NODEINPODNUM * pod + NODEINEDGENUM * edge + node;

	/* install link */
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue (LINKSPEED));
	p2p.SetChannelAttribute("Delay", StringValue (LINKDELAY));

	NodeContainer nc;
	NetDeviceContainer ndc;
	nc = NodeContainer (this->edgesw.Get(edgen), this->nodes.Get(noden));
	ndc = p2p.Install (nc);

	/* setup IP addresses */
	std::stringstream edgea, nodea;
	EDGENODE_EDGEADDR (edgea, pod, edge, node, prefixlen);
	EDGENODE_NODEADDR (nodea, pod, edge, node, prefixlen);
	AddAddress (nc.Get(0), Seconds(0.12),
		    ndc.Get(0)->GetIfIndex(), edgea.str().c_str());
	AddAddress (nc.Get(1), Seconds(0.12),
		    ndc.Get(1)->GetIfIndex(), nodea.str().c_str());

	LinkUp (nc.Get(0), nc.Get(1), ndc, Seconds (0.13));
      }
    }
  }

  return;
}

void
FattreeHelper::InstallDownRoute(void)
{
  /* Install shortest path through index 0 root, aggregation, edge switch*/

  for (int pod = 0; pod < PODNUM; pod++) {
    for (int root = 0; root < ROOTSWNUM; root++) {

      /* set route from Root to Aggrgation. prefix is Pod+201.0.0.0/8 */

      int aggr = (int)(root / KARY2);
      int aggrn = AGGRSWINPODNUM * pod + aggr;

      std::stringstream podprefix, aggrsim;
      podprefix << pod + 201 << ".0.0.0/8";
      ROOTAGGR_AGGRADDR(aggrsim, root, pod, aggr, "");

      AddRoute (this->rootsw.Get (root), Seconds (0.2),
		podprefix.str().c_str(), aggrsim.str().c_str());
    }
  }

  /* set route between aggregation and edge switches */
  for (int pod = 0; pod < PODNUM; pod++) {
    for (int aggr = 0; aggr < AGGRSWINPODNUM; aggr++) {
      for (int edge = 0; edge < EDGESWINPODNUM; edge++) {

	int aggrn = AGGRSWINPODNUM * pod + aggr;
	int edgen = EDGESWINPODNUM * pod + edge;

	/* set route from aggr to edge
	 * Prefix is Pod + 201.Edge.0.0/16
	 */
	std::stringstream edgeprefix, edgesim;
	edgeprefix << pod + 201 << "." << edge + 1 << ".0.0/16";
	AGGREDGE_EDGEADDR (edgesim, pod, aggr, edge, "");
	AddRoute (this->aggrsw.Get (aggrn), Seconds (0.21),
		  edgeprefix.str().c_str(), edgesim.str().c_str());
      }
    }
  }

  /* set route between edge switch and end nodes */
  for (int pod = 0; pod < PODNUM; pod++) {
    for (int edge = 0; edge < EDGESWINPODNUM; edge++) {
      for (int node = 0; node < NODEINEDGENUM; node++) {

	int edgen = EDGESWINPODNUM * pod + edge;
	int noden = NODEINPODNUM * pod + NODEINEDGENUM * edge + node;

	/* edge to node is connected route.
	 * set up default route to edge switch from end node.
	 */
	std::stringstream prefix, edgesim;
	prefix << "0.0.0.0/0";
	EDGENODE_EDGEADDR (edgesim, pod, edge, node, "");
	AddRoute (this->nodes.Get (noden), Seconds (0.22),
		  prefix.str().c_str(), edgesim.str().c_str());
      }
    }
  }

  /* default routes on aggregation and edg switches are isntalled by
   * InstallRoute() and InstallRouteECMP().
   */

  return;
}

void
FattreeHelper::InstallRoute ()
{
  /* Install shortest path through index 0 root, aggregation, edge switch*/

  /* install routes from Root to end node */
  InstallDownRoute ();

  /* install default routes */
  for (int pod = 0; pod < PODNUM; pod++) {

    int root = ROOTROOTSW;
    int aggr = (int)(root / KARY2);
    int aggrn = AGGRSWINPODNUM * pod + aggr;

    std::stringstream prefix, rootsim;
    prefix << "0.0.0.0/0";
    ROOTAGGR_ROOTADDR(rootsim, root, pod, aggr, "");
    AddRoute (this->aggrsw.Get (aggrn), Seconds (0.3),
	      prefix.str().c_str(), rootsim.str().c_str());
  }

  for (int pod = 0; pod < PODNUM; pod++) {
    for (int edge = 0; edge < EDGESWINPODNUM; edge++) {

      int aggr = ROOTAGGRSW;
      int aggrn = AGGRSWINPODNUM * pod + aggr;
      int edgen = EDGESWINPODNUM * pod + edge;

      /* set up default route from edge to aggr */
      std::stringstream prefix, aggrsim;
      prefix << "0.0.0.0/0";
      AGGREDGE_AGGRADDR (aggrsim, pod, aggr, edge, "");
      AddRoute (this->edgesw.Get (edgen), Seconds (0.31),
		prefix.str().c_str(), aggrsim.str().c_str());
    }
  }

  return;
}

void
FattreeHelper::InstallRouteECMP ()
{
  /* Install ECMP routes through index 0 root, aggregation, edge switch*/

  /* install routes from Root to end node */
  InstallDownRoute();

  /* set up ECMP routes. ECMPed route is Default route only.
   * From edge to aggregation, and aggregation to root.
   */

  /* setup ECMP for default route from aggr to root */
  for (int aggrn = 0; aggrn < AGGRSWNUM; aggrn++) {
    int pod = (int)(aggrn / AGGRSWINPODNUM);
    int aggr = aggrn % AGGRSWINPODNUM;

    std::stringstream route;
    route << "route add to default";

    for (int root = (KARY2 * aggr);
	 root < KARY2 * (aggr + 1); root++) {
      route << " nexthop via "
	    << root + 1 << "." << pod + 1 << "."
	    << aggr + 1 << "." << "1 weight 1";
    }
    printf ("aggr route %d, %s\n",
	    aggrn, route.str().c_str());
    RunIp (aggrsw.Get(aggrn), Seconds(0.41), route.str());
  }

  /* setup ECMP for deafult route from edge to aggr */
  for (int edgen = 0; edgen < EDGESWNUM; edgen++) {
    int pod = (int)(edgen / EDGESWINPODNUM);
    int edge = edgen % EDGESWINPODNUM;

    std::stringstream route;
    route << "route add to default";

    for (int aggr = 0;
	 aggr < AGGRSWINPODNUM; aggr++) {
      route << " nexthop via "
	    << pod + 101 << "." << aggr + 1 << "."
	    << edge + 1 << ".1 weight 1" ;
    }
    printf ("edge route %d, %s\n",
	    edgen, route.str().c_str());
    RunIp (edgesw.Get(edgen), Seconds(0.41), route.str());
  }

  return;
}


void
FattreeHelper::SetLinkSpeed (char *speed)
{
  strncpy (LINKSPEED, speed, 16);

  return;
}

void
FattreeHelper::SetLinkDelay (char *delay)
{
  strncpy (LINKDELAY, delay, 16);

  return;
}

int
FattreeHelper::GetRootSwitchNum (void)
{
  return ROOTSWNUM;
}

int
FattreeHelper::GetAggregationSwitchNum (void)
{
  return AGGRSWNUM;
}

int
FattreeHelper::GetEdgeSwitchNum (void)
{
  return EDGESWNUM;
}

int
FattreeHelper::GetNodeNum (void)
{
  return NODENUM;
}

Ptr<Node>
FattreeHelper::GetRootSwitch (int index)
{
  return this->rootsw.Get (index);
}

Ptr<Node>
FattreeHelper::GetAggregationSwitch (int index)
{
  return this->aggrsw.Get (index);
}

Ptr<Node>
FattreeHelper::GetEdgeSwitch (int index)
{
  return this->edgesw.Get (index);
}

Ptr<Node>
FattreeHelper::GetNode (int index)
{
  return this->nodes.Get (index);
}



} // namespace ns3
