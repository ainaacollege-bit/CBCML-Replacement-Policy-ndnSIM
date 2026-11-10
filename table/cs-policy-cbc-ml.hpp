//
// CBC version 3
//

#ifndef NFD_DAEMON_TABLE_CS_POLICY_CBC_ML_HPP
#define NFD_DAEMON_TABLE_CS_POLICY_CBC_ML_HPP

#include "cs-policy.hpp"

#include <list>
#include <ns3/nstime.h>
#include <ns3/node.h>
#include <ns3/node-list.h>
#include "ns3/ndnSIM/model/ndn-l3-protocol.hpp"
#include "ns3/pointer.h"
#include "fw/forwarder.hpp"
#include "math.h"
#include "ns3/simulator.h"
#include <algorithm>
#include <fstream>
#include <queue>

using namespace std;

namespace nfd {
namespace cs {
namespace cbc_ml {

using Queue = std::list<Policy::EntryRef>;

enum QueueType {
  PRIMARY_QUEUE,
  SECONDARY_QUEUE,
  CBC_QUEUE
};

struct EntryInfo
{
  QueueType queueType;
  Queue::iterator queueIt;
  scheduler::EventId moveToSecondaryQueue;
  Name prefix;
  uint64_t link_cost;
  double last_access;
  double recency;
  double frequency;
  
  double popularity;
  //double linkCostRate;
  double cost_value;
};

class CbcMlPolicy final : public Policy
{
public:
  CbcMlPolicy();

  ~CbcMlPolicy() final;

public:
  static const std::string POLICY_NAME;

private:
  void
  doAfterInsert(EntryRef i) final;

  void
  doAfterRefresh(EntryRef i) final;

  void
  doBeforeErase(EntryRef i) final;

  void
  doBeforeUse(EntryRef i) final;

  void
  evictEntries() final;

private:
  /** \brief evicts one entry
   *  \pre CS is not empty
   */
  void
  evictOne();

  /** \brief attaches the entry to an appropriate queue
   *  \pre the entry is not in any queue
   */
  void
  attachQueue(EntryRef i);
  
  /** \brief detaches the entry from its current queue
   *  \post the entry is not in any queue
   */
  void
  detachQueue(EntryRef i);

  /** \brief moves an entry from SECONDARY_QUEUE to PRIMARY_QUEUE
   */
  void
  moveToSecondaryQueue(EntryRef i);
  
  /** \brief moves an entry from SECONDARY_QUEUE to PRIMARY_QUEUE
   */
  void
  moveToPrimaryQueue(EntryRef i);
  
  void
  updateCostValue(EntryRef i);
  
  void
  updateQueue();
  
  void
  printQueue();
  
  /** \brief get the link_cost of the data source
   */
  uint64_t
  getSourceDistance(EntryRef i);
  
  /** \brief get the current simulation time
   */
  double
  getCurrentTime();
   
  /**void
  updateLinkCostRatio() 
  {
    double averageLinkCost;
    for(auto it = m_queues[PRIMARY_QUEUE].begin(); it != m_queues[PRIMARY_QUEUE].end(); ++it) {
        averageLinkCost = averageLinkCost + m_entryInfoMap[*it]->link_cost;
    }
    averageLinkCost = averageLinkCost / this->getCs()->size();
    
    for(auto it = m_queues[PRIMARY_QUEUE].begin(); it != m_queues[PRIMARY_QUEUE].end(); ++it) {
        m_entryInfoMap[*it]->linkCostRate = m_entryInfoMap[*it]->link_cost / averageLinkCost;
    }
  };**/
  
    /** \brief calculate the popularity using EWMA
   */
  
  double 
  calculateEWMA(double value, double prevValue) {
    return ((1 - alpha) * prevValue) + (alpha * value);
  };
  
  void 
  writeInfo(EntryRef i) 
  {
    EntryInfo* entryInfo = m_entryInfoMap[i];
    std::ofstream outFile("data.dat", std::ios::app);
    
    if (ns3::Simulator::GetContext() < 1000) { // this check prevents 4535459 value of Context
      ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext()); 

      
      // Timestamp, node, prefix, cacheHit
      outFile 	<< getCurrentTime() << "\t"
              	<< node->GetId() << "\t"
              	<< entryInfo->prefix << "\t"
        	<< entryInfo->link_cost << "\t"
    	    	<< entryInfo->last_access << "\t" 
    	    	<< entryInfo->recency << "\t" 
    	    	<< entryInfo->frequency << "\t" 
    	    	<< entryInfo->popularity << "\t"
    	    	//<< entryInfo->linkCostRate << "\t"
    	    	<< entryInfo->cost_value
              << std::endl;
      
    }
    
    outFile.close();
  };

  private:
  Queue m_queues[CBC_QUEUE];
  std::map<EntryRef, EntryInfo*> m_entryInfoMap;
  
  double alpha = 0.125;  // Smoothing factor (can adjust this)
};

} // namespace cbc_ml

using cbc_ml::CbcMlPolicy;

} // namespace cs
} // namespace nfd

#endif // NFD_DAEMON_TABLE_CS_POLICY_CBC_HPP
