//
// cs-policy-cbcml.cpp
//

#include "cs-policy-cbcml.hpp"
#include "cs.hpp"
#include "common/global.hpp"
#include "common/logger.hpp"
#include "fib.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <ndn-cxx/name.hpp> // Include the ndn-cxx Name header if necessary

NFD_LOG_INIT(CbcmlPolicy);

using namespace std;

namespace nfd {
namespace cs {
namespace cbcml {

const std::string CbcmlPolicy::POLICY_NAME = "cbcml";
NFD_REGISTER_CS_POLICY(CbcmlPolicy);

// Global map to store content popularity scores
std::unordered_map<ndn::Name, double> m_popularityScores;

// Function to load popularity scores from CSV
void LoadPopularityScores(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open popularity scores file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::string contentIdStr;
    double predictedScore;

    // Read each line from CSV
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        
        // Extract contentId as a string and then convert to ndn::Name
        if (std::getline(ss, contentIdStr, ',') && ss >> predictedScore) {
            ndn::Name contentId(contentIdStr);  // Convert string to ndn::Name
            m_popularityScores[contentId] = predictedScore;
        }
    }
    file.close();
}
CbcmlPolicy::CbcmlPolicy()
  : Policy(POLICY_NAME)
{
  // Load popularity scores when the policy is initialized
  LoadPopularityScores("predicted_popularity_scores_abilene.csv"); 
  
  // predicted_popularity_scores.csv for 5x5
  // predicted_popularity_scores_abilene.csv for Abilene

}

CbcmlPolicy::~CbcmlPolicy()
{
  for (auto entryInfoMapPair : m_entryInfoMap) {
    delete entryInfoMapPair.second;
  }
}

double
CbcmlPolicy::getPopularityScore(EntryRef i) 
{
  double predictedPopularity = 1.0; // Default if not found
  auto it = m_popularityScores.find(i->getName());
  if (it != m_popularityScores.end()) {
        predictedPopularity = it->second;
  }
  
  return predictedPopularity;
}

void
CbcmlPolicy::doAfterInsert(EntryRef i) // the policy decides whether to accept the new entry
{

  this->attachQueue(i); // if accepted
  this->evictEntries(); // if CS size exceeds the capacity limit
  
}

void
CbcmlPolicy::doAfterRefresh(EntryRef i) // the policy may update its cleanup index to note that the same Data has arrived again.
{
  this->detachQueue(i); 
  this->attachQueue(i);
}

void
CbcmlPolicy::doBeforeErase(EntryRef i) // the policy should delete the corresponding item in its cleanup index.
{ 
  this->detachQueue(i);
}

void
CbcmlPolicy::doBeforeUse(EntryRef i) 
// the policy may update its cleanup index to note that the indicated entry is accessed to satisfy an incoming Interest.
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());
  
  //NFD_LOG_TRACE("CACHEHITS: " << i->getName());
  
  EntryInfo* entryInfo = m_entryInfoMap[i];
  entryInfo->frequency = entryInfo->frequency + 1;
  
  if (entryInfo->queueType == PRIMARY_QUEUE) {
    //NFD_LOG_TRACE("CACHEHITS_PRIMARY: " << i->getName());
    updateCostValue(i);
  }
  else if (entryInfo->queueType == SECONDARY_QUEUE) {
    //NFD_LOG_TRACE("CACHEHITS_SECONDARY: " << i->getName());
    double currentTime = ns3::Simulator::Now().GetSeconds();
    double lastAccess = entryInfo->last_access;
    entryInfo->recency = currentTime - lastAccess;
    entryInfo->last_access = currentTime;
    moveToPrimaryQueue(i);
  }
}

void
CbcmlPolicy::evictEntries() // the policy should evict enough entries so that the CS does not exceed capacity limit.
{
  BOOST_ASSERT(this->getCs() != nullptr);

  while (this->getCs()->size() > this->getLimit()) {
    this->evictOne();
  }
}

void
CbcmlPolicy::evictOne()
{
  BOOST_ASSERT(!m_queues[PRIMARY_QUEUE].empty() ||
               !m_queues[SECONDARY_QUEUE].empty());
  
  EntryRef i;
  if (!m_queues[SECONDARY_QUEUE].empty()) {
    i = m_queues[SECONDARY_QUEUE].front();
    //NFD_LOG_TRACE("EVICT_SECONDARY_QUEUE: " << i->getName());
  }
  else if (m_queues[SECONDARY_QUEUE].empty()) {
    updateQueue();
    i = m_queues[SECONDARY_QUEUE].front();
    //NFD_LOG_TRACE("EVICT_SECONDARY_QUEUE(UPDATE): " << i->getName());
  }
  else if (!m_queues[PRIMARY_QUEUE].empty()) {
    i = m_queues[PRIMARY_QUEUE].front();
    //NFD_LOG_TRACE("EVICT_PRIMARY_QUEUE: " << i->getName());
  }
  
  //writeInfo(i, "evicted");

  this->detachQueue(i);
  this->emitSignal(beforeEvict, i);
  totalEviction++;
  m_cs->writeInfo(i->getName(), "evict");
}


void
CbcmlPolicy::attachQueue(EntryRef i)
{ 
  BOOST_ASSERT(m_entryInfoMap.find(i) == m_entryInfoMap.end());

  EntryInfo* entryInfo = new EntryInfo();
  
  // initialize the value
  entryInfo->prefix = i->getName();
  entryInfo->link_cost = getSourceDistance(i);
  entryInfo->last_access = ns3::Simulator::Now().GetSeconds();
  entryInfo->recency = 0;
  entryInfo->frequency = 1;
  entryInfo->cost_value = 1.0;
  
  double popularityThreshold = 734.2830; // (75%) 8520.2707 for 5x5 grid, 734.2830 for Abilene, (90% 1174.7878) 
  
  if (i->isFresh()) {
    double popularityScore = getPopularityScore(i);

    if (popularityScore > popularityThreshold) {
        entryInfo->queueType = PRIMARY_QUEUE;
        entryInfo->moveToSecondaryQueue = getScheduler().schedule(i->getData().getFreshnessPeriod(),
                                                          [=] { moveToSecondaryQueue(i); });
    }
    
  }
  else {
    entryInfo->queueType = SECONDARY_QUEUE;    
  }

  Queue& queue = m_queues[entryInfo->queueType];
  entryInfo->queueIt = queue.insert(queue.end(), i);
  m_entryInfoMap[i] = entryInfo;
}

void
CbcmlPolicy::detachQueue(EntryRef i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  EntryInfo* entryInfo = m_entryInfoMap[i];
  if (entryInfo->queueType == PRIMARY_QUEUE) {
    entryInfo->moveToSecondaryQueue.cancel();
  }

  m_queues[entryInfo->queueType].erase(entryInfo->queueIt);
  m_entryInfoMap.erase(i);
  delete entryInfo;
}

void
CbcmlPolicy::moveToSecondaryQueue(EntryRef i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  //NFD_LOG_TRACE("MOVED_TO_SECONDARY_QUEUE: " << i->getName());
  
  EntryInfo* entryInfo = m_entryInfoMap[i];
  // BOOST_ASSERT(entryInfo->queueType == PRIMARY_QUEUE);
  
  if (entryInfo->queueType == PRIMARY_QUEUE) {
     entryInfo->moveToSecondaryQueue.cancel();   
  }

  m_queues[PRIMARY_QUEUE].erase(entryInfo->queueIt);

  entryInfo->queueType = SECONDARY_QUEUE;
  Queue& queue = m_queues[SECONDARY_QUEUE];
  entryInfo->queueIt = queue.insert(queue.end(), i);
  m_entryInfoMap[i] = entryInfo;
}

void
CbcmlPolicy::moveToPrimaryQueue(EntryRef i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  //NFD_LOG_TRACE("MOVED_TO_PRIMARY_QUEUE: " << i->getName());
  
  EntryInfo* entryInfo = m_entryInfoMap[i];
  // BOOST_ASSERT(entryInfo->queueType == PRIMARY_QUEUE);

  m_queues[SECONDARY_QUEUE].erase(entryInfo->queueIt);

  entryInfo->queueType = PRIMARY_QUEUE;
  entryInfo->moveToSecondaryQueue = getScheduler().schedule(i->getData().getFreshnessPeriod(),
                                                          [=] { moveToSecondaryQueue(i); });
  Queue& queue = m_queues[PRIMARY_QUEUE];
  entryInfo->queueIt = queue.insert(queue.end(), i);
  m_entryInfoMap[i] = entryInfo;
}

void
CbcmlPolicy::updateCostValue(EntryRef i) {
  BOOST_ASSERT(!m_queues[PRIMARY_QUEUE].empty() ||
               !m_queues[SECONDARY_QUEUE].empty());
  
  // Extract features
  double freq = m_entryInfoMap[i]->frequency;
  double distance = m_entryInfoMap[i]->link_cost;
  double recency = ns3::Simulator::Now().GetSeconds() - m_entryInfoMap[i]->last_access;
  double predictedPopularity = getPopularityScore(i);
  m_entryInfoMap[i]->last_access = ns3::Simulator::Now().GetSeconds();
  
  //normalizeFeatures(freq, recency, distance);
  
  // Implement EWMA for recency
  double alpha = 0.8;
  double prevR = m_entryInfoMap[i]->recency;
  double currentR = recency;
  double ewma = alpha * currentR + (1 - alpha) * prevR;
  m_entryInfoMap[i]->recency = ewma;
    
  // Apply recency boost for content accessed within the last 10 seconds
  double currentTime = ns3::Simulator::Now().GetSeconds();
  double lastAccessTime = m_entryInfoMap[i]->last_access;
  double boostFactor = (currentTime - lastAccessTime) < 10 ? 1.2 : 1.0;

  // Updated cost function incorporating predicted popularity
  // double k1 = 0.1, k2 = 0.2, k3 = 0.3, k4 = 0.4; // Adjust these weights as needed (5x5)
  double k1 = 0.1, k2 = 0.1, k3 = 0.5, k4 = 0.3; // Abilene
  double cost = (k1 * distance) + (k2 * ewma) + (k3 * freq) + (k4 * predictedPopularity);
  m_entryInfoMap[i]->cost_value = cost * boostFactor;
    
  
  if (!m_queues[PRIMARY_QUEUE].empty())
    updateQueue();
}

void
CbcmlPolicy::updateQueue() {
  BOOST_ASSERT(!m_queues[PRIMARY_QUEUE].empty() ||
               !m_queues[SECONDARY_QUEUE].empty());
  
  EntryRef candidate;
  double temp, cost;
  
  if (!m_queues[PRIMARY_QUEUE].empty()) {
    candidate = m_queues[PRIMARY_QUEUE].front();
    temp = m_entryInfoMap[candidate]->cost_value;
    
      for(auto it = m_queues[PRIMARY_QUEUE].begin(); it != m_queues[PRIMARY_QUEUE].end(); ++it) {
        if (*it == m_queues[PRIMARY_QUEUE].front())
            continue;
        else {
            cost = m_entryInfoMap[*it]->cost_value;
            if (temp > cost) {
                temp = cost;
                candidate = *it;              
            }
        }
    }
    moveToSecondaryQueue(candidate); 
  } 
  
}

} // namespace cbcml
} // namespace cs
} // namespace nfd

