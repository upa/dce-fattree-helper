
#include "fattree-helper.h"
#include "ns3/names.h"


NS_LOG_COMPONENT_DEFINE ("FattreeHelper");

#define FATTREE_DEFAULT_ARYSIZE	4

#define LINKSPEED "10mbps"

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

/* Link between root and aggrgation. root+1.pod+1.aggr+1.(1|2) */
#define ROOTAGGR_ROOTADDR(addr, root, pod, aggr) \
  addr << root + 1 << "." << pod + 1 << "." << aggr + 1 << "." << "1/24"
#define ROOTAGGR_AGGRADDR(addr, root, pod, aggr) \
  addr << root + 1 << "." << pod + 1 << "." << aggr + 1 << "." << "2/24"

/* Link between aggrgation and edge. pod+1+100.aggr+1.edge+1.(1|2) */
#define AGGREDGE_AGGRADDR(addr, pod, aggr, edge) \
  addr << pod + 101 << "." << aggr + 1 << "." << edge + 1 << "1/24"
#define AGGREDGE_EDGEADDR(addr, pod, aggr, edge) \
  addr << pod + 101 << "." << aggr + 1 << "." << edge + 1 << "2/24"

/* Link between edge and end node. pod+1+200.edge+1.node+1.(1|2) */
#define EDGENODE_EDGEADDR(add, pod, edge, node) \
  addr << pod + 201 << "." << edge + 1 << "." << node + 1 << "1/24"
#define EDGENODE_NODEADDR(add, pod, edge, node) \
  addr << pod + 201 << "." << edge + 1 << "." << node + 1 << "2/24"




namespace ns3 {

static void
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

static void
FattreeHelper::AddAddress(Ptr<Node> node, Time at, int ifindex, 
			  const char *address)
{
  std::ostringstream oss;
  oss << "-f inet addr add " << address
      << " dev sim" << ifindex;
  RunIp(node, at, oss.str());
}


static void
FattreeHelper::AddLoAddress(Ptr<Node> node, Time at, const char *address)
{
  std::ostringstream oss;
  oss << "-f inet addr add " << address << " dev lo";
  RunIp(node, at, oss.str());
}

static void
FattreeHelper::AddRoute(Ptr<Node> node, Time at,
			const char *dst, const char *next)
{
  std::ostringstream oss;
  oss << "-f inet route add to " << dst << " via " << next;
  RunIp(node, at, oss.str());
}
  

void
FattreeHelper::FattreeHelper ()
{
  this->kary = FATTREE_DEFAULT_ARYSIZE;
}

void
FattreeHelper::SetATtribute (std::string name, const AttributeValue &value)
{
}

void
FattreeHelper::SetArySeize (int kary)
{
  if (kary % 2 != 0) {
    NS_LOG_INFO ("invalid kary size %d\n", kary);
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

      int linkn = PODNUM * pod + root;	/* link number */
      int aggr = (int)(root / KARY2);
      int aggrn = AGRSWINPODNUM * pod + aggr;
      
      /* install link */
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute("DataRate", StringValue (LINKSPEED));
      p2p.SetChannelAttribute("Delay", StringValue ("0ms"));

      NodeContainer nc;
      NetDeviceContainer ndc;
      nc = NodeContainer (this->rootsw.Get(root), this->aggrsw.Get(aggrn));
      ndc = p2p.Install (nc);

      /* setup IP addresses */
      std::stringstream rootaddr, aggraddr;
      ROOTAGGR_ROOTADDR (roota, root, pod, aggr);
      ROOTAGGR_AGGRADDR (aggra, root, pod, aggr);
      AddAddress (nc.Get(0), Seconds(0.1), ndc.Get(0)->GetIfIndex(), roota);
      AddAddress (nc.Get(0), Seconds(0.1), ndc.Get(1)->GetIfIndex(), aggra);
    }
  }
  
  /* set up links between aggregation and edge switches */
  for (int pod = 0; pod < PODNUM; pod++) {
    for (int aggr = 0; aggr < AGGRSWINPODNUM; aggr++) {
      for (int edge = 0; edge < EDGESWINPODNUM; edge++) {

	int aggrn = AGGRSWINPODNUM * pod + aggr;
	int edgen = EEDGESWINPODNUM * pod + edge;
	
	/* install link */
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue (LINKSPEED));
	p2p.SetChannelAttribute("Delay", StringValue ("0ms"));

	NodeContainer nc;
	NetDeviceContainer ndc;
	nc = NodeContainer (this->aggrsw.Get(aggrn), this->edgesw.Get(edgen));
	ndc = p2p.Install (nc);

	/* setup IP addresses */
	std::stringstream aggra, edgea;
	AGGREDGE_AGGRADDR (aggra, pod, aggr, edge);
	AGGREDGE_EDGEADDR (edgea, pod, aggr, edge);
	AddAddress (nc.Get(0), Seconds(0.2), ndc.Get(0)->GetIfIndex(), aggra);
	AddAddress (nc.Get(0), Seconds(0.2), ndc.Get(1)->GetIfIndex(), edgea);
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
	p2p.SetChannelAttribute("Delay", StringValue ("0ms"));

	NodeContainer nc;
	NetDeviceContainer ndc;
	nc = NodeContainer (this->edgesw.Get(edgen), this->nodes.Get(noden));
	ndc = p2p.Install (nc);

	/* setup IP addresses */
	std::stringstream edgea, noden;
	EDGENODE_EDGEADDR (edgea, pod, edge, node);
	EDGENODE_NODEADDR (edgea, pod, edge, node);
	AddAddress (nc.Get(0), Seconds(0.3), ndc.Get(0)->GetIfIndex(), edgea);
	AddAddress (nc.Get(0), Seconds(0.3), ndc.Get(1)->GetIfIndex(), nodea);
      }
    }
  }

  return;
}

void
FattreeHelper::InstallRoute ()
{
  
}


} // namespace ns3
