#include <iostream>
#include <fstream>
#include <cassert>

#include "partitioned_LRU.h"
#include "openssl/sha.h"

struct Partitioned_LRU::Partition
{
  Partition(const size_t& partition_size, stats& r_stat)
    : m_r_stats(r_stat)
    , m_partition_size(partition_size)
    , m_partition_bytes_cached(0)
    , m_partition_map{}
    , m_partition_queue{}
  {
  }
  
  // Places a new item into the cache partition. Performs object eviction until
  // adequate space exists to accomodate the new request. This function should
  // always succeed (cache semantics).
  void place(const Request* request)
  {
    auto it = m_partition_map.find(request->kid);
    assert(it == m_partition_map.end());

    while (m_partition_bytes_cached + request->size() > m_partition_size)
    {
      Request* eviction_candidate = &m_partition_queue.back();
      m_partition_map.erase(eviction_candidate->kid);
      m_partition_queue.pop_back();

      m_partition_bytes_cached -= eviction_candidate->size();
      m_r_stats.bytes_cached -= eviction_candidate->size();
      m_r_stats.evicted_bytes += eviction_candidate->size();
      m_r_stats.evicted_items++;
    }

    m_partition_queue.emplace_front(*request);
    m_partition_map[request->kid] = m_partition_queue.begin();
    m_partition_bytes_cached += request->size();
    m_r_stats.bytes_cached += request->size();
  }

  private:
  stats& m_r_stats;
  const size_t m_partition_size;
  size_t m_partition_bytes_cached;
  std::unordered_map<uint32_t, std::list<Request>::iterator> m_partition_map;
  std::list<Request> m_partition_queue;
};

Partitioned_LRU::Partitioned_LRU(stats stat, const size_t& num_partitions)
  : Policy{stat}
  , m_num_partitions{num_partitions}
  , m_p_partitions{}
{
  size_t partition_size = stat.global_mem / m_num_partitions;
  m_p_partitions.reserve(num_partitions);
  for (size_t i = 0; i < num_partitions; ++i)
  {
    m_p_partitions[i] = std::make_unique<Partition>(partition_size, stat);
  }
}

Partitioned_LRU::~Partitioned_LRU () 
{
}

// Performs request key lookup in partitions map. If found, retrieves the
// partition that potentially contains the matching request and performs lookup
// in the internal partition map.
size_t Partitioned_LRU::process_request(const Request* request, bool warmup) 
{
  size_t hash = request->hash_key(m_num_partitions);

  return PROC_MISS;
}

// Simply returns the current number of bytes cached.
size_t Partitioned_LRU::get_bytes_cached() const 
{ 
  return stat.bytes_cached; 
}
