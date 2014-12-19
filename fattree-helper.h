/*
 *
 */

#ifndef FATTREE_HELPER_H
#define FATTREE_HELPER_H

#include "ns3/dce-manager-helper.h"
#include "ns3/dce-application-helper.h"
#include "ns3/net-device-container.h"
#include "ns3/attribute.h"
#include "ns3/string.h"

namespace ns3 {

class FattreeHelper
{
public:
  
  /**
   * Create a FattreeHelper which is used to make life easier for people
   * wanting to use Fat-tree Topology.
   */
  FattreeHelper ();


  /**
   * Set kary size. defualt 4.
   */
  void SetArySize (int kary);
  
  /**
   * \brief Configure ping applications attribute
   *
   * \param name   attribute's name
   * \param value  attribute's value
   */
  void SetAttribute (std::string name, const AttributeValue &value);
  
  /**
   * Create all nodes and links.
   */
  void Create ();

  /**
   * Install shortest path tree static routes.
   */
  void InstallRoute ();

  /**
   * Install shortest path tree route with ECMP.
   */
  void InstallRouteECMP ();

  /**
   * Set link speed.
   */
  void SetLinkSpeed (char *speed);

  /**
   * Set link delay.
   */
  void SetLinkDelay (char *delay);

  /**
   * Get number of root swich.
   */
  int GetRootSwitchNum (void);

  /**
   * Get number of aggregation swich.
   */
  int GetAggregationSwitchNum (void);

  /**
   * Get number of edge swich.
   */
  int GetEdgeSwitchNum (void);

  /**
   * Get number of end nodes.
   */
  int GetNodeNum (void);

  /**
   * Get Root switch.
   */
  Ptr<Node> GetRootSwitch (int index);

  /**
   * Get Aggregation switch.
   */
  Ptr<Node> GetAggregationSwitch (int index);

  /**
   * Get Edge switch.
   */
  Ptr<Node> GetEdgeSwitch (int index);

  /**
   * Get end node.
   */
  Ptr<Node> GetNode (int index);


private:
  static void RunIp(Ptr<Node> node, Time at, std::string str);
  static void AddAddress(Ptr<Node> node, Time at, int ifindex,
			 const char *address);
  static void AddLoAddress(Ptr<Node> node, Time at, const char *address);
  static void AddRoute(Ptr<Node> node, Time at, 
		       const char *dst, const char *next);
  static void LinkUp(NodeContainer nc, NetDeviceContainer ndc, Time at);

  void InstallDownRoute(void);
  
  /**
   * \internal
   */
  int kary;

  char linkspeed[16];
  char linkdelay[16];

  NodeContainer rootsw;
  NodeContainer aggrsw;
  NodeContainer edgesw;
  NodeContainer nodes;

}; // FattreeHelper
  
} // namespace ns3


#endif /* FATTREE_HELPER_H */
