/*
 *
 */

#ifndef FATTREE_HELPER_H
#define FATTREE_HELPER_H

#include "ns3/dce-manager-helper.h"
#include "ns3/dce-application-helper.h"

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

  NodeContainer nodes GetRootNodes ();
  Ptr<Node> rootsw GetRootNode(int index);

  NodeContainer nodes GetAggrgationNodes ();
  Ptr<Node> aggrsw GetAggregatationNode(int index);

  NodeContainer nodes GetEdgeNodes ();
  Ptr<Node> edgesw GetEdgeNode(int index);

  NodeContainer nodes GetEndNodes ();
  Ptr<Node> edgesw GetEndNode(int index);


private:
  static void RunIp(Ptr<Node> node, Time at, std::string str);
  static void AddAddress(Ptr<Node> node, Time at, int ifindex,
			 const char *address);
  static void AddLoAddress(Ptr<Node> node, Time at, const char *address);
  static void AddRoute(Ptr<Node> node, Time at, 
		       const char *dst, const char *next);

  
  /**
   * \internal
   */
  int kary;
  char * rootloprefix;
  char * aggrloprefix;

  NodeContainer rootsw;
  NodeContainer aggrsw;
  NodeContainer edgesw;
  NodeContainer nodes;
}
  
} // namespace ns3

