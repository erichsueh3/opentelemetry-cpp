#pragma once

#include <mutex>
#include <vector>

#include "opentelemetry/sdk/metrics/record.h"
#include "prometheus/collectable.h"
#include "prometheus/metric_family.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace prometheus
{
/**
 * The Prometheus Collector maintains the intermediate collection in Prometheus Exporter
 */
class PrometheusCollector : public prometheus::Collectable
{
public:
  /**
   * Default Constructor.
   *
   * This constructor initializes the collection for metrics to export
   * in this class with default capacity
   */
  PrometheusCollector();

  /**
   * Collects all metrics data from metricsToCollect collection.
   *
   * @return all metrics in the metricsToCollect snapshot
   */
  std::vector<MetricFamily> collect();

  /**
   * This function is called by export() function and add the collection of
   * records to the metricsToCollect collection
   *
   * @param records a collection of records to add to the metricsToCollect collection
   */
  void addMetricData(std::vector<Record> records);

  /**
   * Get the current collection in the collector.
   *
   * @return current metricsToCollect collection
   */
  std::vector<Record> getCollection();

private:
  /**
   * Collection of metrics data from the export() function, and to be export
   * to user when they send a pull request.
   */
  std::vector<Record> metricsToCollect;

  /*
   * Lock when operating the metricsToCollect collection
   */
  mutable std::mutex collection_lock;
};
}  // namespace prometheus
}  // namespace exporter
OPENTELEMETRY_END_NAMESPACE