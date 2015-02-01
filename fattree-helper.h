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
   * Install routing table on all switches and nodes. The route follows
   * shortest path tree.
   */
  void InstallRoute ();

  /**
   * Install routing table on all switches and nodes. All multipath links
   * are used as Equal Cost Multi path.
   */
  void InstallRouteECMP ();

  /**
   * Set link speed of all links. SetLinkSpeed must be called beofre Create ().
   *
   * \param speed Link speed attirubte of ns-3.
   */
  void SetLinkSpeed (char *speed);

  /**
   * Set link delay of all links. SetLinkDelay must be called before Create ().
   *
   * \param delay Link delay attribute of ns-3.
   */
  void SetLinkDelay (char *delay);

  /**
   * Get number of root swich.
   *
   * \returns The number of root switches.
   */
  int GetRootN (void);

  /**
   * Get number of aggregation swich.
   *
   * \returns The number of aggregation switches.
   */
  int GetAggrN (void);

  /**
   * Get number of edge swich.
   *
   * \returns The number of edge swithces.
   */
  int GetEdgeN (void);

  /**
   * Get number of end nodes.
   *
   * \returns The number of end nodes.
   */
  int GetNodeN (void);

  /**
   * Get Root switch.
   *
   * \param index The index of root switch.
   * \returns A pointer of the root switch node.
   */
  Ptr<Node> GetRoot (int index);

  /**
   * Get Aggregation switch.
   *
   * \param index The index of aggregation switch.
   * \returns A pointer of the aggregation switch node.
   */
  Ptr<Node> GetAggr (int index);

  /**
   * Get Edge switch.
   *
   * \param index The index of edge switch.
   * \returns A pointer of the edge switch node.
   */
  Ptr<Node> GetEdge (int index);

  /**
   * Get end node.
   *
   * \param index The index of end node.
   * \returns A pointer of the end node.
   */
  Ptr<Node> GetNode (int index);


private:
  /**
   * \internal
   */

  static void RunIp (Ptr<Node> node, Time at, std::string str);
  static void AddAddress (Ptr<Node> node, Time at, int ifindex,
			  const char *address);
  static void AddLoAddress (Ptr<Node> node, Time at, const char *address);
  static void AddRoute (Ptr<Node> node, Time at,
			const char *dst, const char *next);
  static void LinkUp (NodeContainer nc, NetDeviceContainer ndc, Time at);
  void InstallDownRoute (void);


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
