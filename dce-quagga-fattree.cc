
#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/quagga-helper.h"
#include "ns3/fattree-helper.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("DceQuaggaFattree");

static void
RunIp (Ptr<Node> node, Time at, std::string str)
{
  DceApplicationHelper process;
  ApplicationContainer apps;
  process.SetBinary ("ip");
  process.SetStackSize (1 << 16);
  process.ResetArguments ();
  process.ParseArguments (str.c_str ());
  apps = process.Install (node);
  apps.Start (at);
}



int
main (int argc, char **argv)
{
  int stop = 120;
  bool enable_ecmp = false;
  bool enable_quagga = true;
  CommandLine cmd;

  cmd.AddValue ("ecmp", "Install ECMP route.", enable_ecmp);
  cmd.AddValue ("quagga", "Using quagga on switches.", enable_quagga);
  cmd.Parse (argc, argv);

  /* set up fattree topology*/
  FattreeHelper ftree;
  ftree.Create ();

  if (!enable_quagga) {
    if (enable_ecmp) 
      ftree.InstallRouteECMP ();
    else
      ftree.InstallRoute ();
  }

  /* run quagga */
  if (enable_quagga) {
    QuaggaHelper quagga;

    for (int n = 0; n < ftree.GetRootSwitchNum (); n++) {
      quagga.EnableOspf (ftree.GetRootSwitch (n), "0.0.0.0/0");
      quagga.Install (ftree.GetRootSwitch (n));
    }

    for (int n = 0; n < ftree.GetAggregationSwitchNum (); n++) {
      quagga.EnableOspf (ftree.GetAggregationSwitch (n), "0.0.0.0/0");
      quagga.Install (ftree.GetAggregationSwitch (n));
    }

    for (int n = 0; n < ftree.GetEdgeSwitchNum (); n++) {
      quagga.EnableOspf (ftree.GetEdgeSwitch (n), "0.0.0.0/0");
      quagga.Install (ftree.GetEdgeSwitch (n));
    }
  }


  /* set show ip parameters */
  std::stringstream as, ls, rs;
  ls << "link show";
  as << "addr show";
  rs << "route show";

  for (int root = 0; root < ftree.GetRootSwitchNum (); root++) {
    RunIp (ftree.GetRootSwitch (root), Seconds (stop - 3), ls.str());
    RunIp (ftree.GetRootSwitch (root), Seconds (stop - 2), as.str());
    RunIp (ftree.GetRootSwitch (root), Seconds (stop - 1), rs.str());
  }

  for (int aggr = 0; aggr < ftree.GetAggregationSwitchNum (); aggr++) {
    RunIp (ftree.GetAggregationSwitch (aggr), Seconds (stop - 3), ls.str());
    RunIp (ftree.GetAggregationSwitch (aggr), Seconds (stop - 2), as.str());
    RunIp (ftree.GetAggregationSwitch (aggr), Seconds (stop - 1), rs.str());
  }

  for (int edge = 0; edge < ftree.GetEdgeSwitchNum (); edge++) {
    RunIp (ftree.GetEdgeSwitch (edge), Seconds (stop - 3), ls.str());
    RunIp (ftree.GetEdgeSwitch (edge), Seconds (stop - 2), as.str());
    RunIp (ftree.GetEdgeSwitch (edge), Seconds (stop - 1), rs.str());
  }

  for (int node = 0; node < ftree.GetNodeNum (); node++) {
    RunIp (ftree.GetNode (node), Seconds (stop - 3), ls.str());
    RunIp (ftree.GetNode (node), Seconds (stop - 2), as.str());
    RunIp (ftree.GetNode (node), Seconds (stop - 1), rs.str());
  }


  Simulator::Stop (Seconds (stop));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
