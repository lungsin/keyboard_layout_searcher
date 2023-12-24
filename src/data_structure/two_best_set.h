#pragma once

#include <compare>
#include <map>
#include <vector>

template <typename TMetric1, typename TMetric2>
struct MetricPair {
  TMetric1 m1;
  TMetric2 m2;

  std::strong_ordering operator<=>(
      MetricPair<TMetric1, TMetric2> const& o) const {
    if (m1 == o.m1) return m2 <=> o.m2;
    return m1 <=> o.m1;
  }

  bool operator==(MetricPair<TMetric1, TMetric2> const& o) const {
    return m1 == o.m1 && m2 == o.m2;
  }

  bool betterThan(MetricPair const& o) const {
    return m1 <= o.m1 && m2 <= o.m2 && (*this) != o;
  }
};

template <std::totally_ordered Metric1, std::totally_ordered Metric2,
          typename Data>
class TwoBestSet {
  using MetricKey = MetricPair<Metric1, Metric2>;

 public:
  std::vector<Data> insert(Metric1 m1, Metric2 m2, Data data) {
    MetricPair m = {m1, m2};

    auto lowerbound_it = store_.lower_bound(m);
    // if the store has the same metric stored
    if (lowerbound_it != store_.end() && lowerbound_it->first == m) {
      lowerbound_it->second.push_back(data);
      return {};
    }
    // if exists a better metric in the store
    if (lowerbound_it != store_.end() &&
        std::prev(lowerbound_it)->first.betterThan(m)) {
      return {};
    }

    // erase all that are worse
    auto worse_it = lowerbound_it;
    std::vector<Data> erased_data;
    while (m.betterThan(worse_it->first)) {
      erased_data.insert(erased_data.end(), worse_it->second.begin(),
                         worse_it->second.end());
      ++worse_it;
    }
    store_.erase(lowerbound_it, worse_it);

    // insert the metric with its data
    store_[m].push_back(data);

    return erased_data;
  }

  std::vector<Data> getAllData() {
    std::vector<Data> result;
    for (auto const& [metric_key, data_list] : store_)
      result.insert(result.end(), data_list.begin(), data_list.end());
    return result;
  }

 private:
  std::map<MetricKey, std::vector<Data>> store_;
};
