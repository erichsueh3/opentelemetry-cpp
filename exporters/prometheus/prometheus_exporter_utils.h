#pragma once

#include <string>
#include <vector>

#include "opentelemetry/sdk/metrics/record.h"
#include "prometheus/metric_family.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace prometheus
{
/**
 * The Prometheus Utils contains utility functions for Prometheus Exporter
 */
class PrometheusExporterUtils
{
public:
  /**
   * Helper function to convert OpenTelemetry metrics data collection
   * to Prometheus metrics data collection
   *
   * @param OTelMetrics a collection of metrics in OpenTelemetry
   * @return a collection of translated metrics that is acceptable by Prometheus
   */
  std::vector<MetricFamily> translateToPrometheus(std::vector<Record> OTelMetrics);

  /**
   * Sanitize the given metric name or label according to Prometheus rule.
   *
   * This function is needed because names in OpenTelemetry can contain
   * alphanumeric characters, '_', '.', and '-', whereas in Prometheus the
   * name should only contain alphanumeric characters and '_'.
   *
   * @param key name in OpenTelemetry
   * @return sanitized name in Prometheus
   */
  std::string sanitizeNames(std::string key);
};
}  // namespace prometheus
}  // namespace exporter
OPENTELEMETRY_END_NAMESPACE